#include "screenshoteditor.h"
#include "main.h"
#include <stdio.h>
#include <gtk/gtk.h>

int main (int argc, char **argv)
{
        //Init variables
        GtkWidget *window;
        GtkWidget *main_editor;
        GtkWidget *main_vbox, *main_editor_frame, *buttons_box, *save_button;
        GtkWidget *close_button, *upload_button, *copy_button;
        GtkWidget *main_editor_hbox;
        cairo_surface_t *screenshot;
        gint width, height;

        gtk_init (&argc, &argv);

		//Get the screenshot
        screenshot = get_screenshot();
        gdk_drawable_get_size (gdk_get_default_root_window (), &width, &height);
		
		//Create all the widgets
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        main_editor = screenshot_editor_new ();
        main_vbox = gtk_vbox_new(FALSE, 0);
        main_editor_frame = gtk_frame_new(NULL);
        main_editor_hbox = gtk_hbox_new(FALSE, 0);
        buttons_box = gtk_hbox_new(FALSE, 0);
        save_button = gtk_button_new_with_label("Save to file");
        close_button = gtk_button_new_with_label("Dismiss");
        copy_button = gtk_button_new_with_label("Copy to clipboard");
        upload_button = gtk_button_new_with_label("Upload to web");
        
        
        //Configure all the widgets
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 550); 
        screenshot_editor_load_screenshot(SCREENSHOT_EDITOR(main_editor), screenshot, width, height);
        gtk_window_set_title(GTK_WINDOW(window), "Edit Screenshot");
		
		//Pack the widgets
        gtk_container_add (GTK_CONTAINER (window), main_vbox);
		gtk_container_add (GTK_CONTAINER (main_editor_frame), main_editor);
		gtk_box_pack_start(GTK_BOX (main_editor_hbox), main_editor_frame, TRUE, TRUE, 10);
		gtk_box_pack_start(GTK_BOX (main_vbox), main_editor_hbox, TRUE, TRUE, 10);
        gtk_box_pack_start (GTK_BOX (main_vbox), buttons_box, FALSE, FALSE, 5);
        gtk_box_pack_end (GTK_BOX (buttons_box), close_button, FALSE, FALSE, 10);
        gtk_box_pack_end (GTK_BOX (buttons_box), save_button, FALSE, FALSE, 10);
        gtk_box_pack_end (GTK_BOX (buttons_box), upload_button, FALSE, FALSE, 10);
        gtk_box_pack_end (GTK_BOX (buttons_box), copy_button, FALSE, FALSE, 10);
        
        //Connect the widget signals
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
        g_signal_connect (close_button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
        
        //Show all the widgets
        gtk_widget_show_all (window);
        gtk_widget_show(main_editor);

		//Runtime!
        gtk_main ();
}

cairo_surface_t * get_screenshot(){
    GdkPixbuf *screenshot;
    GdkWindow *root_window;
    cairo_t *cr;
    gint x_orig, y_orig;
    gint width, height;
	
	//Get the screenshot.
    root_window = gdk_get_default_root_window ();
    gdk_drawable_get_size (root_window, &width, &height);      
    gdk_window_get_origin (root_window, &x_orig, &y_orig);
    screenshot = gdk_pixbuf_get_from_drawable (NULL, root_window, NULL, x_orig, y_orig, 0, 0, width, height);
	
	//Prepare the cairo surface.
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, gdk_pixbuf_get_width(screenshot), gdk_pixbuf_get_height(screenshot));
    cr = cairo_create(surface);
    
    //Copy the pixbuf to the surface and paint it.
    gdk_cairo_set_source_pixbuf(cr, screenshot, 0, 0);    
    cairo_paint(cr);
	
	//Clean up and return.
    cairo_destroy(cr);
    return surface;
}
