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

#include "../operations-types.h"

#include "operations/layer-modes-legacy/gimpoperationadditionlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationburnlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationdarkenonlylegacy.h"
#include "operations/layer-modes-legacy/gimpoperationdifferencelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationdividelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationdodgelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationgrainextractlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationgrainmergelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationhardlightlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationhsvcolorlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationhsvhuelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationhsvsaturationlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationhsvvaluelegacy.h"
#include "operations/layer-modes-legacy/gimpoperationlightenonlylegacy.h"
#include "operations/layer-modes-legacy/gimpoperationmultiplylegacy.h"
#include "operations/layer-modes-legacy/gimpoperationscreenlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationsoftlightlegacy.h"
#include "operations/layer-modes-legacy/gimpoperationsubtractlegacy.h"

#include "gimpoperationantierase.h"
#include "gimpoperationbehind.h"
#include "gimpoperationcolorerase.h"
#include "gimpoperationdissolve.h"
#include "gimpoperationerase.h"
#include "gimpoperationnormal.h"
#include "gimpoperationreplace.h"

#include "gimp-layer-modes.h"


typedef struct _GimpLayerModeInfo GimpLayerModeInfo;

struct _GimpLayerModeInfo
{
  GimpLayerMode          layer_mode;
  const gchar           *op_name;
  GimpLayerModeFunc      function;
  GimpLayerModeFlags     flags;
  GimpLayerCompositeMode paint_composite_mode;
  GimpLayerCompositeMode composite_mode;
  GimpLayerColorSpace    composite_space;
  GimpLayerColorSpace    blend_space;
};


/*  static variables  */

static const GimpLayerModeInfo layer_mode_infos[] =
{
  { GIMP_LAYER_MODE_NORMAL,
    "gimp:normal",
    gimp_operation_normal_process,
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
  },

  { GIMP_LAYER_MODE_DISSOLVE,
    "gimp:dissolve",
    gimp_operation_dissolve_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA     |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
  },

  { GIMP_LAYER_MODE_BEHIND,
    "gimp:behind",
    gimp_operation_behind_process,
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_DST_ATOP,
    GIMP_LAYER_COMPOSITE_DST_ATOP
  },

  { GIMP_LAYER_MODE_MULTIPLY_LEGACY,
    "gimp:multiply-legacy",
    gimp_operation_multiply_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_SCREEN_LEGACY,
    "gimp:screen-legacy",
    gimp_operation_screen_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_OVERLAY_LEGACY,
    "gimp:softlight-legacy",
    gimp_operation_softlight_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_DIFFERENCE_LEGACY,
    "gimp:difference-legacy",
    gimp_operation_difference_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_ADDITION_LEGACY,
    "gimp:addition-legacy",
    gimp_operation_addition_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_SUBTRACT_LEGACY,
    "gimp:subtract-legacy",
    gimp_operation_subtract_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY,
    "gimp:darken-only-legacy",
    gimp_operation_darken_only_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY,
    "gimp:lighten-only-legacy",
    gimp_operation_lighten_only_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_HSV_HUE_LEGACY,
    "gimp:hsv-hue-legacy",
    gimp_operation_hsv_hue_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_HSV_SATURATION_LEGACY,
    "gimp:hsv-saturation-legacy",
    gimp_operation_hsv_saturation_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_HSV_COLOR_LEGACY,
    "gimp:hsv-color-legacy",
    gimp_operation_hsv_color_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_HSV_VALUE_LEGACY,
    "gimp:hsv-value-legacy",
    gimp_operation_hsv_value_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_DIVIDE_LEGACY,
    "gimp:divide-legacy",
    gimp_operation_divide_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_DODGE_LEGACY,
    "gimp:dodge-legacy",
    gimp_operation_dodge_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_BURN_LEGACY,
    "gimp:burn-legacy",
    gimp_operation_burn_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_HARDLIGHT_LEGACY,
    "gimp:hardlight-legacy",
    gimp_operation_hardlight_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_SOFTLIGHT_LEGACY,
    "gimp:softlight-legacy",
    gimp_operation_softlight_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY,
    "gimp:grain-extract-legacy",
    gimp_operation_grain_extract_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY,
    "gimp:grain-merge-legacy",
    gimp_operation_grain_merge_legacy_process,
    GIMP_LAYER_MODE_FLAG_LEGACY                    |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE     |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_COLOR_ERASE,
    "gimp:color-erase",
    gimp_operation_color_erase_process,
    0,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
  },

  { GIMP_LAYER_MODE_OVERLAY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_LCH_HUE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_LAB
  },

  { GIMP_LAYER_MODE_LCH_CHROMA,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_LAB
  },

  { GIMP_LAYER_MODE_LCH_COLOR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_LAB
  },

  { GIMP_LAYER_MODE_LCH_LIGHTNESS,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_LAB
  },

  { GIMP_LAYER_MODE_NORMAL_LINEAR,
    "gimp:normal",
    gimp_operation_normal_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
  },

  { GIMP_LAYER_MODE_BEHIND_LINEAR,
    "gimp:behind",
    gimp_operation_behind_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_DST_ATOP,
    GIMP_LAYER_COMPOSITE_DST_ATOP,
  },

  { GIMP_LAYER_MODE_MULTIPLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_MULTIPLY_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_SCREEN,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_SCREEN_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_OVERLAY_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_DIFFERENCE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_DIFFERENCE_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_ADDITION,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_ADDITION_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_SUBTRACT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_SUBTRACT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_DARKEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_LIGHTEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_HSV_HUE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_HSV_SATURATION,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_HSV_COLOR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_HSV_VALUE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_DIVIDE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_DIVIDE_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_DODGE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_DODGE_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_BURN,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_BURN_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_HARDLIGHT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_HARDLIGHT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_SOFTLIGHT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_SOFTLIGHT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_GRAIN_EXTRACT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_GRAIN_MERGE,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_VIVID_LIGHT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_VIVID_LIGHT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_PIN_LIGHT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_PIN_LIGHT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_LINEAR_LIGHT,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_LINEAR_LIGHT_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_HARD_MIX,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_HARD_MIX_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_EXCLUSION,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_EXCLUSION_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_LINEAR_BURN,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_LINEAR_BURN_LINEAR,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_LUMA_DARKEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_LUMINANCE_DARKEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_PERCEPTUAL
  },

  { GIMP_LAYER_MODE_LUMINANCE_LIGHTEN_ONLY,
    "gimp:layer-mode",
    gimp_operation_layer_mode_process_pixels,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR,
    GIMP_LAYER_COLOR_SPACE_RGB_LINEAR
  },

  { GIMP_LAYER_MODE_ERASE,
    "gimp:erase",
    gimp_operation_erase_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP
  },

  { GIMP_LAYER_MODE_REPLACE,
    "gimp:replace",
    gimp_operation_replace_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
    GIMP_LAYER_COMPOSITE_SRC_OVER,
  },

  { GIMP_LAYER_MODE_ANTI_ERASE,
    "gimp:anti-erase",
    gimp_operation_anti_erase_process,
    GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA     |
    GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE |
    GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE,
    GIMP_LAYER_COMPOSITE_SRC_ATOP,
    GIMP_LAYER_COMPOSITE_SRC_ATOP
  }
};

static const GimpLayerMode layer_mode_group_default[] =
{
  GIMP_LAYER_MODE_NORMAL,
  GIMP_LAYER_MODE_DISSOLVE,

  GIMP_LAYER_MODE_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_LUMINANCE_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_SCREEN,
  GIMP_LAYER_MODE_DODGE,
  GIMP_LAYER_MODE_ADDITION,

  GIMP_LAYER_MODE_DARKEN_ONLY,
  GIMP_LAYER_MODE_LUMINANCE_DARKEN_ONLY,
  GIMP_LAYER_MODE_MULTIPLY,
  GIMP_LAYER_MODE_BURN,

  GIMP_LAYER_MODE_OVERLAY,
  GIMP_LAYER_MODE_SOFTLIGHT,
  GIMP_LAYER_MODE_HARDLIGHT,
  GIMP_LAYER_MODE_VIVID_LIGHT,
  GIMP_LAYER_MODE_PIN_LIGHT,
  GIMP_LAYER_MODE_LINEAR_LIGHT,
  GIMP_LAYER_MODE_HARD_MIX,

  GIMP_LAYER_MODE_DIFFERENCE,
  GIMP_LAYER_MODE_SUBTRACT,
  GIMP_LAYER_MODE_GRAIN_EXTRACT,
  GIMP_LAYER_MODE_GRAIN_MERGE,
  GIMP_LAYER_MODE_DIVIDE,

  GIMP_LAYER_MODE_LCH_HUE,
  GIMP_LAYER_MODE_LCH_CHROMA,
  GIMP_LAYER_MODE_LCH_COLOR,
  GIMP_LAYER_MODE_LCH_LIGHTNESS,

  GIMP_LAYER_MODE_EXCLUSION,
  GIMP_LAYER_MODE_LINEAR_BURN
};

static const GimpLayerMode layer_mode_group_linear[] =
{
  GIMP_LAYER_MODE_NORMAL_LINEAR,
  GIMP_LAYER_MODE_DISSOLVE,

  GIMP_LAYER_MODE_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_LUMINANCE_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_SCREEN_LINEAR,
  GIMP_LAYER_MODE_DODGE_LINEAR,
  GIMP_LAYER_MODE_ADDITION_LINEAR,

  GIMP_LAYER_MODE_DARKEN_ONLY,
  GIMP_LAYER_MODE_LUMINANCE_DARKEN_ONLY,
  GIMP_LAYER_MODE_MULTIPLY_LINEAR,
  GIMP_LAYER_MODE_BURN_LINEAR,

  GIMP_LAYER_MODE_OVERLAY_LINEAR,
  GIMP_LAYER_MODE_SOFTLIGHT_LINEAR,
  GIMP_LAYER_MODE_HARDLIGHT_LINEAR,
  GIMP_LAYER_MODE_VIVID_LIGHT_LINEAR,
  GIMP_LAYER_MODE_PIN_LIGHT_LINEAR,
  GIMP_LAYER_MODE_LINEAR_LIGHT_LINEAR,
  GIMP_LAYER_MODE_HARD_MIX_LINEAR,

  GIMP_LAYER_MODE_DIFFERENCE_LINEAR,
  GIMP_LAYER_MODE_SUBTRACT_LINEAR,
  GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR,
  GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR,
  GIMP_LAYER_MODE_DIVIDE_LINEAR,

  GIMP_LAYER_MODE_EXCLUSION_LINEAR,
  GIMP_LAYER_MODE_LINEAR_BURN_LINEAR
};

static const GimpLayerMode layer_mode_group_perceptual[] =
{
  GIMP_LAYER_MODE_NORMAL,
  GIMP_LAYER_MODE_DISSOLVE,

  GIMP_LAYER_MODE_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY,
  GIMP_LAYER_MODE_SCREEN,
  GIMP_LAYER_MODE_DODGE,
  GIMP_LAYER_MODE_ADDITION,

  GIMP_LAYER_MODE_DARKEN_ONLY,
  GIMP_LAYER_MODE_LUMA_DARKEN_ONLY,
  GIMP_LAYER_MODE_MULTIPLY,
  GIMP_LAYER_MODE_BURN,

  GIMP_LAYER_MODE_OVERLAY,
  GIMP_LAYER_MODE_SOFTLIGHT,
  GIMP_LAYER_MODE_HARDLIGHT,
  GIMP_LAYER_MODE_VIVID_LIGHT,
  GIMP_LAYER_MODE_PIN_LIGHT,
  GIMP_LAYER_MODE_LINEAR_LIGHT,
  GIMP_LAYER_MODE_HARD_MIX,

  GIMP_LAYER_MODE_DIFFERENCE,
  GIMP_LAYER_MODE_SUBTRACT,
  GIMP_LAYER_MODE_GRAIN_EXTRACT,
  GIMP_LAYER_MODE_GRAIN_MERGE,
  GIMP_LAYER_MODE_DIVIDE,

  GIMP_LAYER_MODE_HSV_HUE,
  GIMP_LAYER_MODE_HSV_SATURATION,
  GIMP_LAYER_MODE_HSV_COLOR,
  GIMP_LAYER_MODE_HSV_VALUE,

  GIMP_LAYER_MODE_LCH_HUE,
  GIMP_LAYER_MODE_LCH_CHROMA,
  GIMP_LAYER_MODE_LCH_COLOR,
  GIMP_LAYER_MODE_LCH_LIGHTNESS,

  GIMP_LAYER_MODE_EXCLUSION,
  GIMP_LAYER_MODE_LINEAR_BURN
};

static const GimpLayerMode layer_mode_group_legacy[] =
{
  GIMP_LAYER_MODE_NORMAL,
  GIMP_LAYER_MODE_DISSOLVE,

  GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY,
  GIMP_LAYER_MODE_SCREEN_LEGACY,
  GIMP_LAYER_MODE_DODGE_LEGACY,
  GIMP_LAYER_MODE_ADDITION_LEGACY,

  GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY,
  GIMP_LAYER_MODE_MULTIPLY_LEGACY,
  GIMP_LAYER_MODE_BURN_LEGACY,

  GIMP_LAYER_MODE_SOFTLIGHT_LEGACY,
  GIMP_LAYER_MODE_HARDLIGHT_LEGACY,

  GIMP_LAYER_MODE_DIFFERENCE_LEGACY,
  GIMP_LAYER_MODE_SUBTRACT_LEGACY,
  GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY,
  GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY,
  GIMP_LAYER_MODE_DIVIDE_LEGACY,

  GIMP_LAYER_MODE_HSV_HUE_LEGACY,
  GIMP_LAYER_MODE_HSV_SATURATION_LEGACY,
  GIMP_LAYER_MODE_HSV_COLOR_LEGACY,
  GIMP_LAYER_MODE_HSV_VALUE_LEGACY
};

static const GimpLayerMode layer_mode_groups[][4] =
{
  { GIMP_LAYER_MODE_NORMAL,
    GIMP_LAYER_MODE_NORMAL_LINEAR,
    GIMP_LAYER_MODE_NORMAL,
    GIMP_LAYER_MODE_NORMAL
  },

  { GIMP_LAYER_MODE_DISSOLVE,
    GIMP_LAYER_MODE_DISSOLVE,
    GIMP_LAYER_MODE_DISSOLVE,
    GIMP_LAYER_MODE_DISSOLVE
  },

  { GIMP_LAYER_MODE_BEHIND,
    GIMP_LAYER_MODE_BEHIND_LINEAR,
    GIMP_LAYER_MODE_BEHIND,
    GIMP_LAYER_MODE_BEHIND
  },

  { GIMP_LAYER_MODE_MULTIPLY,
    GIMP_LAYER_MODE_MULTIPLY_LINEAR,
    GIMP_LAYER_MODE_MULTIPLY,
    GIMP_LAYER_MODE_MULTIPLY_LEGACY
  },

  { GIMP_LAYER_MODE_SCREEN,
    GIMP_LAYER_MODE_SCREEN_LINEAR,
    GIMP_LAYER_MODE_SCREEN,
    GIMP_LAYER_MODE_SCREEN_LEGACY
  },

  { GIMP_LAYER_MODE_OVERLAY,
    GIMP_LAYER_MODE_OVERLAY_LINEAR,
    GIMP_LAYER_MODE_OVERLAY,
    -1
  },

  { GIMP_LAYER_MODE_DIFFERENCE,
    GIMP_LAYER_MODE_DIFFERENCE_LINEAR,
    GIMP_LAYER_MODE_DIFFERENCE,
    GIMP_LAYER_MODE_DIFFERENCE_LEGACY
  },

  { GIMP_LAYER_MODE_ADDITION,
    GIMP_LAYER_MODE_ADDITION_LINEAR,
    GIMP_LAYER_MODE_ADDITION,
    GIMP_LAYER_MODE_ADDITION_LEGACY
  },

  { GIMP_LAYER_MODE_SUBTRACT,
    GIMP_LAYER_MODE_SUBTRACT_LINEAR,
    GIMP_LAYER_MODE_SUBTRACT,
    GIMP_LAYER_MODE_SUBTRACT_LEGACY
  },

  { GIMP_LAYER_MODE_DARKEN_ONLY,
    GIMP_LAYER_MODE_DARKEN_ONLY,
    GIMP_LAYER_MODE_DARKEN_ONLY,
    GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY
  },

  { GIMP_LAYER_MODE_LIGHTEN_ONLY,
    GIMP_LAYER_MODE_LIGHTEN_ONLY,
    GIMP_LAYER_MODE_LIGHTEN_ONLY,
    GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY
  },

  { -1,
    -1,
    GIMP_LAYER_MODE_HSV_HUE,
    GIMP_LAYER_MODE_HSV_HUE_LEGACY
  },

  { -1,
    -1,
    GIMP_LAYER_MODE_HSV_SATURATION,
    GIMP_LAYER_MODE_HSV_SATURATION_LEGACY
  },

  { -1,
    -1,
    GIMP_LAYER_MODE_HSV_COLOR,
    GIMP_LAYER_MODE_HSV_COLOR_LEGACY
  },

  { -1,
    -1,
    GIMP_LAYER_MODE_HSV_VALUE,
    GIMP_LAYER_MODE_HSV_VALUE_LEGACY
  },

  { GIMP_LAYER_MODE_DIVIDE,
    GIMP_LAYER_MODE_DIVIDE_LINEAR,
    GIMP_LAYER_MODE_DIVIDE,
    GIMP_LAYER_MODE_DIVIDE_LEGACY
  },

  { GIMP_LAYER_MODE_DODGE,
    GIMP_LAYER_MODE_DODGE_LINEAR,
    GIMP_LAYER_MODE_DODGE,
    GIMP_LAYER_MODE_DODGE_LEGACY
  },

  { GIMP_LAYER_MODE_BURN,
    GIMP_LAYER_MODE_BURN_LINEAR,
    GIMP_LAYER_MODE_BURN,
    GIMP_LAYER_MODE_BURN_LEGACY
  },

  { GIMP_LAYER_MODE_HARDLIGHT,
    GIMP_LAYER_MODE_HARDLIGHT_LINEAR,
    GIMP_LAYER_MODE_HARDLIGHT,
    GIMP_LAYER_MODE_HARDLIGHT_LEGACY
  },

  { GIMP_LAYER_MODE_SOFTLIGHT,
    GIMP_LAYER_MODE_SOFTLIGHT_LINEAR,
    GIMP_LAYER_MODE_SOFTLIGHT,
    GIMP_LAYER_MODE_SOFTLIGHT_LEGACY
  },

  { GIMP_LAYER_MODE_GRAIN_EXTRACT,
    GIMP_LAYER_MODE_GRAIN_EXTRACT_LINEAR,
    GIMP_LAYER_MODE_GRAIN_EXTRACT,
    GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY
  },

  { GIMP_LAYER_MODE_GRAIN_MERGE,
    GIMP_LAYER_MODE_GRAIN_MERGE_LINEAR,
    GIMP_LAYER_MODE_GRAIN_MERGE,
    GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY
  },

  { GIMP_LAYER_MODE_COLOR_ERASE,
    -1,
    GIMP_LAYER_MODE_COLOR_ERASE,
    -1,
  },

  { GIMP_LAYER_MODE_VIVID_LIGHT,
    GIMP_LAYER_MODE_VIVID_LIGHT_LINEAR,
    GIMP_LAYER_MODE_VIVID_LIGHT,
    -1
  },

  { GIMP_LAYER_MODE_PIN_LIGHT,
    GIMP_LAYER_MODE_PIN_LIGHT_LINEAR,
    GIMP_LAYER_MODE_PIN_LIGHT,
    -1
  },

  { GIMP_LAYER_MODE_LINEAR_LIGHT,
    GIMP_LAYER_MODE_LINEAR_LIGHT_LINEAR,
    GIMP_LAYER_MODE_LINEAR_LIGHT,
    -1
  },

  { GIMP_LAYER_MODE_HARD_MIX,
    GIMP_LAYER_MODE_HARD_MIX_LINEAR,
    GIMP_LAYER_MODE_HARD_MIX,
    -1
  },

  { GIMP_LAYER_MODE_EXCLUSION,
    GIMP_LAYER_MODE_EXCLUSION_LINEAR,
    GIMP_LAYER_MODE_EXCLUSION,
    -1
  },

  { GIMP_LAYER_MODE_LINEAR_BURN,
    GIMP_LAYER_MODE_LINEAR_BURN_LINEAR,
    GIMP_LAYER_MODE_LINEAR_BURN,
    -1
  },

  { GIMP_LAYER_MODE_LUMINANCE_DARKEN_ONLY,
    GIMP_LAYER_MODE_LUMINANCE_DARKEN_ONLY,
    GIMP_LAYER_MODE_LUMA_DARKEN_ONLY,
    -1
  },

  { GIMP_LAYER_MODE_LUMINANCE_LIGHTEN_ONLY,
    GIMP_LAYER_MODE_LUMINANCE_LIGHTEN_ONLY,
    GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY,
    -1
  },

  { GIMP_LAYER_MODE_ERASE,
    GIMP_LAYER_MODE_ERASE,
    -1,
    -1
  },

  { GIMP_LAYER_MODE_REPLACE,
    GIMP_LAYER_MODE_REPLACE,
    -1,
    -1
  },

  { GIMP_LAYER_MODE_ANTI_ERASE,
    GIMP_LAYER_MODE_ANTI_ERASE,
    -1,
    -1
  }
};


/*  public functions  */

void
gimp_layer_modes_init (void)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (layer_mode_infos); i++)
    {
      g_assert ((GimpLayerMode) i == layer_mode_infos[i].layer_mode);
    }
}

static const GimpLayerModeInfo *
gimp_layer_mode_info (GimpLayerMode mode)
{
  g_return_val_if_fail (mode < G_N_ELEMENTS (layer_mode_infos),
                        &layer_mode_infos[0]);

  return &layer_mode_infos[mode];
}

gboolean
gimp_layer_mode_is_legacy (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return FALSE;
  return (info->flags & GIMP_LAYER_MODE_FLAG_LEGACY) != 0;
}

gboolean
gimp_layer_mode_wants_linear_data (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return FALSE;
  return (info->flags & GIMP_LAYER_MODE_FLAG_WANTS_LINEAR_DATA) != 0;
}

gboolean
gimp_layer_mode_is_blend_space_mutable (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return FALSE;
  return (info->flags & GIMP_LAYER_MODE_FLAG_BLEND_SPACE_IMMUTABLE) == 0;
}

gboolean
gimp_layer_mode_is_composite_space_mutable (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return FALSE;
  return (info->flags & GIMP_LAYER_MODE_FLAG_COMPOSITE_SPACE_IMMUTABLE) == 0;
}

gboolean
gimp_layer_mode_is_composite_mode_mutable (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return FALSE;
  return (info->flags & GIMP_LAYER_MODE_FLAG_COMPOSITE_MODE_IMMUTABLE) == 0;
}

GimpLayerColorSpace
gimp_layer_mode_get_blend_space (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;
  return info->blend_space;
}

GimpLayerColorSpace
gimp_layer_mode_get_composite_space (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return GIMP_LAYER_COLOR_SPACE_RGB_LINEAR;
  return info->composite_space;
}

GimpLayerCompositeMode
gimp_layer_mode_get_composite_mode (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return GIMP_LAYER_COMPOSITE_SRC_OVER;
  return info->composite_mode;
}

GimpLayerCompositeMode
gimp_layer_mode_get_paint_composite_mode (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return GIMP_LAYER_COMPOSITE_SRC_OVER;
  return info->paint_composite_mode;
}

const gchar *
gimp_layer_mode_get_operation (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return "gimp:layer-mode";
  return info->op_name;
}

GimpLayerModeFunc
gimp_layer_mode_get_function (GimpLayerMode  mode)
{
  const GimpLayerModeInfo *info = gimp_layer_mode_info (mode);
  if (!info)
    return gimp_operation_layer_mode_process_pixels;
  return info->function;
}

static gboolean
is_mode_in_array (const GimpLayerMode *modes,
                  gint                 n_modes,
                  GimpLayerMode        mode)
{
  gint i;

  for (i = 0; i < n_modes; i++)
    if (modes[i] == mode)
      return TRUE;

  return FALSE;
}

GimpLayerModeGroup
gimp_layer_mode_get_group (GimpLayerMode  mode)
{
  if (is_mode_in_array (layer_mode_group_default,
                        G_N_ELEMENTS (layer_mode_group_default), mode))
    {
      return GIMP_LAYER_MODE_GROUP_DEFAULT;
    }
  else if (is_mode_in_array (layer_mode_group_linear,
                             G_N_ELEMENTS (layer_mode_group_linear), mode))
    {
      return GIMP_LAYER_MODE_GROUP_LINEAR;
    }
  else if (is_mode_in_array (layer_mode_group_perceptual,
                             G_N_ELEMENTS (layer_mode_group_perceptual), mode))
    {
      return GIMP_LAYER_MODE_GROUP_PERCEPTUAL;
    }
  else if (is_mode_in_array (layer_mode_group_legacy,
                             G_N_ELEMENTS (layer_mode_group_legacy), mode))
    {
      return GIMP_LAYER_MODE_GROUP_LEGACY;
    }

  return GIMP_LAYER_MODE_GROUP_DEFAULT;
}

const GimpLayerMode *
gimp_layer_mode_get_group_array (GimpLayerModeGroup  group,
                                 gint               *n_modes)
{
  g_return_val_if_fail (n_modes != NULL, NULL);

  switch (group)
    {
    case GIMP_LAYER_MODE_GROUP_DEFAULT:
      *n_modes = G_N_ELEMENTS (layer_mode_group_default);
      return layer_mode_group_default;

    case GIMP_LAYER_MODE_GROUP_LINEAR:
      *n_modes = G_N_ELEMENTS (layer_mode_group_linear);
      return layer_mode_group_linear;

    case GIMP_LAYER_MODE_GROUP_PERCEPTUAL:
      *n_modes = G_N_ELEMENTS (layer_mode_group_perceptual);
      return layer_mode_group_perceptual;

    case GIMP_LAYER_MODE_GROUP_LEGACY:
      *n_modes = G_N_ELEMENTS (layer_mode_group_legacy);
      return layer_mode_group_legacy;

    default:
      g_return_val_if_reached (NULL);
    }
}

gboolean
gimp_layer_mode_get_for_group (GimpLayerMode       old_mode,
                               GimpLayerModeGroup  new_group,
                               GimpLayerMode      *new_mode)
{
  gint i;

  g_return_val_if_fail (new_mode != NULL, FALSE);

  for (i = 0; i < G_N_ELEMENTS (layer_mode_groups); i++)
    {
      if (is_mode_in_array (layer_mode_groups[i], 4, old_mode))
        {
          *new_mode = layer_mode_groups[i][new_group];

          if (*new_mode != -1)
            return TRUE;

          return FALSE;
        }
    }

  *new_mode = -1;

  return FALSE;
}
