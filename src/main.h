/* 
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define PLUGIN_RESULT_OK 1
#define PLUGIN_RESULT_WARNING 0
#define PLUGIN_RESULT_ERROR -1

typedef struct{
  gint32 layer_ID;
  gint32 mat_layer_ID; // for the border
  gint32 group_ID;
  gint   position_n;
} InputNode;

typedef struct
{
  GSList   *input_filenames; // list of image file names & position in the grid
  gint     n_cols; // number of images wide to make the grid
  gint     n_rows; // number of images tall to make the grid
  gint     col_width; // width of one column
  gint     row_height;// height of one row
  gint     output_width; // output image widht
  gint     output_height;// outout image height
  gint     gutter_x; // horizonal gutter between images
  gint     gutter_y; // vertical gutter between images
  gint     border_width;
  gint     seed;
  gboolean  random_seed;
  gint32    image_ID;
  GSList   *input_nodes; // list of InputNode layer ids & position in the grid
  gint      max_input_x; // input image max width
  gint      max_input_y; // input image max height
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

gboolean cleanupInputNode(InputNode *node);

#endif /* __MAIN_H__ */
