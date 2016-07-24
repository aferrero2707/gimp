/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "display-types.h"

#include "core/gimpimage.h"

#include "gimpdisplay.h"
#include "gimpdisplayshell.h"
#include "gimpdisplayshell-scale.h"
#include "gimpdisplayshell-transform.h"
#include "gimpdisplayshell-scrollbars.h"


#define MINIMUM_STEP_AMOUNT 1.0


/**
 * gimp_display_shell_scrollbars_update:
 * @shell:
 *
 **/
void
gimp_display_shell_scrollbars_update (GimpDisplayShell *shell)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));

  if (! shell->display)
    return;

  /* Horizontal scrollbar */

  g_object_freeze_notify (G_OBJECT (shell->hsbdata));

  /* Update upper and lower value before we set the new value */
  gimp_display_shell_scrollbars_setup_horizontal (shell, shell->offset_x);

  g_object_set (shell->hsbdata,
                "value",          (gdouble) shell->offset_x,
                "page-size",      (gdouble) shell->disp_width,
                "page-increment", (gdouble) shell->disp_width / 2,
                NULL);

  g_object_thaw_notify (G_OBJECT (shell->hsbdata)); /* emits "changed" */


  /* Vertcal scrollbar */

  g_object_freeze_notify (G_OBJECT (shell->vsbdata));

  /* Update upper and lower value before we set the new value */
  gimp_display_shell_scrollbars_setup_vertical (shell, shell->offset_y);

  g_object_set (shell->vsbdata,
                "value",          (gdouble) shell->offset_y,
                "page-size",      (gdouble) shell->disp_height,
                "page-increment", (gdouble) shell->disp_height / 2,
                NULL);

  g_object_thaw_notify (G_OBJECT (shell->vsbdata)); /* emits "changed" */
}

/**
 * gimp_display_shell_scrollbars_setup_horizontal:
 * @shell:
 * @value:
 *
 * Setup the limits of the horizontal scrollbar
 **/
void
gimp_display_shell_scrollbars_setup_horizontal (GimpDisplayShell *shell,
                                                gdouble           value)
{
  GimpImage *image;
  gdouble    dx, dy;
  gdouble    dw, dh;
  gdouble    lower;
  gdouble    upper;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));

  if (! shell->display)
    return;

  image = gimp_display_get_image (shell->display);
  if (! image)
    return;

  gimp_display_shell_transform_bounds (shell,
                                       0, 0,
                                       gimp_image_get_width (image),
                                       gimp_image_get_height (image),
                                       &dx, &dy,
                                       &dw, &dh);

  /* Convert scrolled (x1, x2) to unscrolled (x, width). */
  dw -= dx;
  dx += shell->offset_x;

  if (shell->disp_width < dw)
    {
      lower = MIN (value, dx);
      upper = MAX (value + shell->disp_width, dx + dw);
    }
  else
    {
      lower = MIN (value, dx - (shell->disp_width - dw) / 2);
      upper = MAX (value + shell->disp_width,
                   dx + dw + (shell->disp_width - dw) / 2);
    }

  g_object_set (shell->hsbdata,
                "lower",          lower,
                "upper",          upper,
                "step-increment", (gdouble) MAX (shell->scale_x,
                                                 MINIMUM_STEP_AMOUNT),
                NULL);
}

/**
 * gimp_display_shell_scrollbars_setup_vertical:
 * @shell:
 * @value:
 *
 * Setup the limits of the vertical scrollbar
 **/
void
gimp_display_shell_scrollbars_setup_vertical (GimpDisplayShell *shell,
                                              gdouble           value)
{
  GimpImage *image;
  gdouble    dx, dy;
  gdouble    dw, dh;
  gdouble    lower;
  gdouble    upper;

  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));

  if (! shell->display)
    return;

  image = gimp_display_get_image (shell->display);
  if (! image)
    return;

  gimp_display_shell_transform_bounds (shell,
                                       0, 0,
                                       gimp_image_get_width (image),
                                       gimp_image_get_height (image),
                                       &dx, &dy,
                                       &dw, &dh);

  /* Convert scrolled (y1, y2) to unscrolled (y, height). */
  dh -= dy;
  dy += shell->offset_y;

  if (shell->disp_height < dh)
    {
      lower = MIN (value, dy);
      upper = MAX (value + shell->disp_height, dy + dh);
    }
  else
    {
      lower = MIN (value, dy - (shell->disp_height - dh) / 2);
      upper = MAX (value + shell->disp_height,
                   dy + dh + (shell->disp_height - dh) / 2);
    }

  g_object_set (shell->vsbdata,
                "lower",          lower,
                "upper",          upper,
                "step-increment", (gdouble) MAX (shell->scale_y,
                                                 MINIMUM_STEP_AMOUNT),
                NULL);
}

/**
 * gimp_display_shell_scrollbars_update_steppers:
 * @shell:
 * @min_offset_x:
 * @max_offset_x:
 * @min_offset_y:
 * @max_offset_y:
 *
 * Sets the scrollbars' stepper sensitivity which is set differently
 * from its adjustment limits because we support overscrolling.
 **/
void
gimp_display_shell_scrollbars_update_steppers (GimpDisplayShell *shell,
                                               gint              min_offset_x,
                                               gint              max_offset_x,
                                               gint              min_offset_y,
                                               gint              max_offset_y)
{
  g_return_if_fail (GIMP_IS_DISPLAY_SHELL (shell));

  gtk_range_set_lower_stepper_sensitivity (GTK_RANGE (shell->hsb),
                                           min_offset_x < shell->offset_x ?
                                           GTK_SENSITIVITY_ON :
                                           GTK_SENSITIVITY_OFF);

  gtk_range_set_upper_stepper_sensitivity (GTK_RANGE (shell->hsb),
                                           max_offset_x > shell->offset_x ?
                                           GTK_SENSITIVITY_ON :
                                           GTK_SENSITIVITY_OFF);

  gtk_range_set_lower_stepper_sensitivity (GTK_RANGE (shell->vsb),
                                           min_offset_y < shell->offset_y ?
                                           GTK_SENSITIVITY_ON :
                                           GTK_SENSITIVITY_OFF);

  gtk_range_set_upper_stepper_sensitivity (GTK_RANGE (shell->vsb),
                                           max_offset_y > shell->offset_y ?
                                           GTK_SENSITIVITY_ON :
                                           GTK_SENSITIVITY_OFF);
}
