#include "screenshoteditor.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

gdouble screenshot_editor_get_scroll_x (ScreenshotEditor *self);
gdouble screenshot_editor_get_scroll_y (ScreenshotEditor *self);
void screenshot_editor_set_scroll_x (ScreenshotEditor *self, gdouble scroll_x);
void screenshot_editor_set_scroll_y (ScreenshotEditor *self, gdouble scroll_y);
gint screenshot_editor_get_width(ScreenshotEditor *editor);
gint screenshot_editor_get_height(ScreenshotEditor *editor);
void screenshot_editor_scroll_relative(ScreenshotEditor *self, gdouble move_x, gdouble move_y);

G_END_DECLS
