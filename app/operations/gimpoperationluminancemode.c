/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpoperationluminancemode.c
 * Copyright (C) 2015 Elle Stone <ellestone@ninedegreesbelow.com>
 *                    immanuel <ISchBug@der-ball-ist-rund.net>
 *                    Massimo Valentini <mvalentini@src.gnome.org>
 *                    Thomas Manni <thomas.manni@free.fr>
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
#include <gegl-plugin.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libgimpcolor/gimpcolor.h"

#include "operations-types.h"

#include "gimpoperationluminancemode.h"


static gboolean gimp_operation_luminance_mode_process (GeglOperation       *operation,
                                                           void                *in_buf,
                                                           void                *aux_buf,
                                                           void                *aux2_buf,
                                                           void                *out_buf,
                                                           glong                samples,
                                                           const GeglRectangle *roi,
                                                           gint                 level);

G_DEFINE_TYPE (GimpOperationLuminanceMode, gimp_operation_luminance_mode,
               GIMP_TYPE_OPERATION_POINT_LAYER_MODE)

#define parent_class gimp_operation_luminance_mode_parent_class


static void
gimp_operation_luminance_mode_class_init (GimpOperationLuminanceModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  operation_class->want_in_place = FALSE;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gimp:luminance-mode",
                                 "description", "GIMP Luminance mode operation",
                                 NULL);

  point_class->process = gimp_operation_luminance_mode_process;
}

static void
gimp_operation_luminance_mode_init (GimpOperationLuminanceMode *self)
{
}

static gboolean
gimp_operation_luminance_mode_process (GeglOperation       *operation,
                                           void                *in_buf,
                                           void                *aux_buf,
                                           void                *aux2_buf,
                                           void                *out_buf,
                                           glong                samples,
                                           const GeglRectangle *roi,
                                           gint                 level)
{
  GimpOperationPointLayerMode *gimp_op = GIMP_OPERATION_POINT_LAYER_MODE (operation);
  gfloat                       opacity = gimp_op->opacity;

  return (gimp_operation_luminance_mode_process_pixels)
    (in_buf, aux_buf, aux2_buf, out_buf, opacity, samples, roi, level);
}

static void
luminance_pre_process (const Babl   *format,
                       const gfloat *in,
                       const gfloat *layer,
                       gfloat       *out,
                       glong         samples)
{
  gfloat tmp1[2 * samples], *layer_Y = tmp1;
  gfloat tmp2[2 * samples], *in_Y = tmp2;
  gint i;

  babl_process (babl_fish (format, "RGBA float"), in, out, samples);
  babl_process (babl_fish (format, "YA float"), layer, layer_Y, samples);
  babl_process (babl_fish (format, "YA float"), in, in_Y, samples);

  for (i = 0; i < samples; ++i)
    {
        gfloat ratio = layer_Y[2 * i] / MAX(in_Y[2 * i], 0.0000000000000000001);
        out[4 * i] *= ratio;
        out[4 * i + 1] *= ratio;
        out[4 * i + 2] *= ratio;
    }

  babl_process (babl_fish ("RGBA float", format), out, out, samples);
}

static void
luminance_post_process (const gfloat *in,
                        const gfloat *layer,
                        const gfloat *mask,
                        gfloat       *out,
                        gfloat        opacity,
                        glong         samples)
{
  while (samples--)
    {
      gfloat comp_alpha, new_alpha;

      comp_alpha = MIN (in[ALPHA], layer[ALPHA]) * opacity;
      if (mask)
        comp_alpha *= *mask++;

      new_alpha = in[ALPHA] + (1.0f - in[ALPHA]) * comp_alpha;

      if (comp_alpha && new_alpha)
        {
          gfloat ratio = comp_alpha / new_alpha;

          out[RED]   = out[RED]   * ratio + in[RED]   * (1.0f - ratio);
          out[GREEN] = out[GREEN] * ratio + in[GREEN] * (1.0f - ratio);
          out[BLUE]  = out[BLUE]  * ratio + in[BLUE]  * (1.0f - ratio);
        }
      else
        {
          gint b;

          for (b = RED; b < ALPHA; b++)
            {
              out[b] = in[b];
            }
        }

      out[ALPHA] = in[ALPHA];

      in    += 4;
      layer += 4;
      out   += 4;
    }
}

gboolean
gimp_operation_luminance_mode_process_pixels (gfloat              *in,
                                                  gfloat              *layer,
                                                  gfloat              *mask,
                                                  gfloat              *out,
                                                  gfloat               opacity,
                                                  glong                samples,
                                                  const GeglRectangle *roi,
                                                  gint                 level)
{
  luminance_pre_process (babl_format ("RGBA float"), in, layer, out, samples);
  luminance_post_process (in, layer, mask, out, opacity, samples);

  return TRUE;
}
