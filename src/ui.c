/* 
 */

#include "config.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "ui.h"

#include "plugin-intl.h"


/*  Constants  */

#define SCALE_WIDTH        180
#define SPIN_BUTTON_WIDTH   75
#define RANDOM_SEED_WIDTH  100


/*  Local function prototypes  */

static gboolean   dialog_image_constraint_func (gint32    image_id,
                                                gpointer  data);


/*  Local variables  */

static PlugInUIVals *ui_state = NULL;


/*  Public functions  */

gboolean
dialog (
	PlugInVals         *vals,
	PlugInUIVals       *ui_vals)
{
  GtkWidget *dlg;
  GtkWidget *main_vbox;
  GtkWidget *frame;
  GtkWidget *coordinates;
  GtkWidget *combo;
  gboolean   run = FALSE;
  GimpUnit   unit;

  ui_state = ui_vals;

  gimp_ui_init (PLUGIN_NAME, TRUE);

  dlg = gimp_dialog_new (_("GIMP Plug-In Template"), PLUGIN_NAME,
                         NULL, 0,
			 gimp_standard_help_func, "plug-in-template",

			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 GTK_STOCK_OK,     GTK_RESPONSE_OK,

			 NULL);

  main_vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dlg)->vbox), main_vbox);

  /*  gimp_scale_entry_new() examples  */

  frame = gimp_frame_new (_("ScaleEntry Examples"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  Show the main containers  */

  gtk_widget_show (main_vbox);
  gtk_widget_show (dlg);

  run = (gimp_dialog_run (GIMP_DIALOG (dlg)) == GTK_RESPONSE_OK);

  if (run) {
      /*  Save ui values  */
      ui_state->chain_active =
        gimp_chain_button_get_active (GIMP_COORDINATES_CHAINBUTTON (coordinates));
  }

  gtk_widget_destroy (dlg);

  return run;
}


/*  Private functions  */

static gboolean
dialog_image_constraint_func (gint32    image_id,
                              gpointer  data)
{
  return (gimp_image_base_type (image_id) == GIMP_RGB);
}
