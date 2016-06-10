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

#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gegl.h>

#include "libgimpcolor/gimpcolor.h"

#include "core-types.h"

#include "gegl/gimp-gegl-apply-operation.h"
#include "gegl/gimp-gegl-utils.h"

#include "gimp-utils.h"
#include "gimpbezierdesc.h"
#include "gimpchannel.h"
#include "gimpdrawable-fill.h"
#include "gimperror.h"
#include "gimpfilloptions.h"
#include "gimpimage.h"
#include "gimppattern.h"
#include "gimppickable.h"
#include "gimpscanconvert.h"

#include "vectors/gimpvectors.h"

#include "gimp-intl.h"


/*  public functions  */

void
gimp_drawable_fill (GimpDrawable *drawable,
                    GimpContext  *context,
                    GimpFillType  fill_type)
{
  GimpRGB      color;
  GimpPattern *pattern;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (GIMP_IS_CONTEXT (context));

  if (! gimp_get_fill_params (context, fill_type, &color, &pattern, NULL))
    return;

  if (pattern)
    {
      GeglBuffer *src_buffer = gimp_pattern_create_buffer (pattern);

      gegl_buffer_set_pattern (gimp_drawable_get_buffer (drawable),
                               NULL, src_buffer, 0, 0);
      g_object_unref (src_buffer);
    }
  else
    {
      GeglColor *gegl_color;

      gimp_pickable_srgb_to_image_color (GIMP_PICKABLE (drawable),
                                         &color, &color);

      if (! gimp_drawable_has_alpha (drawable))
        gimp_rgb_set_alpha (&color, 1.0);

      gegl_color = gimp_gegl_color_new (&color);
      gegl_buffer_set_color (gimp_drawable_get_buffer (drawable),
                             NULL, gegl_color);
      g_object_unref (gegl_color);
    }

  gimp_drawable_update (drawable,
                        0, 0,
                        gimp_item_get_width  (GIMP_ITEM (drawable)),
                        gimp_item_get_height (GIMP_ITEM (drawable)));
}

void
gimp_drawable_fill_boundary (GimpDrawable       *drawable,
                             GimpFillOptions    *options,
                             const GimpBoundSeg *bound_segs,
                             gint                n_bound_segs,
                             gint                offset_x,
                             gint                offset_y,
                             gboolean            push_undo)
{
  GimpScanConvert *scan_convert;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (gimp_item_is_attached (GIMP_ITEM (drawable)));
  g_return_if_fail (GIMP_IS_FILL_OPTIONS (options));
  g_return_if_fail (bound_segs == NULL || n_bound_segs != 0);
  g_return_if_fail (gimp_fill_options_get_style (options) !=
                    GIMP_FILL_STYLE_PATTERN ||
                    gimp_context_get_pattern (GIMP_CONTEXT (options)) != NULL);

  scan_convert = gimp_scan_convert_new_from_boundary (bound_segs, n_bound_segs,
                                                      offset_x, offset_y);

  if (scan_convert)
    {
      gimp_drawable_fill_scan_convert (drawable, options,
                                       scan_convert, push_undo);
      gimp_scan_convert_free (scan_convert);
    }
}

gboolean
gimp_drawable_fill_vectors (GimpDrawable     *drawable,
                            GimpFillOptions  *options,
                            GimpVectors      *vectors,
                            gboolean          push_undo,
                            GError          **error)
{
  const GimpBezierDesc *bezier;

  g_return_val_if_fail (GIMP_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (gimp_item_is_attached (GIMP_ITEM (drawable)), FALSE);
  g_return_val_if_fail (GIMP_IS_FILL_OPTIONS (options), FALSE);
  g_return_val_if_fail (GIMP_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (gimp_fill_options_get_style (options) !=
                        GIMP_FILL_STYLE_PATTERN ||
                        gimp_context_get_pattern (GIMP_CONTEXT (options)) != NULL,
                        FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  bezier = gimp_vectors_get_bezier (vectors);

  if (bezier && bezier->num_data > 4)
    {
      GimpScanConvert *scan_convert = gimp_scan_convert_new ();

      gimp_scan_convert_add_bezier (scan_convert, bezier);
      gimp_drawable_fill_scan_convert (drawable, options,
                                       scan_convert, push_undo);

      gimp_scan_convert_free (scan_convert);

      return TRUE;
    }

  g_set_error_literal (error, GIMP_ERROR, GIMP_FAILED,
                       _("Not enough points to fill"));

  return FALSE;
}

void
gimp_drawable_fill_scan_convert (GimpDrawable    *drawable,
                                 GimpFillOptions *options,
                                 GimpScanConvert *scan_convert,
                                 gboolean         push_undo)
{
  GimpContext *context;
  GimpImage   *image;
  GeglBuffer  *buffer;
  GeglBuffer  *mask_buffer;
  gint         x, y, w, h;
  gint         off_x;
  gint         off_y;

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (gimp_item_is_attached (GIMP_ITEM (drawable)));
  g_return_if_fail (GIMP_IS_FILL_OPTIONS (options));
  g_return_if_fail (scan_convert != NULL);
  g_return_if_fail (gimp_fill_options_get_style (options) !=
                    GIMP_FILL_STYLE_PATTERN ||
                    gimp_context_get_pattern (GIMP_CONTEXT (options)) != NULL);

  context = GIMP_CONTEXT (options);
  image   = gimp_item_get_image (GIMP_ITEM (drawable));

  /*  must call gimp_channel_is_empty() instead of relying on
   *  gimp_item_mask_intersect() because the selection pretends to
   *  be empty while it is being stroked, to prevent masking itself.
   */
  if (gimp_channel_is_empty (gimp_image_get_mask (image)))
    {
      x = 0;
      y = 0;
      w = gimp_item_get_width  (GIMP_ITEM (drawable));
      h = gimp_item_get_height (GIMP_ITEM (drawable));
    }
  else if (! gimp_item_mask_intersect (GIMP_ITEM (drawable), &x, &y, &w, &h))
    {
      return;
    }

  /* fill a 1-bpp GeglBuffer with black, this will describe the shape
   * of the stroke.
   */
  mask_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, w, h),
                                 babl_format ("Y u8"));

  /* render the stroke into it */
  gimp_item_get_offset (GIMP_ITEM (drawable), &off_x, &off_y);

  gimp_scan_convert_render (scan_convert, mask_buffer,
                            x + off_x, y + off_y,
                            gimp_fill_options_get_antialias (options));

  buffer = gimp_fill_options_create_buffer (options, drawable,
                                            GEGL_RECTANGLE (0, 0, w, h));

  gimp_gegl_apply_opacity (buffer, NULL, NULL, buffer,
                           mask_buffer, 0, 0, 1.0);
  g_object_unref (mask_buffer);

  /* Apply to drawable */
  gimp_drawable_apply_buffer (drawable, buffer,
                              GEGL_RECTANGLE (0, 0, w, h),
                              push_undo, C_("undo-type", "Render Stroke"),
                              gimp_context_get_opacity (context),
                              gimp_context_get_paint_mode (context),
                              NULL, x, y);

  g_object_unref (buffer);

  gimp_drawable_update (drawable, x, y, w, h);
}
