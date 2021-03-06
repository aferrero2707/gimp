/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimp-babl-compat.h
 * Copyright (C) 2012 Michael Natterer <mitch@gimp.org>
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

#include "gimp-gegl-types.h"

#include "gimp-babl.h"
#include "gimp-babl-compat.h"


GimpImageType
gimp_babl_format_get_image_type (const Babl *format)
{
  const Babl *model;

  g_return_val_if_fail (format != NULL, -1);

  model = babl_format_get_model (format);

  if (model == babl_model ("Y"))
    {
      return GIMP_GRAY_IMAGE;
    }
  else if (model == babl_model ("YA"))
    {
      return GIMP_GRAYA_IMAGE;
    }
  else if (model == babl_model ("RGB"))
    {
      return GIMP_RGB_IMAGE;
    }
  else if (model == babl_model ("RGBA"))
    {
      return GIMP_RGBA_IMAGE;
    }

  g_return_val_if_reached (-1);
}

const Babl *
gimp_babl_compat_u8_format (const Babl *format)
{
  g_return_val_if_fail (format != NULL, NULL);

  return gimp_babl_format (gimp_babl_format_get_base_type (format),
                           GIMP_PRECISION_U8_GAMMA,
                           babl_format_has_alpha (format));
}
