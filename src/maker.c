/* 
 */

#include "config.h"

#include <gtk/gtk.h>
#include <libgimp/gimp.h>

#include "main.h"
#include "maker.h"
#include "cgrid_utils.h"

#include "plugin-intl.h"


/*  Public functions  */

gboolean   build_image_grid (PlugInVals *vals) {
  gint32 new_image_display;

  g_printf("Selected %d images.\n", g_slist_length(vals->input_filenames));
  g_printf("first filename: %s\n", vals->input_filenames->data);
  g_printf("gutter x: %d\n", vals->margin_x);
  g_printf("gutter y: %d\n", vals->margin_y);

  vals->image_ID = gimp_image_new(MAX((gint)vals->margin_x, 1), MAX((gint)vals->margin_y, 1), GIMP_RGB);

  if(vals->image_ID <= 0) {
    return FALSE;
  }

  g_slist_foreach(vals->input_filenames, load_input_layer_action, vals);
  compute_image_size(vals);

  // resize the image to computed size
  gimp_image_resize(vals->image_ID, vals->output_width, vals->output_height, 0, 0);

  // insert each layer into the image
  g_slist_foreach(vals->input_nodes, place_layer_action, vals);
    
  // display the image
  new_image_display = gimp_display_new(vals->image_ID);
}

gboolean compute_location(gint position_num, PlugInVals *vals, gint *x_location, gint *y_location) {
  gint row_num = FLOOR_POS(position_num / vals->n_cols);
  gint col_num = (position_num) % vals->n_cols;

  g_printf("position: %d, row=%d col=%d \n",position_num, row_num, col_num);
  
  *x_location = vals->margin_x + col_num * vals->col_width;
  *y_location = vals->margin_y + row_num * vals->row_height;
}

gboolean place_layer_action(InputNode *node,  PlugInVals *vals) {
  gimp_image_insert_layer(vals->image_ID, node->layer_ID, 0, -1);

  // compute the x,y position
  gint x_location;
  gint y_location;
  compute_location(node->position_n, vals, &x_location, &y_location);

  g_print("placing image %d at (%d, %d)\n", node->position_n, x_location, y_location);
  
  // place the layer in its position
  gimp_layer_set_offsets(node->layer_ID, x_location, y_location);
}

gboolean compute_image_size(PlugInVals *vals) {
  gint n_images;

  gint total_margin_x;
  gint total_margin_y;
  
  n_images = g_slist_length(vals->input_filenames);
  vals->n_rows = compute_n_rows(n_images, vals->n_cols);
  g_printf("n_rows=%d\n", vals->n_rows);
  
  // compute width
  // width of one column
  vals->col_width = vals->margin_x + vals->max_input_x;
  vals->output_width = vals->margin_x + (vals->n_cols * vals->col_width);

  // compute height
  // height of one row
  vals->row_height = vals->margin_y + vals->max_input_y;  
  vals->output_height = vals->margin_y + (vals->n_rows * vals->row_height);

  return TRUE;  
}


gboolean load_input_layer_action(gchar *input_filename,  PlugInVals *vals) {
  gint32 layer_ID;
  gint im_width;
  gint im_height;

  layer_ID = gimp_file_load_layer(GIMP_RUN_NONINTERACTIVE, vals->image_ID, input_filename);
  g_printf("new layer id %d loaded into image id %d\n", layer_ID, vals->image_ID); 
 
  im_width = gimp_drawable_width(layer_ID);
  im_height = gimp_drawable_height(layer_ID);

  vals->max_input_x = MAX(im_width, vals->max_input_x);
  vals->max_input_y = MAX(im_height, vals->max_input_y);

  // append to layers list
  InputNode *node = calloc(1, sizeof(InputNode));
  node->layer_ID = layer_ID;
  node->position_n = g_slist_length(vals->input_nodes);
  vals->input_nodes = g_slist_append(vals->input_nodes, node);
  g_print("%d number of nodes\n", g_slist_length(vals->input_nodes));

  return TRUE;
}
