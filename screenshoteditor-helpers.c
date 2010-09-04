//This file contains a collection of helper functions mostly used in screenshoteditor.c
#include <gtk/gtk.h>
#include "screenshoteditor-helpers.h"


//Scrollbar functions
gdouble screenshot_editor_get_scroll_x (ScreenshotEditor *self){
	GtkAdjustment *h_adjust;
	
	h_adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));
	return gtk_adjustment_get_value(GTK_ADJUSTMENT(h_adjust));
}

gdouble screenshot_editor_get_scroll_y (ScreenshotEditor *self){
	GtkAdjustment *v_adjust;
	
	v_adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));
	return gtk_adjustment_get_value(GTK_ADJUSTMENT(v_adjust));
}

void screenshot_editor_set_scroll_y (ScreenshotEditor *self, gdouble scroll_y){
	GtkAdjustment *v_adjust;
	
	v_adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(v_adjust), scroll_y);
	
	return;
}

void screenshot_editor_set_scroll_x (ScreenshotEditor *self, gdouble scroll_x){
	GtkAdjustment *h_adjust;
	
	h_adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(h_adjust), scroll_x);
	
	return;
}


//Convience function for canvas dragging
void screenshot_editor_scroll_relative(ScreenshotEditor *self, gdouble move_x, gdouble move_y){
	gdouble x, y;
	GtkAdjustment *h_adjust;
	GtkAdjustment *v_adjust;
	
	h_adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));	
	v_adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(self->scrolled_window));
	
	x = gtk_adjustment_get_value(GTK_ADJUSTMENT(h_adjust));
	y = gtk_adjustment_get_value(GTK_ADJUSTMENT(v_adjust));
	
	x = x + move_x;
	y = y + move_y;
	
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > screenshot_editor_get_width(self)) x = screenshot_editor_get_width(self);
	if (y > screenshot_editor_get_height(self)) y = screenshot_editor_get_height(self);
	
	gtk_adjustment_set_value(GTK_ADJUSTMENT(h_adjust), x);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(v_adjust), y);
	return;
}


//Widget size functions
gint screenshot_editor_get_width(ScreenshotEditor *editor){
	GtkAllocation allocation = {0, 0, -1, -1};
	gtk_widget_get_allocation(GTK_WIDGET(editor), &allocation);
	
	return allocation.width;
}

gint screenshot_editor_get_height(ScreenshotEditor *editor){
	GtkAllocation allocation = {0, 0, -1, -1};
	gtk_widget_get_allocation(GTK_WIDGET(editor), &allocation);
	
	return allocation.height;
}
