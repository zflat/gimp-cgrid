/* 
 */

#include "config.h"
#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "ui.h"
#include "render.h"
#include "maker.h"

#include "plugin-intl.h"


/*  Constants  */

#define PROCEDURE_NAME   "gimp_collection_grid_maker"

#define DATA_KEY_VALS    "plug_in_template"
#define DATA_KEY_UI_VALS "plug_in_template_ui"

#define PARASITE_KEY     "plug-in-template-options"


/*  Local function prototypes  */

static void   query (void);
static void   run   (const gchar      *name,
                     gint              nparams,
                     const GimpParam  *param,
                     gint             *nreturn_vals,
                     GimpParam       **return_vals);


/*  Local variables  */

const PlugInVals default_vals =
  {
    NULL,
    1, // n_cols
    0, // n_rows
    0, // col_width
    0, // row_height
    1, // output image width
    1, // output image height
    20, // margin_x
    20, // margin_y
    0,  // border_width
    0, 
    FALSE,
    0, // image ID
    NULL, // list 
    0, // max x 
    0  // max y
  };

const PlugInUIVals default_ui_vals =
  {
    TRUE
  };

static PlugInVals         vals;
static PlugInImageVals    image_vals;
static PlugInDrawableVals drawable_vals;
static PlugInUIVals       ui_vals;


GimpPlugInInfo PLUG_IN_INFO =
  {
    NULL,  /* init_proc  */
    NULL,  /* quit_proc  */
    query, /* query_proc */
    run,   /* run_proc   */
  };

MAIN ()

static void query (void) {
  gchar *help_path;
  gchar *help_uri;

  static GimpParamDef args[] =
    {
      { GIMP_PDB_INT32,    "run_mode",   "Interactive, non-interactive"    },
    };

  gimp_plugin_domain_register (GETTEXT_PACKAGE, LOCALEDIR);

  help_path = g_build_filename (DATADIR, "help", NULL);
  help_uri = g_filename_to_uri (help_path, NULL, NULL);
  g_free (help_path);

  gimp_plugin_help_register ("http://developer.gimp.org/plug-in-template/help",
                             help_uri);

  g_free (help_uri);

  gimp_install_procedure (PROCEDURE_NAME,
                          PLUG_IN_DESCRIPTION,
                          "William Wedler",
                          "William Wedler",
                          "wwedler.com",
                          "2015",
                          N_("Collection Grid Maker..."),
                          "",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  gimp_plugin_menu_register (PROCEDURE_NAME, "<Image>/File/Open");
}

static void
run (const gchar      *name,
     gint              n_params,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals) {
  static GimpParam   values[1];
  GimpDrawable      *drawable;
  gint32             image_ID;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  /*  Initialize i18n support  */
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
  textdomain (GETTEXT_PACKAGE);

  run_mode = param[0].data.d_int32;

  /*  Initialize with default values  */
  vals          = default_vals;
  ui_vals       = default_ui_vals;

  if (strcmp (name, PROCEDURE_NAME) == 0) {
    switch (run_mode)	{
    case GIMP_RUN_NONINTERACTIVE:
      break;

    case GIMP_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      gimp_get_data (DATA_KEY_VALS,    &vals);
      gimp_get_data (DATA_KEY_UI_VALS, &ui_vals);

      if ( !dialog(&vals, &ui_vals) ) {
	      status = GIMP_PDB_CANCEL;
        break;
	    } 

      if( !build_image_grid(&vals)  ) {
        break;
      }

      break;
    case GIMP_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      gimp_get_data (DATA_KEY_VALS, &vals);

      if (vals.random_seed)
        vals.seed = g_random_int ();
      break;

    default:
      break;
    }
  } else {
    status = GIMP_PDB_CALLING_ERROR;
  }

  if (status == GIMP_PDB_SUCCESS) {
    render (NULL, NULL, &vals, NULL, NULL);

    if (run_mode != GIMP_RUN_NONINTERACTIVE)
      gimp_displays_flush ();

    if (run_mode == GIMP_RUN_INTERACTIVE) {
      gimp_set_data (DATA_KEY_VALS,    &vals,    sizeof (vals));
      gimp_set_data (DATA_KEY_UI_VALS, &ui_vals, sizeof (ui_vals));
    }
    if(NULL != drawable) {
      gimp_drawable_detach (drawable);
    }

    // cleanup vals
    g_slist_free_full(vals.input_nodes, cleanupInputNode);
    g_slist_free(vals.input_filenames);

  }
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

gboolean cleanupInputNode(InputNode *node) {
  if(node) {
    free(node);
  }
}
