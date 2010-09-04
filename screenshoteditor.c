#include "screenshoteditor.h"

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
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
	GtkWidget *scrolled_window;
	gboolean needs_update_scrollbars;
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
    g_timeout_add(10, (GSourceFunc) screenshot_editor_timer_handler, self);
    
    //Initalize a few variables.
    self->zoom_level = 1;
	self->translate_x = 0;
	self->translate_y = 0;
	self->permanant_translate_x = 0;
	self->permanant_translate_y = 0;
	self->click_state = SCREENSHOT_EDITOR_NOTHING;
	self->zoom_point_x = 0;
	self->zoom_point_y = 0;
	self->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	
	//Pack the screenshot editor widget into the scrolled window
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(self->scrolled_window), GTK_WIDGET(self));
	
}

gboolean screenshot_editor_timer_handler(ScreenshotEditor *self){
	if (self->needs_update_scrollbars == TRUE){
		self->needs_update_scrollbars = FALSE;
		screenshot_editor_scroll_relative(self, self->translate_x, self->translate_y);
	}
	return TRUE;
}

void screenshot_editor_draw_screenshot(cairo_t *cr, ScreenshotEditor *self){
	gint width, height;
	width = screenshot_editor_get_widget_width(GTK_WIDGET(self)); height = screenshot_editor_get_widget_height(GTK_WIDGET(self));
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
     
    cairo_destroy(cr);
    return FALSE;
}
gboolean screenshot_editor_clicked(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self){
	if (event->button == 2){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		gdouble width = screenshot_editor_get_widget_width(GTK_WIDGET(self));
		gdouble height = screenshot_editor_get_widget_height(GTK_WIDGET(self));
		if (self->screenshot_width * self->zoom_level < screenshot_editor_get_widget_width(GTK_WIDGET(self))  &&  self->screenshot_height * self->zoom_level < screenshot_editor_get_widget_height(GTK_WIDGET(self))){
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
	if ((event->state & GDK_BUTTON2_MASK) && (self->click_state == SCREENSHOT_EDITOR_DRAG)){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		self->translate_x = self->start_drag_mouse_x - event->x;
		self->translate_y = self->start_drag_mouse_y - event->y;
		self->needs_update_scrollbars = TRUE;
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
			
			self->zoom_point_x = (screenshot_editor_get_widget_width(GTK_WIDGET(self)) / 2) - event->x + self->translate_x + self->permanant_translate_x;
			self->zoom_point_y = (screenshot_editor_get_widget_height(GTK_WIDGET(self)) / 2) - event->y + self->translate_y + self->permanant_translate_y;
			self->zoom_level /= .9;
		}
		if (event->direction == GDK_SCROLL_DOWN && self->zoom_level > .1){
			self->zoom_point_x = (((screenshot_editor_get_widget_width(GTK_WIDGET(self)) / 2) - event->x + self->translate_x + self->permanant_translate_x));
			self->zoom_point_y = (((screenshot_editor_get_widget_height(GTK_WIDGET(self)) / 2) - event->y + self->translate_y + self->permanant_translate_y));
			self->zoom_level *= .9;
			
			if (self->screenshot_width * self->zoom_level < screenshot_editor_get_widget_width(GTK_WIDGET(self))  &&  self->screenshot_height * self->zoom_level < screenshot_editor_get_widget_height(GTK_WIDGET(self))){
				self->permanant_translate_x = 0;
				self->permanant_translate_y = 0;
				self->translate_x = 0;
				self->translate_y = 0;
			}
		}
		gtk_widget_queue_draw(widget);
	}
}

