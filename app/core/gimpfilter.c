/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpfilter.c
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

#include "core-types.h"

#include "gimp.h"
#include "gimp-utils.h"
#include "gimpfilter.h"


enum
{
  PROP_0,
  PROP_IS_LAST_NODE
};


typedef struct _GimpFilterPrivate GimpFilterPrivate;

struct _GimpFilterPrivate
{
  GeglNode *node;
  gboolean  is_last_node;
};

#define GET_PRIVATE(filter) G_TYPE_INSTANCE_GET_PRIVATE (filter, \
                                                         GIMP_TYPE_FILTER, \
                                                         GimpFilterPrivate)


/*  local function prototypes  */

static void       gimp_filter_finalize      (GObject      *object);
static void       gimp_filter_set_property  (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void       gimp_filter_get_property  (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);

static gint64     gimp_filter_get_memsize   (GimpObject   *object,
                                             gint64       *gui_size);

static GeglNode * gimp_filter_real_get_node (GimpFilter *filter);


G_DEFINE_TYPE (GimpFilter, gimp_filter, GIMP_TYPE_VIEWABLE)

#define parent_class gimp_filter_parent_class


static void
gimp_filter_class_init (GimpFilterClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  GimpObjectClass *gimp_object_class = GIMP_OBJECT_CLASS (klass);

  object_class->finalize         = gimp_filter_finalize;
  object_class->set_property     = gimp_filter_set_property;
  object_class->get_property     = gimp_filter_get_property;

  gimp_object_class->get_memsize = gimp_filter_get_memsize;

  klass->get_node                = gimp_filter_real_get_node;

  g_object_class_install_property (object_class, PROP_IS_LAST_NODE,
                                   g_param_spec_boolean ("is-last-node",
                                                         NULL, NULL,
                                                         FALSE,
                                                         GIMP_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GimpFilterPrivate));
}

static void
gimp_filter_init (GimpFilter *filter)
{
}

static void
gimp_filter_finalize (GObject *object)
{
  GimpFilterPrivate *private = GET_PRIVATE (object);

  if (private->node)
    {
      g_object_unref (private->node);
      private->node = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_filter_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GimpFilterPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IS_LAST_NODE:
      private->is_last_node = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_filter_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GimpFilterPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IS_LAST_NODE:
      g_value_set_boolean (value, private->is_last_node);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
gimp_filter_get_memsize (GimpObject *object,
                         gint64     *gui_size)
{
  GimpFilterPrivate *private = GET_PRIVATE (object);
  gint64             memsize = 0;

  memsize += gimp_g_object_get_memsize (G_OBJECT (private->node));

  return memsize + GIMP_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static GeglNode *
gimp_filter_real_get_node (GimpFilter *filter)
{
  GimpFilterPrivate *private = GET_PRIVATE (filter);

  private->node = gegl_node_new ();

  return private->node;
}


/*  public functions  */

GimpFilter *
gimp_filter_new (const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);

  return g_object_new (GIMP_TYPE_FILTER,
                       "name", name,
                       NULL);
}

GeglNode *
gimp_filter_get_node (GimpFilter *filter)
{
  GimpFilterPrivate *private;

  g_return_val_if_fail (GIMP_IS_FILTER (filter), NULL);

  private = GET_PRIVATE (filter);

  if (private->node)
    return private->node;

  return GIMP_FILTER_GET_CLASS (filter)->get_node (filter);
}

GeglNode *
gimp_filter_peek_node (GimpFilter *filter)
{
  g_return_val_if_fail (GIMP_IS_FILTER (filter), NULL);

  return GET_PRIVATE (filter)->node;
}

void
gimp_filter_set_is_last_node (GimpFilter *filter,
                              gboolean    is_last_node)
{
  GimpFilterPrivate *private;

  g_return_if_fail (GIMP_IS_FILTER (filter));

  private = GET_PRIVATE (filter);

  if (is_last_node != private->is_last_node)
    {
      g_object_set (filter,
                    "is-last-node", is_last_node ? TRUE : FALSE,
                    NULL);
    }
}

gboolean
gimp_filter_get_is_last_node (GimpFilter *filter)
{
  g_return_val_if_fail (GIMP_IS_FILTER (filter), FALSE);

  return GET_PRIVATE (filter)->is_last_node;
}