/*
 * Dieki Noordhoek
 * GPL v3
 */

/* inclusion guard */
#ifndef __SCREENSHOT_EDITOR_H__
#define __SCREENSHOT_EDITOR_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define SCREENSHOT_TYPE_EDITOR                  (screenshot_editor_get_type ())
#define SCREENSHOT_EDITOR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCREENSHOT_TYPE_EDITOR, ScreenshotEditor))
#define SCREENSHOT_EDITOR_CLASS(obj)          (G_TYPE_CHECK_CLASS_CAST ((obj), SCREENSHOT_EDITOR, ScreenshotEditorClass))
#define SCREENSHOT_IS_EDITOR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCREENSHOT_TYPE_EDITOR))
#define SCREENSHOT_IS_EDITOR_CLASS(obj)       (G_TYPE_CHECK_CLASS_TYPE ((obj), SCREENSHOT_TYPE_EDITOR))
#define SCREENSHOT_EDITOR_GET_CLASS        (G_TYPE_INSTANCE_GET_CLASS ((obj), SCREENSHOT_TYPE_EDITOR, ScreenshotEditorClass))

typedef struct _ScreenshotEditor        ScreenshotEditor;
typedef struct _ScreenshotEditorClass   ScreenshotEditorClass;
typedef enum{
    SCREENSHOT_EDITOR_DRAG,
    SCREENSHOT_EDITOR_DRAG_SCROLLBAR_X,
    SCREENSHOT_EDITOR_DRAG_SCROLLBAR_Y,
    SCREENSHOT_EDITOR_NOTHING
} click_states;
typedef enum{
    SCROLLBAR_NONE,
    SCROLLBAR_X,
    SCROLLBAR_Y
} which_scrollbar;
struct _ScreenshotEditor
{
    GtkDrawingArea parent;
    gdouble zoom_level;
    gdouble translate_x, translate_y, permanant_translate_x, permanant_translate_y;
    GtkWidget *scrolled_window;
    /* <private> */
    cairo_surface_t *screenshot;
    gint screenshot_width, screenshot_height;
	gdouble start_drag_mouse_x, start_drag_mouse_y;
	gint click_state;
	gint scrollbar_mouseover;
	gdouble zoom_point_x, zoom_point_y;
	gboolean needs_update_scrollbars;
	

};

struct _ScreenshotEditorClass
{
    GtkDrawingAreaClass parent_class;

    /* class members */
};




GtkWidget *screenshot_editor_new (void);
gboolean screenshot_editor_clicked(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self);
gboolean screenshot_editor_expose(GtkWidget *editor, GdkEventExpose *event, ScreenshotEditor *self);
void screenshot_editor_load_screenshot(ScreenshotEditor *self, cairo_surface_t *screenshot, gint screenshot_width, gint screenshot_height);
void screenshot_editor_scroll(GtkWidget *widget, GdkEventScroll *event, ScreenshotEditor *self);
gboolean screenshot_editor_move_mouse(GtkWidget *editor, GdkEventMotion *event, ScreenshotEditor *self);
gboolean screenshot_editor_released(GtkWidget *editor, GdkEventButton *event, ScreenshotEditor *self);
void screenshot_editor_draw_screenshot(cairo_t *cr, ScreenshotEditor *self);
void screenshot_editor_draw_scrollbars(cairo_t *cr, ScreenshotEditor *self);
gboolean screenshot_editor_timer_handler(ScreenshotEditor *self);
#include "screenshoteditor-helpers.h"
G_END_DECLS

#endif /* __SCREENSHOT_EDITOR_H__ */
