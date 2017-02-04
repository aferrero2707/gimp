/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimp-layer-modes.c
 * Copyright (C) 2017 Michael Natterer <mitch@gimp.org>
 *                    Øyvind Kolås <pippin@gimp.org>
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

#include <glib-object.h>

#include "core-types.h"

#include "gimp-layer-modes.h"


gboolean
gimp_layer_mode_is_legacy (GimpLayerMode  mode)
{
  switch (mode)
    {
    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
    case GIMP_LAYER_MODE_HSV_COLOR_LEGACY:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
    case GIMP_LAYER_MODE_DODGE_LEGACY:
    case GIMP_LAYER_MODE_BURN_LEGACY:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
      return TRUE;

    default:
      break;
    }

  return FALSE;
}

gboolean
gimp_layer_mode_is_linear (GimpLayerMode  mode)
{
  switch (mode)
    {
    case GIMP_LAYER_MODE_NORMAL_NON_LINEAR:
      return FALSE;

    case GIMP_LAYER_MODE_DISSOLVE:
      return TRUE;

    case GIMP_LAYER_MODE_BEHIND:
      return FALSE;

    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
    case GIMP_LAYER_MODE_HSV_COLOR_LEGACY:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
    case GIMP_LAYER_MODE_DODGE_LEGACY:
    case GIMP_LAYER_MODE_BURN_LEGACY:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
      return FALSE;

    case GIMP_LAYER_MODE_COLOR_ERASE:
    case GIMP_LAYER_MODE_OVERLAY:
      return FALSE;

    case GIMP_LAYER_MODE_LCH_HUE:
    case GIMP_LAYER_MODE_LCH_CHROMA:
    case GIMP_LAYER_MODE_LCH_COLOR:
    case GIMP_LAYER_MODE_LCH_LIGHTNESS:
      return TRUE;

    case GIMP_LAYER_MODE_NORMAL:
      return TRUE;

    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_HSV_HUE:
    case GIMP_LAYER_MODE_HSV_SATURATION:
    case GIMP_LAYER_MODE_HSV_COLOR:
    case GIMP_LAYER_MODE_HSV_VALUE:
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_MERGE:
      return TRUE;

    case GIMP_LAYER_MODE_BEHIND_LINEAR:
    case GIMP_LAYER_MODE_MULTIPLY_LINEAR:
    case GIMP_LAYER_MODE_SCREEN_LINEAR:
    case GIMP_LAYER_MODE_OVERLAY_LINEAR:
    case GIMP_LAYER_MODE_DIFFERENCE_LINEAR:
    case GIMP_LAYER_MODE_ADDITION_LINEAR:
    case GIMP_LAYER_MODE_SUBTRACT_LINEAR:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LINEAR:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LINEAR:
    case GIMP_LAYER_MODE_DIVIDE_LINEAR:
    case GIMP_LAYER_MODE_DODGE_LINEAR:
    case GIMP_LAYER_MODE_BURN_LINEAR:
    case GIMP_LAYER_MODE_HARDLIGHT_LINEAR:
    case GIMP_LAYER_MODE_SOFTLIGHT_LINEAR:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR:
      return TRUE;

    case GIMP_LAYER_MODE_ERASE:
      return TRUE;

    case GIMP_LAYER_MODE_REPLACE:
      return FALSE;

    case GIMP_LAYER_MODE_ANTI_ERASE:
      return TRUE;
  }

  return FALSE;
}

GimpLayerColorSpace
gimp_layer_mode_get_blend_space (GimpLayerMode  mode)
{
  switch (mode)
    {
    case GIMP_LAYER_MODE_NORMAL_NON_LINEAR:
      return GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL;

    case GIMP_LAYER_MODE_DISSOLVE:
      return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;

    case GIMP_LAYER_MODE_BEHIND:
      return GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL;

    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
    case GIMP_LAYER_MODE_HSV_COLOR_LEGACY:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
    case GIMP_LAYER_MODE_DODGE_LEGACY:
    case GIMP_LAYER_MODE_BURN_LEGACY:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
      return FALSE;

    case GIMP_LAYER_MODE_COLOR_ERASE:
    case GIMP_LAYER_MODE_OVERLAY:
      return GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL;

    case GIMP_LAYER_MODE_LCH_HUE:
    case GIMP_LAYER_MODE_LCH_CHROMA:
    case GIMP_LAYER_MODE_LCH_COLOR:
    case GIMP_LAYER_MODE_LCH_LIGHTNESS:
      return GIMP_LAYER_COLOR_SPACE_LAB;

    case GIMP_LAYER_MODE_NORMAL:
      return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;

    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_HSV_HUE:
    case GIMP_LAYER_MODE_HSV_SATURATION:
    case GIMP_LAYER_MODE_HSV_COLOR:
    case GIMP_LAYER_MODE_HSV_VALUE:
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_MERGE:
      return GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL;

    case GIMP_LAYER_MODE_BEHIND_LINEAR:
    case GIMP_LAYER_MODE_MULTIPLY_LINEAR:
    case GIMP_LAYER_MODE_SCREEN_LINEAR:
    case GIMP_LAYER_MODE_OVERLAY_LINEAR:
    case GIMP_LAYER_MODE_DIFFERENCE_LINEAR:
    case GIMP_LAYER_MODE_ADDITION_LINEAR:
    case GIMP_LAYER_MODE_SUBTRACT_LINEAR:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LINEAR:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LINEAR:
    case GIMP_LAYER_MODE_DIVIDE_LINEAR:
    case GIMP_LAYER_MODE_DODGE_LINEAR:
    case GIMP_LAYER_MODE_BURN_LINEAR:
    case GIMP_LAYER_MODE_HARDLIGHT_LINEAR:
    case GIMP_LAYER_MODE_SOFTLIGHT_LINEAR:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR:
      return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;

    case GIMP_LAYER_MODE_ERASE:
      return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;

    case GIMP_LAYER_MODE_REPLACE:
      return GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL;

    case GIMP_LAYER_MODE_ANTI_ERASE:
      return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;
  }

  return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;
}

GimpLayerColorSpace
gimp_layer_mode_get_composite_space (GimpLayerMode  mode)
{
  return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;
}

GimpLayerCompositeMode
gimp_layer_mode_get_composite_mode (GimpLayerMode  mode)
{
  return GIMP_LAYER_COMPOSITE_SRC_OVER;
}

const gchar *
gimp_layer_mode_get_operation (GimpLayerMode  mode)
{
  switch (mode)
    {
    case GIMP_LAYER_MODE_NORMAL_NON_LINEAR:
    case GIMP_LAYER_MODE_NORMAL:
      return "gimp:normal";

    case GIMP_LAYER_MODE_DISSOLVE:
      return "gimp:dissolve";

    case GIMP_LAYER_MODE_BEHIND:
    case GIMP_LAYER_MODE_BEHIND_LINEAR:
      return "gimp:behind";

    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
      return "gimp:multiply-legacy";

    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_MULTIPLY_LINEAR:
      return "gimp:multiply";

    case GIMP_LAYER_MODE_SCREEN_LEGACY:
      return "gimp:screen-legacy";

    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_SCREEN_LINEAR:
      return "gimp:screen";

    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
      return "gimp:softlight-legacy";

    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_OVERLAY_LINEAR:
      return "gimp:overlay";

    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
      return "gimp:difference-legacy";

    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_DIFFERENCE_LINEAR:
      return "gimp:difference";

    case GIMP_LAYER_MODE_ADDITION_LEGACY:
      return "gimp:addition-legacy";

    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_ADDITION_LINEAR:
      return "gimp:addition";

    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
      return "gimp:subtract-legacy";

    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_SUBTRACT_LINEAR:
      return "gimp:subtract";

    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
      return "gimp:darken-only-legacy";

    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LINEAR:
      return "gimp:darken-only";

    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
      return "gimp:lighten-only-legacy";

    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LINEAR:
      return "gimp:lighten-only";

    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
      return "gimp:hsv-hue-legacy";

    case GIMP_LAYER_MODE_HSV_HUE:
      return "gimp:hsv-hue";

    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
      return "gimp:hsv-saturation-legacy";

    case GIMP_LAYER_MODE_HSV_SATURATION:
      return "gimp:hsv-saturation";

    case GIMP_LAYER_MODE_HSV_COLOR_LEGACY:
      return "gimp:hsv-color-legacy";

    case GIMP_LAYER_MODE_HSV_COLOR:
      return "gimp:hsv-color";

    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
      return "gimp:hsv-value-legacy";

    case GIMP_LAYER_MODE_HSV_VALUE:
      return "gimp:hsv-value";

    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
      return "gimp:divide-legacy";

    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DIVIDE_LINEAR:
      return "gimp:divide";

    case GIMP_LAYER_MODE_DODGE_LEGACY:
      return "gimp:dodge-legacy";

    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_DODGE_LINEAR:
      return "gimp:dodge";

    case GIMP_LAYER_MODE_BURN_LEGACY:
      return "gimp:burn-legacy";

    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_BURN_LINEAR:
      return "gimp:burn";

    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
      return "gimp:hardlight-legacy";

    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_HARDLIGHT_LINEAR:
      return "gimp:hardlight";

    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
      return "gimp:softlight-legacy";

    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT_LINEAR:
      return "gimp:softlight";

    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
      return "gimp:grain-extract-legacy";

    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR:
      return "gimp:grain-extract";

    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
      return "gimp:grain-merge-legacy";

    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR:
      return "gimp:grain-merge";

    case GIMP_LAYER_MODE_COLOR_ERASE:
      return "gimp:color-erase";

    case GIMP_LAYER_MODE_LCH_HUE:
      return "gimp:lch-hue";

    case GIMP_LAYER_MODE_LCH_CHROMA:
      return "gimp:lch-chroma";

    case GIMP_LAYER_MODE_LCH_COLOR:
      return "gimp:lch-color";

    case GIMP_LAYER_MODE_LCH_LIGHTNESS:
      return "gimp:lch-lightness";

    case GIMP_LAYER_MODE_ERASE:
      return "gimp:erase";

    case GIMP_LAYER_MODE_REPLACE:
      return "gimp:replace";

    case GIMP_LAYER_MODE_ANTI_ERASE:
      return "gimp:anti-erase";
    }

  return NULL;
}
