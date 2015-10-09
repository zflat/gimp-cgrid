/* 
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define PLUGIN_RESULT_OK 1
#define PLUGIN_RESULT_WARNING 0
#define PLUGIN_RESULT_ERROR -1

typedef struct
{
  gint      dummy1;
  gint      dummy2;
  gint      dummy3;
  guint     seed;
  gboolean  random_seed;
} PlugInVals;

typedef struct
{
  gint32    image_id;
} PlugInImageVals;

typedef struct
{
  gint32    drawable_id;
} PlugInDrawableVals;

typedef struct
{
  gboolean  chain_active;
} PlugInUIVals;


/*  Default values  */


#define PLUG_IN_FULLNAME "Collection Grid Maker"
#define PLUG_IN_DESCRIPTION "Arrange a collection of images in a grid or strip"
#define PLUG_IN_COPYRIGHT "(C) 2015 - William Wedler"
#define PLUG_IN_WEBSITE "http://wwedler.com"
#define PLUG_IN_BINARY "cgrid"
#define PLUG_IN_PROC "plug-in-cgrid"
#define PLUG_IN_VERSION "0.1.0"

extern const PlugInVals         default_vals;
extern const PlugInImageVals    default_image_vals;
extern const PlugInDrawableVals default_drawable_vals;
extern const PlugInUIVals       default_ui_vals;

GSList *cgrid_input_filenames;
gboolean plugin_is_busy;

#endif /* __MAIN_H__ */
