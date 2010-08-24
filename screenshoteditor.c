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
	self->scrollbar_mouseover = SCROLLBAR_NONE;
	
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
		scrollbar_x_length = (width - (SCROLLBAR_SPACING * 2)) / (image_pixel_width / (double)width);
		scrollbar_x_position = width - (width / 2 + (width  * ((self->translate_x + self->permanant_translate_x) / (double)image_pixel_width)));
		if (self->scrollbar_mouseover == SCROLLBAR_X){
			//Draw the scrollbar as a 12px line with rounded ends. Add a white edge for visibility
			cairo_set_line_width(cr, 12);
			cairo_set_source_rgba(cr, 255,255,255, .8);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
			cairo_move_to(cr, scrollbar_x_position - (scrollbar_x_length / 2), height - 10);
			cairo_line_to(cr, scrollbar_x_position + (scrollbar_x_length / 2), height - 10);
			cairo_stroke_preserve(cr);
			cairo_set_line_width(cr, 11);
			cairo_set_source_rgba(cr, .2, .2, .2, .7);
			cairo_stroke(cr);
			//Draw lines on it to indicate clickability
			cairo_set_source_rgba(cr, 0, 0, 0, .5);
			cairo_set_line_width(cr, 1);
			cairo_move_to(cr, scrollbar_x_position, height - 7);
			cairo_line_to(cr, scrollbar_x_position, height - 13);
			cairo_move_to(cr, scrollbar_x_position + 4, height - 7);
			cairo_line_to(cr, scrollbar_x_position + 4, height - 13);
			cairo_move_to(cr, scrollbar_x_position - 4, height - 7);
			cairo_line_to(cr, scrollbar_x_position - 4, height - 13);
			cairo_stroke(cr);
		}
		else {
			//Draw the scrollbar as a 12px line with rounded ends. Add a white edge for visibility
			cairo_set_line_width(cr, 12);
			cairo_set_source_rgba(cr, 255,255,255, .8);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
			cairo_move_to(cr, scrollbar_x_position - (scrollbar_x_length / 2), height - 10);
			cairo_line_to(cr, scrollbar_x_position + (scrollbar_x_length / 2), height - 10);
			cairo_stroke_preserve(cr);
			cairo_set_line_width(cr, 11);
			cairo_set_source_rgba(cr, 0, 0, 0, .7);
			cairo_stroke(cr);
			//Draw lines on it to indicate clickability
			cairo_set_source_rgba(cr, 0, 0, 0, .5);
			cairo_set_line_width(cr, 1);
			cairo_move_to(cr, scrollbar_x_position, height - 7);
			cairo_line_to(cr, scrollbar_x_position, height - 13);
			cairo_move_to(cr, scrollbar_x_position + 4, height - 7);
			cairo_line_to(cr, scrollbar_x_position + 4, height - 13);
			cairo_move_to(cr, scrollbar_x_position - 4, height - 7);
			cairo_line_to(cr, scrollbar_x_position - 4, height - 13);
			cairo_stroke(cr);
		}
	}
	if (image_pixel_height > height){
		scrollbar_y_length = (height - (SCROLLBAR_SPACING * 2)) / (image_pixel_height / (double)height);
		scrollbar_y_position = height - (height / 2 + (height  * ((self->translate_y + self->permanant_translate_y) / (double)image_pixel_height)));
		if (self->scrollbar_mouseover == SCROLLBAR_Y){
			//Draw the scrollbar as a 12px line with rounded ends. Add a white edge for visibility
			cairo_set_line_width(cr, 12);
			cairo_set_source_rgba(cr, 255, 255, 255, .8);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
			cairo_move_to(cr, width - 10, scrollbar_y_position - (scrollbar_y_length / 2));
			cairo_line_to(cr, width - 10, scrollbar_y_position + (scrollbar_y_length / 2));
			cairo_stroke_preserve(cr);
			cairo_set_line_width(cr, 11);
			cairo_set_source_rgba(cr, .2, .2, .2, .7);
			cairo_stroke(cr);
			//Draw lines on it to indicate clickability
			cairo_set_source_rgba(cr, 0, 0, 0, .5);
			cairo_set_line_width(cr, 1);
			cairo_move_to(cr, width - 7, scrollbar_y_position);
			cairo_line_to(cr, width - 13, scrollbar_y_position);
			cairo_move_to(cr, width - 7, scrollbar_y_position + 4);
			cairo_line_to(cr, width - 13, scrollbar_y_position + 4);
			cairo_move_to(cr, width - 7, scrollbar_y_position - 4);
			cairo_line_to(cr, width - 13, scrollbar_y_position - 4);
			cairo_stroke(cr);
		} 
		else {
			//Draw the scrollbar as a 12px line with rounded ends. Add a white edge for visibility
			cairo_set_line_width(cr, 12);
			cairo_set_source_rgba(cr, 255, 255, 255, .8);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
			cairo_move_to(cr, width - 10, scrollbar_y_position - (scrollbar_y_length / 2));
			cairo_line_to(cr, width - 10, scrollbar_y_position + (scrollbar_y_length / 2));
			cairo_stroke_preserve(cr);
			cairo_set_line_width(cr, 11);
			cairo_set_source_rgba(cr, 0, 0, 0, .7);
			cairo_stroke(cr);
			//Draw lines on it to indicate clickability
			cairo_set_source_rgba(cr, 0, 0, 0, .5);
			cairo_set_line_width(cr, 1);
			cairo_move_to(cr, width - 7, scrollbar_y_position);
			cairo_line_to(cr, width - 13, scrollbar_y_position);
			cairo_move_to(cr, width - 7, scrollbar_y_position + 4);
			cairo_line_to(cr, width - 13, scrollbar_y_position + 4);
			cairo_move_to(cr, width - 7, scrollbar_y_position - 4);
			cairo_line_to(cr, width - 13, scrollbar_y_position - 4);
			cairo_stroke(cr);
		}
	}
	
	
}
gint screenshot_editor_check_mouse_scrollbars(gint mouse_x, gint mouse_y, ScreenshotEditor *self){
	gint width, height, image_pixel_width, image_pixel_height;
	gint scrollbar_x_length, scrollbar_x_start, scrollbar_x_end, scrollbar_x_center, scrollbar_x_position;
	gint scrollbar_y_length, scrollbar_y_start, scrollbar_y_end, scrollbar_y_center, scrollbar_y_position;
	gdouble scrollbar_x_fraction, scrollbar_y_fraction;
	
	width = screenshot_editor_get_width(self); 
	height = screenshot_editor_get_height(self);
	image_pixel_width = (self->screenshot_width + 40) * self->zoom_level;
	image_pixel_height = (self->screenshot_height + 40) * self->zoom_level;
	if (image_pixel_width > width){
		scrollbar_x_length = (width - (SCROLLBAR_SPACING * 2)) / (image_pixel_width / (double)width);
		scrollbar_x_position = width - (width / 2 + (width  * ((self->translate_x + self->permanant_translate_x) / (double)image_pixel_width)));
		if (mouse_x > (scrollbar_x_position - (scrollbar_x_length / 2)) && mouse_x < (scrollbar_x_position + (scrollbar_x_length / 2))){
			if (mouse_y > height - 16 && mouse_y < height - 4){
				return SCROLLBAR_X;
			}
		}
	}
	if (image_pixel_height > height){
		scrollbar_y_length = (height - (SCROLLBAR_SPACING * 2)) / (image_pixel_height / (double)height);
		scrollbar_y_position = height - (height / 2 + (height  * ((self->translate_y + self->permanant_translate_y) / (double)image_pixel_height)));
		if (mouse_y > (scrollbar_y_position - (scrollbar_y_length / 2)) && mouse_y < (scrollbar_y_position + (scrollbar_y_length / 2))){
			if (mouse_x > width - 16 && mouse_x < width - 4){
				return SCROLLBAR_Y;
			}
		}
	}
	return SCROLLBAR_NONE;
	
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
	if (event->button == 2){
		gdk_window_set_cursor(editor->window, gdk_cursor_new(GDK_HAND1));
		self->click_state = SCREENSHOT_EDITOR_DRAG;
		self->start_drag_mouse_x = event->x;
		self->start_drag_mouse_y = event->y;
	}
	if (event->button == 1){
		if (screenshot_editor_check_mouse_scrollbars(event->x, event->y, self) == SCROLLBAR_X){
			self->click_state = SCREENSHOT_EDITOR_DRAG_SCROLLBAR_X;
			self->start_drag_mouse_x = event->x;
			self->start_drag_mouse_y = event->y;
		}
		if (screenshot_editor_check_mouse_scrollbars(event->x, event->y, self) == SCROLLBAR_Y){
			self->click_state = SCREENSHOT_EDITOR_DRAG_SCROLLBAR_Y;
			self->start_drag_mouse_x = event->x;
			self->start_drag_mouse_y = event->y;
		}
	}
    return FALSE;
}
gboolean screenshot_editor_released(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self){
	self->permanant_translate_x += self->translate_x;
	self->permanant_translate_y += self->translate_y;
	self->translate_x = 0;
	self->translate_y = 0;
	self->click_state = SCREENSHOT_EDITOR_NOTHING;
	gdk_window_set_cursor(editor->window, NULL);
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
		if (self->translate_y + self->permanant_translate_y > (self->screenshot_height - height) / 2){
			self->start_drag_mouse_y = self->start_drag_mouse_y + (self->translate_y + self->permanant_translate_y - ((self->screenshot_height - height + 20) / 2));
			self->translate_y = event->y - self->start_drag_mouse_y;
		}
		if (self->translate_y + self->permanant_translate_y < -(self->screenshot_height - height) / 2){
			self->start_drag_mouse_y = self->start_drag_mouse_y + (self->translate_y + self->permanant_translate_y + ((self->screenshot_height - height + 20) / 2));
			self->translate_y = event->y - self->start_drag_mouse_y;
		}
		gtk_widget_queue_draw(editor);
	}
	if (self->click_state == SCREENSHOT_EDITOR_DRAG_SCROLLBAR_X){
		self->translate_x = (screenshot_zoomed_width / self->zoom_level) * ((double)(self->start_drag_mouse_x - event->x) / (double)width);
		if (self->translate_x + self->permanant_translate_x > (screenshot_zoomed_width - width) / 2){
			self->translate_x = (screenshot_zoomed_width - width) / 2 - self->permanant_translate_x;
		}
		if (self->translate_x + self->permanant_translate_x < -(screenshot_zoomed_width - width) / 2){
			self->translate_x = -(screenshot_zoomed_width - width) / 2 - self->permanant_translate_x;
		}
		gtk_widget_queue_draw(editor);
	}
	if (self->click_state == SCREENSHOT_EDITOR_DRAG_SCROLLBAR_Y){
		self->translate_y = (self->screenshot_height / self->zoom_level) * ((double)(self->start_drag_mouse_y - event->y) / (double)height);
		if (self->translate_y + self->permanant_translate_y > (self->screenshot_height - height) / 2){
			self->translate_y = (self->screenshot_height - height) / 2 - self->permanant_translate_y;
		}
		if (self->translate_y + self->permanant_translate_y < -(self->screenshot_height - height) / 2){
			self->translate_y = -(self->screenshot_height - height) / 2 - self->permanant_translate_y;
		}
		gtk_widget_queue_draw(editor);
	}
	if (screenshot_editor_check_mouse_scrollbars(event->x, event->y, self) != self->scrollbar_mouseover){
		self->scrollbar_mouseover = screenshot_editor_check_mouse_scrollbars(event->x, event->y, self);
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

