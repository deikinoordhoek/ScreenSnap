#include "screenshoteditor.h"
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
G_DEFINE_TYPE (ScreenshotEditor, screenshot_editor, GTK_TYPE_DRAWING_AREA);
#define SCROLLBAR_SPACING  25

static void
screenshot_editor_class_init (ScreenshotEditorClass *klass)
{

    cairo_surface_t *screenshot;
    gint screenshot_width, screenshot_height;
    gdouble zoom_level;
	gint translate_x, translate_y;
	gdouble start_drag_mouse_x, start_drag_mouse_y;
	gint click_state;
	gint scrollbar_mouseover;
	gdouble zoom_point_x, zoom_point_y;
}

static void
screenshot_editor_init (ScreenshotEditor *self)
{
    //Add events to the widget
    gtk_widget_add_events (GTK_WIDGET(self), GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events (GTK_WIDGET(self), GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events (GTK_WIDGET(self), GDK_SCROLL_MASK);
    gtk_widget_add_events (GTK_WIDGET(self), GDK_POINTER_MOTION_MASK);
    
    //Connect the events to their handlers.
    g_signal_connect(GTK_WIDGET(self), "expose-event", G_CALLBACK(screenshot_editor_expose), self);
    g_signal_connect(GTK_WIDGET(self), "scroll-event", G_CALLBACK(screenshot_editor_scroll), self);
    g_signal_connect (GTK_WIDGET(self), "button-press-event", G_CALLBACK (screenshot_editor_clicked), self);
    g_signal_connect (GTK_WIDGET(self), "button-release-event", G_CALLBACK (screenshot_editor_released), self);
    g_signal_connect (GTK_WIDGET(self), "motion-notify-event", G_CALLBACK (screenshot_editor_move_mouse), self);
    
    //Initalize a few variables.
    self->zoom_level = 1;
	self->translate_x = 0;
	self->translate_y = 0;
	self->permanant_translate_x = 0;
	self->permanant_translate_y = 0;
	self->click_state = SCREENSHOT_EDITOR_NOTHING;
	self->zoom_point_x = 0;
	self->zoom_point_y = 0;
	
}

static gint screenshot_editor_get_width(ScreenshotEditor *editor){
	GtkAllocation allocation = {0, 0, -1, -1};
	gtk_widget_get_allocation(GTK_WIDGET(editor), &allocation);
	return allocation.width;
}
static gint screenshot_editor_get_height(ScreenshotEditor *editor){
	GtkAllocation allocation = {0, 0, -1, -1};
	gtk_widget_get_allocation(GTK_WIDGET(editor), &allocation);
	return allocation.height;
}

void screenshot_editor_draw_screenshot(cairo_t *cr, ScreenshotEditor *self){
	gint width, height;
	width = screenshot_editor_get_width(self); height = screenshot_editor_get_height(self);
	cairo_save(cr);
	//Zoom the view according to zoom_level. Center it in the middle of the view.
	cairo_translate(cr, -self->zoom_point_x , -self->zoom_point_y);
    cairo_scale(cr, self->zoom_level, self->zoom_level);
    cairo_translate(cr, self->zoom_point_x, self->zoom_point_y);
    //Set the source to the screenshot and draw.
    cairo_set_source_surface(cr, self->screenshot, 0, 0);
    cairo_paint(cr);
    
    //Draw a 2px border around the entire thing. Compensate for zoom so that it's always 2px.
    double dashes[] = {5.0 / self->zoom_level, 5.0 / self->zoom_level};
    cairo_set_line_width(cr, (double) 1 / self->zoom_level);
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    cairo_rectangle(cr, 0,0, self->screenshot_width, self->screenshot_height);
    cairo_stroke(cr);
    
    cairo_set_dash(cr, dashes, 2, 0);
    cairo_set_line_width(cr, (double) 1 / self->zoom_level);
    cairo_set_source_rgba(cr, 255, 255, 0, 1);
    cairo_rectangle(cr, 0,0, self->screenshot_width, self->screenshot_height);
    cairo_stroke(cr);
    cairo_restore(cr);
}
gboolean screenshot_editor_expose(GtkWidget *editor, GdkEventExpose *event, ScreenshotEditor *self){
    cairo_t *cr;
    char print_string[50];
    cr = gdk_cairo_create(editor->window);
	
	//Fill the widget with a darker color
	cairo_set_source_rgba(cr, 0, 0, 0, .15);
	cairo_paint(cr);
	gtk_widget_set_size_request(editor, self->screenshot_width * self->zoom_level, self->screenshot_height * self->zoom_level);
	//Draw the screenshot
	screenshot_editor_draw_screenshot(cr, self);
    

    //Show some debug data
    cairo_set_source_rgba(cr, 0, 0, 0, .7);
	cairo_rectangle(cr, 0, 0, 100, 15);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 1, 1, 1); 
	cairo_select_font_face(cr, "UbuntuBeta",
      CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 13);
	cairo_move_to(cr, 10, 10);
	sprintf(print_string, "%f %f %f", self->zoom_level, self->zoom_point_x, self->zoom_point_y);
	cairo_show_text(cr, print_string);
	
    
    cairo_destroy(cr);
    return FALSE;
}
gboolean screenshot_editor_clicked(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self){
	if (event->button == 2){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		gdouble width = screenshot_editor_get_width(self);
		gdouble height = screenshot_editor_get_height(self);
		if (self->screenshot_width * self->zoom_level < screenshot_editor_get_width(self)  &&  self->screenshot_height * self->zoom_level < screenshot_editor_get_height(self)){
			self->click_state = SCREENSHOT_EDITOR_NOTHING;
			self->permanant_translate_x = 0;
			self->permanant_translate_y = 0;
			self->translate_x = 0;
			self->translate_y = 0;
			gtk_widget_queue_draw(editor);
		}
		else{
			self->click_state = SCREENSHOT_EDITOR_DRAG;
		}
		
		self->start_drag_mouse_x = event->x;
		self->start_drag_mouse_y = event->y;
	}
	if (event->button == 1){
		
	}
    return FALSE;
}
gboolean screenshot_editor_released(GtkWidget *widget, GdkEventButton *event, ScreenshotEditor *self){
	self->permanant_translate_x += self->translate_x;
	self->permanant_translate_y += self->translate_y;
	self->translate_x = 0;
	self->translate_y = 0;
	self->click_state = SCREENSHOT_EDITOR_NOTHING;
	gdk_window_set_cursor(widget->window, NULL);
	gtk_widget_queue_draw(widget);
    return FALSE;
}
gboolean screenshot_editor_move_mouse(GtkWidget *editor, GdkEventMotion *event, ScreenshotEditor *self){
	gint width, height;
	gint scrollbar_dragged;
	gint screenshot_zoomed_width, screenshot_zoomed_height;
	screenshot_zoomed_width = self->screenshot_width * self->zoom_level;
	screenshot_zoomed_height = self->screenshot_height * self->zoom_level;
	width = screenshot_editor_get_width(self); height = screenshot_editor_get_height(self);
	if ((event->state & GDK_BUTTON2_MASK) && (self->click_state == SCREENSHOT_EDITOR_DRAG)){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		self->translate_x = event->x - self->start_drag_mouse_x;
		self->translate_y = event->y - self->start_drag_mouse_y;
		
		if (self->translate_x + self->permanant_translate_x > (screenshot_zoomed_width - width) / 2){
			self->start_drag_mouse_x = self->start_drag_mouse_x + (self->translate_x + self->permanant_translate_x - ((screenshot_zoomed_width - width + 20) / 2));
			self->translate_x = event->x - self->start_drag_mouse_x;
		}
		if (self->translate_x + self->permanant_translate_x < -(screenshot_zoomed_width - width) / 2){
			self->start_drag_mouse_x = self->start_drag_mouse_x + (self->translate_x + self->permanant_translate_x + ((screenshot_zoomed_width - width + 20) / 2));
			self->translate_x = event->x - self->start_drag_mouse_x;
		}
		if (self->translate_y + self->permanant_translate_y > (screenshot_zoomed_height - height) / 2){
			self->start_drag_mouse_y = self->start_drag_mouse_y + (self->translate_y + self->permanant_translate_y - ((screenshot_zoomed_height - height + 20) / 2));
			self->translate_y = event->y - self->start_drag_mouse_y;
		}
		if (self->translate_y + self->permanant_translate_y < -(screenshot_zoomed_height - height) / 2){
			self->start_drag_mouse_y = self->start_drag_mouse_y + (self->translate_y + self->permanant_translate_y + ((screenshot_zoomed_height - height + 20) / 2));
			self->translate_y = event->y - self->start_drag_mouse_y;
		}
		gtk_widget_queue_draw(editor);
	}
	
    return FALSE;
}

GtkWidget *screenshot_editor_new(void){
    return g_object_new(SCREENSHOT_TYPE_EDITOR, NULL);
}
void screenshot_editor_load_screenshot(ScreenshotEditor *self, cairo_surface_t *screenshot, gint screenshot_width, gint screenshot_height){

    self->screenshot = screenshot;
    self->screenshot_width = screenshot_width;
    self->screenshot_height = screenshot_height;

}
void screenshot_editor_scroll(GtkWidget *widget, GdkEventScroll *event, ScreenshotEditor *self){
	double mouse_pixel_x, mouse_pixel_y;
	double new_mouse_pixel_x, new_mouse_pixel_y;
	if (event->state & GDK_CONTROL_MASK){
		if (event->direction == GDK_SCROLL_UP){
			
			self->zoom_point_x = (screenshot_editor_get_width(self) / 2) - event->x + self->translate_x + self->permanant_translate_x;
			self->zoom_point_y = (screenshot_editor_get_height(self) / 2) - event->y + self->translate_y + self->permanant_translate_y;
			self->zoom_level /= .9;
		}
		if (event->direction == GDK_SCROLL_DOWN && self->zoom_level > .1){
			self->zoom_point_x = (((screenshot_editor_get_width(self) / 2) - event->x + self->translate_x + self->permanant_translate_x));
			self->zoom_point_y = (((screenshot_editor_get_height(self) / 2) - event->y + self->translate_y + self->permanant_translate_y));
			self->zoom_level *= .9;
			
			if (self->screenshot_width * self->zoom_level < screenshot_editor_get_width(self)  &&  self->screenshot_height * self->zoom_level < screenshot_editor_get_height(self)){
				self->permanant_translate_x = 0;
				self->permanant_translate_y = 0;
				self->translate_x = 0;
				self->translate_y = 0;
			}
		}
		gtk_widget_queue_draw(widget);
	}
}

