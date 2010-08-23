#include "screenshoteditor.h"
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
G_DEFINE_TYPE (ScreenshotEditor, screenshot_editor, GTK_TYPE_DRAWING_AREA);


static void
screenshot_editor_class_init (ScreenshotEditorClass *klass)
{

    cairo_surface_t *screenshot;
    gint screenshot_width, screenshot_height;
    gdouble zoom_level;
	gint translate_x, translate_y;
	gdouble start_drag_mouse_x, start_drag_mouse_y;
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
void screenshot_editor_draw_scrollbars(cairo_t *cr, ScreenshotEditor *self){
	gint width, height, image_pixel_width, image_pixel_height;
	gint scrollbar_x_length, scrollbar_x_start, scrollbar_x_end, scrollbar_x_center, scrollbar_x_position;
	gint scrollbar_y_length, scrollbar_y_start, scrollbar_y_end, scrollbar_y_center, scrollbar_y_position;
	gdouble scrollbar_x_fraction, scrollbar_y_fraction;
	
	width = screenshot_editor_get_width(self); 
	height = screenshot_editor_get_height(self);
	image_pixel_width = (self->screenshot_width + 40) * self->zoom_level;
	image_pixel_height = (self->screenshot_height + 40) * self->zoom_level;
	if (image_pixel_width > width){
		scrollbar_x_length = (double)width / (image_pixel_width / (double)width);
		scrollbar_x_start = scrollbar_x_length / 2;
		scrollbar_x_end = width - (scrollbar_x_length / 2);
		scrollbar_x_fraction = (double)(self->translate_x + self->permanant_translate_x) / (double)image_pixel_width;
		scrollbar_x_center = (scrollbar_x_start + scrollbar_x_end) / 2;
		scrollbar_x_position = width - (scrollbar_x_center + (width  * scrollbar_x_fraction));
		cairo_set_line_width(cr, 12);
		cairo_set_source_rgba(cr, 255,255,255, .8);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_move_to(cr, scrollbar_x_position - (scrollbar_x_length / 2), height - 10);
		cairo_line_to(cr, scrollbar_x_position + (scrollbar_x_length / 2), height - 10);
		cairo_stroke_preserve(cr);
		cairo_set_line_width(cr, 11);
		cairo_set_source_rgba(cr, 0, 0, 0, .7);
		cairo_stroke(cr);
	}
	if (image_pixel_height > height){
		scrollbar_y_length = (double)height / (image_pixel_height / (double)height);
		scrollbar_y_start = scrollbar_y_length / 2;
		scrollbar_y_end = height - (scrollbar_y_length / 2);
		scrollbar_y_fraction = (double)(self->translate_y + self->permanant_translate_y) / (double)image_pixel_height;
		scrollbar_y_center = (scrollbar_y_start + scrollbar_y_end) / 2;
		scrollbar_y_position = height - (scrollbar_y_center + (height  * scrollbar_y_fraction));
		cairo_set_line_width(cr, 12);
		cairo_set_source_rgba(cr, 255, 255, 255, .8);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_move_to(cr, width - 10, scrollbar_y_position - (scrollbar_y_length / 2));
		cairo_line_to(cr, width - 10, scrollbar_y_position + (scrollbar_y_length / 2));
		cairo_stroke_preserve(cr);
		cairo_set_line_width(cr, 11);
		cairo_set_source_rgba(cr, 0, 0, 0, .7);
		cairo_stroke(cr);
	}
	
	
}
	
void screenshot_editor_draw_screenshot(cairo_t *cr, ScreenshotEditor *self){
	gint width, height;
	width = screenshot_editor_get_width(self); height = screenshot_editor_get_height(self);
	cairo_save(cr);
	//Zoom the view according to zoom_level. Center it in the middle of the view.
	cairo_translate(cr, (width / 2) - ((self->screenshot_width * self->zoom_level) / 2) + self->translate_x + self->permanant_translate_x, (height / 2) - ((self->screenshot_height * self->zoom_level) / 2) + self->translate_y + self->permanant_translate_y);
    cairo_scale(cr, self->zoom_level, self->zoom_level);
    
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
    cr = gdk_cairo_create(editor->window);
	
	//Fill the widget with a darker color
	cairo_set_source_rgba(cr, 0, 0, 0, .15);
	cairo_paint(cr);
	
	//Draw the screenshot
	screenshot_editor_draw_screenshot(cr, self);
    
    //Draw the scrollbars
    screenshot_editor_draw_scrollbars(cr, self);
    
    cairo_destroy(cr);
    return FALSE;
}
gboolean screenshot_editor_clicked(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self){
	if ((event->button == 2)){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
	}
	self->start_drag_mouse_x = event->x;
	self->start_drag_mouse_y = event->y;
    return FALSE;
}
gboolean screenshot_editor_released(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self){
	self->permanant_translate_x += self->translate_x;
	self->permanant_translate_y += self->translate_y;
	self->translate_x = 0;
	self->translate_y = 0;
	gdk_window_set_cursor(editor->window, NULL);
    return FALSE;
}
gboolean screenshot_editor_move_mouse(GtkWidget *editor, GdkEventMotion *event, ScreenshotEditor *self){
	if ((event->state & GDK_BUTTON2_MASK)){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		self->translate_x = event->x - self->start_drag_mouse_x;
		self->translate_y = event->y - self->start_drag_mouse_y;	
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
	if (event->state & GDK_CONTROL_MASK){
		if (event->direction == GDK_SCROLL_UP){
			self->zoom_level /= .9;
		}
		if (event->direction == GDK_SCROLL_DOWN && self->zoom_level > .1){
			self->zoom_level *= .9;
		}
		gtk_widget_queue_draw(widget);
	}
}

