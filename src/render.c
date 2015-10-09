/* 
 */

#include "config.h"

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"

#include "plugin-intl.h"


/*  Public functions  */

void
render (gint32              image_ID,
	GimpDrawable       *drawable,
	PlugInVals         *vals,
	PlugInImageVals    *image_vals,
	PlugInDrawableVals *drawable_vals)
{
  g_message (_("This plug-in is just a dummy. "
               "It has now finished doing nothing."));
}
