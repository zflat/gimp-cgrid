/* 
 */

#ifndef __MAKER_H__
#define __MAKER_H__


/*  Public functions  */

gboolean   build_image_grid (PlugInVals *vals);

/**
 * Callback used to iterate over the file names
 * Acts to load the file as a layer and to compute
 * the max widht/height of the input images
 * 
 * The max width/height and layer IDs are stored
 * in the *vals struct.
 */
gboolean load_input_layer_action(gchar *input_filename, PlugInVals *vals);

/**
 * Callback used to place each layer onto the image
 * at its specified location.
 */
gboolean place_layer_action(InputNode *node,  PlugInVals *vals);

/**
 * imge size base on 
 * max width
 * max height
 * number of rows
 * number of columns
 * margins x and y
 */
gboolean compute_image_size(PlugInVals *vals);

/**
 * Compute the x,y location for the given position number
 * and the number of rows and cols and width & height of 
 * the image
 */
gboolean compute_location(gint position_num, PlugInVals *vals, gint im_width, gint im_height, gint *x_location, gint *y_location);

#endif /* __MAKER_H__ */
