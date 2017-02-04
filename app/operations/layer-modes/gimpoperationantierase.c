/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpoperationantierase.c
 * Copyright (C) 2008 Michael Natterer <mitch@gimp.org>
 *               2012 Ville Sokk <ville.sokk@gmail.com>
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

#include <gegl-plugin.h>

#include "../operations-types.h"

#include "gimpoperationantierase.h"


static gboolean gimp_operation_anti_erase_process (GeglOperation       *operation,
                                                   void                *in_buf,
                                                   void                *aux_buf,
                                                   void                *aux2_buf,
                                                   void                *out_buf,
                                                   glong                samples,
                                                   const GeglRectangle *roi,
                                                   gint                 level);


G_DEFINE_TYPE (GimpOperationAntiErase, gimp_operation_anti_erase,
               GIMP_TYPE_OPERATION_POINT_LAYER_MODE)


static void
gimp_operation_anti_erase_class_init (GimpOperationAntiEraseClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "gimp:anti-erase",
                                 "description", "GIMP anti erase mode operation",
                                 NULL);

  point_class->process = gimp_operation_anti_erase_process;
}

static void
gimp_operation_anti_erase_init (GimpOperationAntiErase *self)
{
}

static gboolean
gimp_operation_anti_erase_process (GeglOperation       *operation,
                                   void                *in_buf,
                                   void                *aux_buf,
                                   void                *aux2_buf,
                                   void                *out_buf,
                                   glong                samples,
                                   const GeglRectangle *roi,
                                   gint                 level)
{
  GimpOperationPointLayerMode *layer_mode = (gpointer) operation;

  return gimp_operation_anti_erase_process_pixels (in_buf, aux_buf, aux2_buf,
                                                   out_buf,
                                                   layer_mode->opacity,
                                                   samples, roi, level,
                                                   layer_mode->blend_trc,
                                                   layer_mode->composite_trc,
                                                   layer_mode->composite_mode);
}

gboolean
gimp_operation_anti_erase_process_pixels (gfloat                *in,
                                          gfloat                *layer,
                                          gfloat                *mask,
                                          gfloat                *out,
                                          gfloat                 opacity,
                                          glong                  samples,
                                          const GeglRectangle   *roi,
                                          gint                   level,
                                          GimpLayerColorSpace    blend_trc,
                                          GimpLayerColorSpace    composite_trc,
                                          GimpLayerCompositeMode composite_mode)
{
  const gboolean has_mask = mask != NULL;

  while (samples--)
    {
      gfloat value = opacity;
      gint   b;

      if (has_mask)
        value *= *mask;

      out[ALPHA] = in[ALPHA] + (1.0 - in[ALPHA]) * layer[ALPHA] * value;

      for (b = RED; b < ALPHA; b++)
        {
          out[b] = in[b];
        }

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}
