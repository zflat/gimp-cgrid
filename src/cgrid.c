/*
   gimptool-2.0 --build cgrid.c

   See https://gimpbook.com/scripting/
   https://developer.gimp.org/plug-ins.html
*/

#include <string.h>

#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>


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





#define MAIN_WINDOW_W 660
#define MAIN_WINDOW_H 470

#define PROGRESSBAR_W MAIN_WINDOW_W - 30
#define PROGRESSBAR_H 20

/* Option panel (and children) dimensions */
#define OPTION_PANEL_W MAIN_WINDOW_W - 30
#define OPTION_PANEL_H MAIN_WINDOW_H - PROGRESSBAR_H - 80

#define INPUT_PANEL_W 350
#define INPUT_PANEL_H OPTION_PANEL_H - 30

#define USEROPTIONS_PANEL_W OPTION_PANEL_W - INPUT_PANEL_W - 10
#define USEROPTIONS_PANEL_H OPTION_PANEL_H - 10

#define USEROPTIONS_CHOOSER_W USEROPTIONS_PANEL_W - 40
#define USEROPTIONS_CHOOSER_H 40

#define FILE_PREVIEW_W 150
#define FILE_PREVIEW_H 130

#define FILE_LIST_PANEL_W INPUT_PANEL_W
#define FILE_LIST_PANEL_H 200

#define FILE_LIST_BUTTONS_PANEL_W FILE_LIST_PANEL_W
#define FILE_LIST_BUTTONS_PANEL_H 30

#define FILE_LIST_BUTTON_W FILE_LIST_BUTTONS_PANEL_W / 2
#define FILE_LIST_BUTTON_H FILE_LIST_BUTTONS_PANEL_H

#define PREVIEW_WINDOW_W 600
#define PREVIEW_WINDOW_H 320

#define PREVIEW_IMG_W (PREVIEW_WINDOW_W / 2) - 30
#define PREVIEW_IMG_H 220


char* str_replace(char*, char*, char*);
char* comp_get_filename(char*);
char* comp_get_filefolder(char*);
gboolean str_contains_cins(char*, char*);
gboolean file_has_extension(char*, char*);
GimpParamDef pdb_proc_get_param_info(gchar*, gint);
char* get_user_dir(void);
/* char* get_cgrid_localedir(void); */
int glib_strcmpi(gconstpointer, gconstpointer);
gchar** get_path_folders (char*);
char* get_datetime(void);
time_t get_modification_time(char*);
int set_modification_time(char*, time_t);

#if defined _WIN32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_STR "\\"
#else
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_STR "/"
#endif

#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))
#define FLOOR_POS(X) ((int)(X))

gint compute_n_rows(gint total_elems, gint n_cols);


GtkWidget* cgrid_window_main;


gboolean   dialog (
		   PlugInVals         *vals,
		   PlugInUIVals       *ui_vals);


void   render (gint32              image_ID,
	       GimpDrawable       *drawable,
	       PlugInVals         *vals,
	       PlugInImageVals    *image_vals,
	       PlugInDrawableVals *drawable_vals)
{
  g_message ("All images loaded. Finished creating grid.");
}

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



#define PROCEDURE_NAME   "gimp_collection_grid_maker"

#define DATA_KEY_VALS    "plug_in_template"
#define DATA_KEY_UI_VALS "plug_in_template_ui"

#define PARASITE_KEY     "plug-in-template-options"



static void   query (void);
static void   run   (const gchar      *name,
		     gint              nparams,
		     const GimpParam  *param,
		     gint             *nreturn_vals,
		     GimpParam       **return_vals);


const PlugInVals default_vals =
  {
    NULL, // input file names
    1, // n_cols
    0, // n_rows
    0, // col_width
    0, // row_height
    1, // output image width
    1, // output image height
    20, // margin_x
    20, // margin_y
    0,  // border_width
    0,  // seed
    FALSE, // random seed
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

  gimp_install_procedure (PROCEDURE_NAME,
			  PLUG_IN_DESCRIPTION,
			  "William Wedler",
			  "William Wedler",
			  "wwedler.com",
			  "2015",
			  "Collection Grid Maker...",
			  "",
			  GIMP_PLUGIN,
			  G_N_ELEMENTS (args), 0,
			  args, NULL);

  gimp_plugin_menu_register (PROCEDURE_NAME, "<Image>/File/Create");
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

  run_mode = param[0].data.d_int32;

  /*  Initialize with default values  */
  vals          = default_vals;
  ui_vals       = default_ui_vals;

  if (strcmp (name, PROCEDURE_NAME) == 0) {
    switch (run_mode)	{
    case GIMP_RUN_NONINTERACTIVE:
      break;

    case GIMP_RUN_INTERACTIVE:
      g_print("GTK version %d.%d.%d\n",
	      GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

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

#include <sys/stat.h>
#include <time.h>
#include <utime.h>

/* replace all the occurrences of 'rep' into 'orig' with text 'with' */
char* str_replace(char *orig, char *rep, char *with) {
  char *result;
  char *ins;
  char *tmp;
  int len_rep;
  int len_with;
  int len_front;
  int count;

  if (!orig) {
    return NULL;
  }
  if (!rep || !(len_rep = strlen(rep))) {
    return NULL;
  }
  if (!(ins = strstr(orig, rep))) {
    return NULL;
  }

  if (!with) {
    with = "";
  }

  len_with = strlen(with);

  for (count = 0; tmp = strstr(ins, rep); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result) {
    return NULL;
  }

  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep;
  }
  strcpy(tmp, orig);
  return result;
}

/* gets the filename from the given path
 * (compatible with unix and win) */
char* comp_get_filename(char* path) {
  char *pfile;

  pfile = path + strlen(path);
  for (; pfile > path; pfile--)
    {
      if ((*pfile == FILE_SEPARATOR)) //'\\') || (*pfile == '/'))
	{
	  pfile++;
	  break;
	}
    }

  return pfile;
}

/* gets only the file folder from the given path
 * (compatible with unix and win) */
char* comp_get_filefolder(char* path) {
  int i;
  char *folder = strdup(path);

  for (i = strlen(folder); i > 0 ; i--)
    {
      if ((folder[i-1] == FILE_SEPARATOR))
	{
	  folder[i] = '\0';
	  break;
	}
    }
  return folder;
}

/* return TRUE if the first string 'fullstr' contains one or more occurences of substring 'search'.
 * (case-insensitive version) */
gboolean str_contains_cins(char* fullstr, char* search) {
  return (
	  strstr(
		 g_ascii_strdown(fullstr, strlen(fullstr)),
		 g_ascii_strdown(search, strlen(search))
		 )!= NULL
	  );
}

gboolean file_has_extension(char* file, char* ext) {
  return g_str_has_suffix(
			  g_ascii_strdown(file, strlen(file)),
			  g_ascii_strdown(ext, strlen(ext))
			  );
}

GimpParamDef pdb_proc_get_param_info(gchar* proc_name, gint arg_num) {
  GimpParamDef param_info;
  GimpPDBArgType type;
  gchar *name;
  gchar *desc;

  gimp_procedural_db_proc_arg (
			       proc_name,
			       arg_num,
			       &type,
			       &name,
			       &desc
			       );

  param_info.type = type;
  param_info.name = g_strdup(name);
  param_info.description = g_strdup(desc);

  return param_info;
}

char* get_user_dir() {
  char* path = NULL;

#ifdef _WIN32
  path = g_strconcat(getenv("HOMEDRIVE"), getenv("HOMEPATH"), NULL);
  if (strlen(path) == 0) path = "C:\\";
#else
  path = getenv("HOME");
  if (strlen(path) == 0) path = "/";
#endif

  return path;
}

/* C-string case-insensitive comparison function (with gconstpointer args) */
int glib_strcmpi(gconstpointer str1, gconstpointer str2) {
  return strcasecmp(str1, str2);
}

gchar** get_path_folders (char *path) {
  char * normalized_path = (char*)g_malloc(sizeof(path));

  normalized_path = g_strdup(path);
  return g_strsplit(normalized_path, FILE_SEPARATOR_STR, 0);
}

/* gets the current date and time in "%Y-%m-%d_%H-%M" format */
char* get_datetime() {
  time_t rawtime;
  struct tm * timeinfo;
  char* format;

  format = (char*)malloc(sizeof(char)*18);
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime (format, 18, "%Y-%m-%d_%H-%M", timeinfo);

  return format;
}

time_t get_modification_time(char* filename) {
  struct stat filestats;
  if (stat(filename, &filestats) < 0) {
    return -1;
  }

  return filestats.st_mtime;
}

int set_modification_time(char* filename, time_t mtime) {
  struct stat filestats;

  if (stat(filename, &filestats) < 0) {
    return -1;
  }

  struct utimbuf new_time;
  new_time.actime = filestats.st_atime;
  new_time.modtime = mtime;
  if (utime(filename, &new_time) < 0) {
    return -1;
  }
  else return 0;
}

gint compute_n_rows(gint total_elems, gint n_cols) {
  return CEILING_POS((gfloat)total_elems/n_cols);
}


gboolean   build_image_grid (PlugInVals *vals) {
  gint32 new_image_display;

  g_printf("Selected %d images.\n", g_slist_length(vals->input_filenames));
  g_printf("first filename: %s\n", vals->input_filenames->data);
  g_printf("gutter x: %d\n", vals->gutter_x);
  g_printf("gutter y: %d\n", vals->gutter_y);

  vals->image_ID = gimp_image_new(MAX((gint)vals->gutter_x, 1), MAX((gint)vals->gutter_y, 1), GIMP_RGB);

  if(vals->image_ID <= 0) {
    return FALSE;
  }

  g_slist_foreach(vals->input_filenames, load_input_layer_action, vals);
  compute_image_size(vals);

  // resize the image to computed size
  gimp_image_resize(vals->image_ID, vals->output_width, vals->output_height, 0, 0);

  // insert each layer into the image
  GSList * starting_node = g_slist_last(vals->input_nodes);
  GSList * rev_list = g_slist_reverse(vals->input_nodes);
  vals->input_nodes = starting_node;
  g_slist_foreach(vals->input_nodes,
		  place_layer_action, vals);

  // display the image
  new_image_display = gimp_display_new(vals->image_ID);
}

gboolean compute_location(gint position_num, PlugInVals *vals, gint im_width, gint im_height, gint *x_location, gint *y_location) {
  gint row_num = FLOOR_POS(position_num / vals->n_cols);
  gint col_num = (position_num) % vals->n_cols;

  gint diff_x = vals->max_input_x - im_width;
  gint diff_y = vals->max_input_y - im_height;

  g_printf("position: %d, row=%d col=%d \n",position_num, row_num, col_num);

  *x_location = vals->gutter_x + col_num * vals->col_width + diff_x/2;
  *y_location = vals->gutter_y + row_num * vals->row_height + diff_y/2;
}

gboolean place_layer_action(InputNode *node,  PlugInVals *vals) {
  gint im_width;
  gint im_height;
  gint x_location;
  gint y_location;

  im_width = gimp_drawable_width(node->layer_ID);
  im_height = gimp_drawable_height(node->layer_ID);

  gimp_image_insert_layer(vals->image_ID, node->layer_ID, 0, -1);

  // compute the x,y position
  compute_location(node->position_n, vals, im_width, im_height, &x_location, &y_location);

  g_print("placing image %d at (%d, %d)\n", node->position_n, x_location, y_location);

  // place the layer in its position
  gimp_layer_set_offsets(node->layer_ID, x_location, y_location);

  // add alpha to the layer, implicit change to RGBA
  gimp_layer_add_alpha(node->layer_ID);
  // resize the layer
  // gimp_layer_resize_to_image_size(node->layer_ID);
}

gboolean compute_image_size(PlugInVals *vals) {
  gint n_images;

  gint total_gutter_x;
  gint total_gutter_y;

  n_images = g_slist_length(vals->input_filenames);
  vals->n_rows = compute_n_rows(n_images, vals->n_cols);
  g_printf("n_rows=%d\n", vals->n_rows);

  // compute width
  // width of one column
  vals->col_width = vals->gutter_x + vals->max_input_x;
  vals->output_width = vals->gutter_x + (vals->n_cols * vals->col_width);

  // compute height
  // height of one row
  vals->row_height = vals->gutter_y + vals->max_input_y;
  vals->output_height = vals->gutter_y + (vals->n_rows * vals->row_height);

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


#define SCALE_WIDTH        180
#define SPIN_BUTTON_WIDTH   75
#define RANDOM_SEED_WIDTH  100

static gboolean   dialog_image_constraint_func (gint32    image_id,
						gpointer  data);

static GtkWidget* option_panel_new();
static void add_input_file(char* filename);
static void add_input_folder_r(char* folder, gboolean with_subdirs);
static void add_input_folder(char* folder, gpointer with_subdirs);
static GSList* get_treeview_selection();
static void remove_input_file(GtkWidget *widget, gpointer data);
static void remove_all_input_files(GtkWidget *widget, gpointer data);
static void select_filename (GtkTreeView *tree_view, gpointer data);
static void update_selection (gchar* filename);
static void open_file_chooser(GtkWidget *widget, gpointer data);
static void open_folder_chooser(GtkWidget *widget, gpointer data);
static void open_addfiles_popupmenu(GtkWidget*, gpointer);
static void open_removefiles_popupmenu(GtkWidget *widget, gpointer data);
static void popmenus_init(void);

static void init_fileview();
static void add_to_fileview(char *str);
static void refresh_fileview();

static void cgrid_set_busy(gboolean busy);

void cgrid_show_error_dialog(char* message, GtkWidget* parent);

/*  Local variables  */

static PlugInUIVals *ui_state = NULL;


GtkWidget *panel_options;
GtkWidget *popmenu_add, *popmenu_edit, *popmenu_addfiles, *popmenu_removefiles;
GtkWidget *treeview_files;
GtkWidget *gutters;
GtkWidget *lbl_files_info;
char *n_files_text = NULL;
GtkWidget* adj_n_cols;
GtkWidget *spin_button_resolution;

//GtkWidget* progressbar_visible;
enum /* TreeView stuff... */
  {
    LIST_ITEM = 0,
    N_COLUMNS
  };

//const gchar* progressbar_data;


static void window_close_callback(GtkWidget* widget, gpointer data) {
  gtk_dialog_response(widget, GTK_RESPONSE_CLOSE);
}


/*  Public functions  */

gboolean dialog (PlugInVals *vals, PlugInUIVals *ui_vals) {
  gboolean return_val = FALSE;
  GtkWidget* vbox_main;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  cgrid_window_main =
    gimp_dialog_new(
		    PLUG_IN_FULLNAME, // title
		    PLUG_IN_BINARY,   //role
		    NULL, // paren
		    0, //flags
		    NULL, // help func
		    NULL, // held_id
		    //GTK_STOCK_ABOUT, GTK_RESPONSE_HELP,
		    GTK_STOCK_OK,     GTK_RESPONSE_OK,
		    GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
		    NULL);

  gimp_window_set_transient (GTK_WINDOW(cgrid_window_main));
  gtk_widget_set_size_request (cgrid_window_main, MAIN_WINDOW_W, MAIN_WINDOW_H);
  gtk_window_set_resizable (GTK_WINDOW(cgrid_window_main), FALSE);
  gtk_window_set_position(GTK_WINDOW(cgrid_window_main), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(cgrid_window_main), 5);

  // Forces the visualization of label AND images on buttons (especially for Windows)
  GtkSettings *default_settings = gtk_settings_get_default();
  g_object_set(default_settings, "gtk-button-images", TRUE, NULL);

  vbox_main = gtk_vbox_new(FALSE, 10);
  panel_options = option_panel_new(vals);
  popmenus_init();

  gtk_box_pack_start(GTK_BOX(vbox_main), panel_options, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(cgrid_window_main)->vbox), vbox_main);


  g_signal_connect (cgrid_window_main,
		    "close",
		    G_CALLBACK (window_close_callback),
		    NULL);

  gtk_widget_show_all(cgrid_window_main);


  cgrid_set_busy(FALSE);

  gboolean retval=TRUE;
  while(TRUE) {
    gint run = gimp_dialog_run (GIMP_DIALOG(cgrid_window_main));
    if (run == GTK_RESPONSE_OK) {
      if (g_slist_length(cgrid_input_filenames) == 0) {
	cgrid_show_error_dialog("The file list is empty!", cgrid_window_main);
      }
      else {
	/* Set options */
	vals->input_filenames = cgrid_input_filenames;
	vals->n_cols = gtk_adjustment_get_value(adj_n_cols);
	gdouble resolution = gtk_spin_button_get_value(spin_button_resolution);
	vals->gutter_x = gimp_units_to_pixels(gimp_size_entry_get_value(gutters, 0), // value
					      gimp_size_entry_get_unit(gutters), //unit
					      resolution
					      );
	vals->gutter_y = gimp_units_to_pixels(gimp_size_entry_get_value(gutters, 1), // value
					      gimp_size_entry_get_unit(gutters), //unit
					      resolution
					      );

	vals->input_nodes = NULL;
	vals->max_input_x = 0;
	vals->max_input_y = 0;
	g_printf("processing files...\n\a");
	retval = TRUE;
	break;
      }
    }
    else if (run == GTK_RESPONSE_HELP) {
      /* open_about();*/
    }
    else if (run == GTK_RESPONSE_CLOSE) {
      retval = FALSE;
      break;
    }
  }
  gtk_widget_destroy (cgrid_window_main);
  return retval;
}

/*  Private functions  */
static gboolean dialog_image_constraint_func (gint32 image_id, gpointer  data) {
  return (gimp_image_base_type (image_id) == GIMP_RGB);
}


static void toggle_resolution_sensitive(GtkWidget* widget, gpointer data) {
  GimpUnit current_unit = gimp_size_entry_get_unit(widget);
  gtk_widget_set_sensitive(data, (current_unit != GIMP_UNIT_PIXEL));
}

static void changed_resolution_callback(GtkWidget* widget, gpointer data) {
  gdouble resolution = gtk_spin_button_get_value(widget);
  gimp_size_entry_set_resolution(data, // GimpSizeEntry
				 0, // field
				 resolution,
				 FALSE
				 );
  gimp_size_entry_set_resolution(data, // GimpSizeEntry
				 1, // field
				 resolution,
				 FALSE
				 );
}

/* builds and returns the panel with file list and options */
static GtkWidget* option_panel_new(PlugInVals *vals) {

  GtkWidget *panel;
  GtkWidget *hbox_buttons;
  GtkWidget *vbox_input;
  GtkWidget *scroll_input;
  GtkWidget *button_add, *button_remove;
  GtkWidget *lbl_n_cols;

  GtkWidget *hbox_resolution;
  GtkWidget *lbl_resolution;
  GtkWidget *lbl_resolution_desc;

  GtkWidget *table_optns;

  panel = gtk_frame_new("Input files and options");
  gtk_widget_set_size_request (panel, OPTION_PANEL_W, OPTION_PANEL_H);

  /* Sub-panel for input file listing and buttons */
  vbox_input = gtk_vbox_new(FALSE, 1);
  gtk_widget_set_size_request(vbox_input, INPUT_PANEL_W, INPUT_PANEL_H);

  /* Sub-sub-panel for input file listing */
  scroll_input = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_input), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scroll_input, FILE_LIST_PANEL_W, FILE_LIST_PANEL_H);

  treeview_files = gtk_tree_view_new();
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview_files), FALSE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files)), GTK_SELECTION_MULTIPLE);

  /* Sub-panel for input file buttons */
  hbox_buttons = gtk_hbox_new(FALSE, 1);
  gtk_widget_set_size_request(hbox_buttons, FILE_LIST_BUTTONS_PANEL_W, FILE_LIST_BUTTONS_PANEL_H);
  lbl_files_info = gtk_label_new("Total Files: 0");
  gtk_widget_set_size_request(lbl_files_info, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
  button_add = gtk_button_new_with_label("Add images");
  gtk_widget_set_size_request(button_add, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
  button_remove = gtk_button_new_with_label("Remove images");
  gtk_widget_set_size_request(button_remove, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);

  gutters = gimp_coordinates_new(GIMP_UNIT_PIXEL,
				 "%s",
				 TRUE,
				 FALSE,
				 SPIN_BUTTON_WIDTH,
				 GIMP_SIZE_ENTRY_UPDATE_SIZE,
				 TRUE,
				 FALSE,
				 "Horizontal spacing",
				 0,
				 300,
				 0,
				 2<<16,
				 0,
				 2<<16,
				 "Vertical spacing",
				 0,
				 300,
				 0,
				 2<<16,
				 0,
				 2<<16
				 );


  table_optns = gtk_table_new(4,//rows
			      3, //cols
			      FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table_optns), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table_optns), 2);

  adj_n_cols = gimp_scale_entry_new (GTK_TABLE (table_optns),
				     0, // column
				     2, // row
				     NULL, // text
				     SCALE_WIDTH, SPIN_BUTTON_WIDTH,
				     vals->n_cols, 1, 1, 1, 10, 0,
				     TRUE, 0, 0,
				     "Cols...tip", NULL);
  lbl_n_cols = gtk_label_new("Max number of columns:");
  gtk_misc_set_alignment(lbl_n_cols, 1, 0.3);
  gtk_table_attach_defaults(table_optns,
			    lbl_n_cols,
			    0, // left
			    1, // right
			    2, // top
			    3);// bottom
  g_signal_connect (adj_n_cols, "value_changed",
		    G_CALLBACK (gimp_int_adjustment_update),
		    &vals->n_cols);

  hbox_resolution = gtk_hbox_new(FALSE, 1);
  //gtk_widget_set_size_request(hbox_resolution, FILE_LIST_BUTTONS_PANEL_W, FILE_LIST_BUTTONS_PANEL_H);
  lbl_resolution = gtk_label_new("Resolution: ");
  lbl_resolution_desc = gtk_label_new("DPI");
  spin_button_resolution = gtk_spin_button_new_with_range(1, 2<<16, 50);
  gtk_spin_button_set_value(spin_button_resolution,
			    300);
  gtk_widget_set_sensitive(spin_button_resolution, FALSE);
  g_signal_connect(gutters,
		   "unit_changed",
		   G_CALLBACK(toggle_resolution_sensitive),
		   spin_button_resolution);
  g_signal_connect(spin_button_resolution,
		   "value_changed",
		   G_CALLBACK(changed_resolution_callback),
		   gutters);


  /* All together */
  gtk_box_pack_start(GTK_BOX(hbox_buttons), lbl_files_info, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_buttons), button_add, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_buttons), button_remove, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(scroll_input), treeview_files);

  gtk_table_attach_defaults(table_optns,
			    gutters,
			    1, // left
			    2, // right
			    0, // top
			    1);// bottom

  gtk_box_pack_start(GTK_BOX(hbox_resolution), lbl_resolution, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_resolution), spin_button_resolution, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_resolution), lbl_resolution_desc, FALSE, FALSE, 0);
  gtk_table_attach_defaults(table_optns,
			    hbox_resolution,
			    1, // left
			    2, // right
			    1, // top
			    2);// bottom

  gtk_box_pack_start(GTK_BOX(vbox_input), scroll_input, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_input), hbox_buttons, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_input), table_optns, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(panel), vbox_input);

  g_signal_connect(G_OBJECT(button_add), "clicked", G_CALLBACK(open_addfiles_popupmenu), NULL);
  g_signal_connect(G_OBJECT(button_remove), "clicked", G_CALLBACK(open_removefiles_popupmenu), NULL);
  g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files))), "changed", G_CALLBACK(select_filename), NULL);

  init_fileview();
  refresh_fileview();

  return panel;
}




/* following: functions that modify the file list widget (addfile/addfolder/remove/removeall)  */

static void add_input_file(char* filename) {
  if (g_slist_find_custom(cgrid_input_filenames, filename, (GCompareFunc)strcmp) == NULL) {
    cgrid_input_filenames = g_slist_append(cgrid_input_filenames, filename);
    refresh_fileview();
  }
}

/* Recursive function to add all files from the hierarchy if desired */
static void add_input_folder_r(char* folder, gboolean with_subdirs) {
  GDir *dp;
  const gchar* entry;
  dp = g_dir_open (folder, 0, NULL);

  if (dp != NULL) {
    while ((entry = g_dir_read_name (dp))) {

      char* filename = g_strconcat(folder, FILE_SEPARATOR_STR, entry, NULL);
      char* file_extension = g_strdup(strrchr(filename, '.'));
      GError *error;
      GFileInfo *file_info = g_file_query_info (g_file_new_for_path(filename), "standard::*", 0, NULL, &error);

      /* Folder processing */
      if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY){
	if (g_strcmp0(entry, ".") == 0 || g_strcmp0(entry, "..") == 0)
	  continue;
	if (with_subdirs)
	  add_input_folder_r(filename, with_subdirs);
	continue;
      }

      if ((
	   g_ascii_strcasecmp(file_extension, ".bmp") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".jpeg") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".jpg") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".jpe") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".gif") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".png") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".tif") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".tiff") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".svg") == 0 ||
	   g_ascii_strcasecmp(file_extension, ".xcf") == 0) &&
	  g_slist_find_custom(cgrid_input_filenames, filename, (GCompareFunc)strcmp) == NULL)
	{
	  cgrid_input_filenames = g_slist_append(cgrid_input_filenames, filename);
	}
    }
    g_dir_close (dp);
  }
  else {
    cgrid_show_error_dialog(g_strdup_printf("Couldn't read into \"%s\" directory.", folder), cgrid_window_main);
  }
}


static void add_input_folder(char* folder, gpointer with_subdirs) {
  add_input_folder_r(folder, (gboolean)GPOINTER_TO_INT(with_subdirs));
  refresh_fileview();
}


/* returns the list of currently selected filenames (NULL of none) */
static GSList* get_treeview_selection() {
  GtkTreeModel *model;
  GList *selected_rows = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files)), &model);
  GList *i = NULL;
  GSList *out = NULL;

  if (selected_rows != NULL) {
    for (i = selected_rows; i != NULL; i = g_list_next(i) ) {
      GtkTreeIter iter;
      char* selected_i;
      if (gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)i->data) == TRUE) {
	gtk_tree_model_get(model, &iter, LIST_ITEM, &selected_i, -1);
	out = g_slist_append(out, selected_i);
      }
    }

    g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (selected_rows);
  }

  return out;
}


static void remove_input_file(GtkWidget *widget, gpointer data) {
  GSList *selection = get_treeview_selection();
  GSList *i;

  if (selection != NULL) {
    for (i = selection; i != NULL; i = g_slist_next(i) ) {
      cgrid_input_filenames = g_slist_delete_link(cgrid_input_filenames, g_slist_find_custom(cgrid_input_filenames, (char*)(i->data), (GCompareFunc)strcmp));
    }

    refresh_fileview();
    update_selection(NULL); /* clear the preview widget */
  }
}

static void remove_all_input_files(GtkWidget *widget, gpointer data) {
  g_slist_free(cgrid_input_filenames);
  cgrid_input_filenames = NULL;
  refresh_fileview();
  update_selection(NULL);
}



static void init_fileview() {
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkListStore *store;

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(
						    "List Items",
						    renderer,
						    "text",
						    LIST_ITEM,
						    NULL
						    );
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_files), column);

  store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_files), GTK_TREE_MODEL(store));
  g_object_unref(store);
}

static void add_to_fileview(char *str) {
  GtkListStore *store;
  GtkTreeIter iter;

  store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_files)));
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

/* update the visual of filename list */
void refresh_fileview() {
  GtkListStore *store;
  GtkTreeModel *model;
  GtkTreeIter  treeiter;
  gint list_len;

  store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (treeview_files)));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_files));

  /* clear all rows in list */
  if (gtk_tree_model_get_iter_first(model, &treeiter) == TRUE) {
    gtk_list_store_clear(store);
  }

  GSList *iter;
  list_len = g_slist_length(cgrid_input_filenames);
  if (list_len > 0) {
    iter = cgrid_input_filenames;
    for (; iter; iter = iter->next) {
      add_to_fileview(iter->data);
    }
  }
  files_list_change_callback();
}

void files_list_change_callback() {
  gint list_len;
  gint adj_curr_upper;

  if(NULL != n_files_text) {
    free(n_files_text);
  }
  list_len = g_slist_length(cgrid_input_filenames);
  n_files_text = calloc(list_len/10+1, sizeof(char));
  sprintf(n_files_text, "Total files: %d", list_len);
  gtk_label_set_text(lbl_files_info, n_files_text);

  //adj_n_cols
  adj_curr_upper = gtk_adjustment_get_upper(adj_n_cols);
  gtk_adjustment_set_upper(adj_n_cols, list_len);
  if(gtk_adjustment_get_value(adj_n_cols) >= list_len) {
    // set the value to number of files
    gtk_adjustment_set_value(adj_n_cols, list_len);
  }
}


/* called when the user clicks on a filename row to update the preview widget */
static void select_filename (GtkTreeView *tree_view, gpointer data)
{
  GSList *selection = get_treeview_selection();

  if (selection != NULL && g_slist_length(selection) == 1) {
    update_selection((gchar*)(selection->data));
  }
  else {
    update_selection(NULL);
  }
}



/* updates the GUI according to the current selected filename */
static void update_selection (gchar* filename)
{
  /*
    g_free(selected_source_folder);
    if (filename != NULL) {
    // update preview
    GdkPixbuf *pixbuf_prev = gdk_pixbuf_new_from_file_at_scale(filename, FILE_PREVIEW_W - 20, FILE_PREVIEW_H - 30, TRUE, NULL);
    gtk_button_set_image(GTK_BUTTON(button_preview), gtk_image_new_from_pixbuf (pixbuf_prev));
    gtk_widget_show(button_preview);

    // update current selection
    selected_source_folder = g_path_get_dirname(filename);
    } else {
    // invalidate
    gtk_button_set_image(GTK_BUTTON(button_preview), NULL);
    gtk_widget_hide(button_preview);
    selected_source_folder = NULL;
    }

    gtk_widget_set_sensitive(button_samefolder, (selected_source_folder != NULL));
  */
}


static void open_file_chooser(GtkWidget *widget, gpointer data)
{
  GSList *selection;

  GtkFileFilter *filter_all, *supported[7];

  GtkWidget* file_chooser = gtk_file_chooser_dialog_new(
							"Select images",
							NULL,
							GTK_FILE_CHOOSER_ACTION_OPEN,
							GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE,
							GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
							);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);

  filter_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filter_all,"All supported types");

  supported[0] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[0], "Bitmap (*.bmp)");
  gtk_file_filter_add_pattern (supported[0], "*.[bB][mM][pP]");
  gtk_file_filter_add_pattern (filter_all, "*.[bB][mM][pP]");

  supported[1] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[1], "JPEG (*.jpg, *.jpeg, *jpe)");
  gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][gG]");
  gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][eE][gG]");
  gtk_file_filter_add_pattern (supported[1], "*.[jJ][pP][eE]");
  gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][gG]");
  gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][eE][gG]");
  gtk_file_filter_add_pattern (filter_all, "*.[jJ][pP][eE]");

  supported[2] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[2], "GIF (*.gif)");
  gtk_file_filter_add_pattern (supported[2], "*.[gG][iI][fF]");
  gtk_file_filter_add_pattern (filter_all, "*.[gG][iI][fF]");

  supported[3] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[3], "PNG (*.png)");
  gtk_file_filter_add_pattern (supported[3], "*.[pP][nN][gG]");
  gtk_file_filter_add_pattern (filter_all, "*.[pP][nN][gG]");

  supported[4] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[4], "Scalable Vector Graphics (*.svg)");
  gtk_file_filter_add_pattern (supported[4], "*.[sS][vV][gG]");
  gtk_file_filter_add_pattern (filter_all, "*.[sS][vV][gG]");

  supported[5] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[5], "TIFF (*tif, *.tiff)");
  gtk_file_filter_add_pattern (supported[5], "*.[tT][iI][fF][fF]");
  gtk_file_filter_add_pattern (supported[5], "*.[tT][iI][fF]");
  gtk_file_filter_add_pattern (filter_all, "*.[tT][iI][fF][fF]");
  gtk_file_filter_add_pattern (filter_all, "*.[tT][iI][fF]");

  supported[6] = gtk_file_filter_new();
  gtk_file_filter_set_name(supported[6], "GIMP XCF (*.xcf)");
  gtk_file_filter_add_pattern (supported[6], "*.[xX][cC][fF]");
  gtk_file_filter_add_pattern (filter_all, "*.[xX][cC][fF]");

  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), filter_all);
  int i;
  for(i = 0; i < 7; i++) {
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), supported[i]);
  }

  /* show dialog */
  if (gtk_dialog_run (GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) {
    selection = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_chooser));
    g_slist_foreach(selection, (GFunc)add_input_file, NULL);
  }

  gtk_widget_destroy (file_chooser);
}




static void open_folder_chooser(GtkWidget *widget, gpointer data)
{
  GSList *selection;
  gboolean include_subdirs;

  GtkWidget* folder_chooser = gtk_file_chooser_dialog_new(
							  "Select folders containing images",
							  NULL,
							  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
							  GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE,
							  GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
							  );
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(folder_chooser), TRUE);

  /* Add checkbox to select the depth of file search */
  GtkWidget* with_subdirs = gtk_check_button_new_with_label("Add files from the whole hierarchy");
  gtk_widget_show (with_subdirs);
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(folder_chooser), GTK_WIDGET(with_subdirs));

  /* show dialog */
  if (gtk_dialog_run (GTK_DIALOG(folder_chooser)) == GTK_RESPONSE_ACCEPT) {
    selection = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(folder_chooser));

    include_subdirs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(with_subdirs));
    g_slist_foreach(selection, (GFunc)add_input_folder, GINT_TO_POINTER(include_subdirs));
  }

  gtk_widget_destroy (folder_chooser);
}


static void open_addfiles_popupmenu(GtkWidget *widget, gpointer data) {
  gtk_menu_popup(GTK_MENU(popmenu_addfiles), NULL, NULL, NULL, NULL, 0, 0);
}

static void open_removefiles_popupmenu(GtkWidget *widget, gpointer data) {
  gtk_menu_popup(GTK_MENU(popmenu_removefiles), NULL, NULL, NULL, NULL, 0, 0);
}

/* initializes the file add/remove menus
 */
static void popmenus_init() {
  GtkWidget *menuitem;

  /* menu to add files to the list in various ways */
  popmenu_addfiles = gtk_menu_new();

  menuitem = gtk_menu_item_new_with_label("Add single images...");
  g_signal_connect(menuitem, "activate", G_CALLBACK(open_file_chooser), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
  menuitem = gtk_menu_item_new_with_label("Add folders...");
  g_signal_connect(menuitem, "activate", G_CALLBACK(open_folder_chooser), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
  //menuitem = gtk_menu_item_new_with_label(_("Add all opened images"));
  //g_signal_connect(menuitem, "activate", G_CALLBACK(add_opened_files), NULL);
  //gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);

  /* menu to remove files to the list */
  popmenu_removefiles = gtk_menu_new();

  menuitem = gtk_menu_item_new_with_label("Remove selected");
  g_signal_connect(menuitem, "activate", G_CALLBACK(remove_input_file), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_removefiles), menuitem);
  menuitem = gtk_menu_item_new_with_label("Remove all");
  g_signal_connect(menuitem, "activate", G_CALLBACK(remove_all_input_files), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_removefiles), menuitem);

  gtk_widget_show_all(popmenu_addfiles);
  gtk_widget_show_all(popmenu_removefiles);
}








/* shows an error dialog with a custom message */
void cgrid_show_error_dialog(char* message, GtkWidget* parent)
{
  GtkWidget* dialog = gtk_message_dialog_new (
					      GTK_WINDOW(parent),
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_OK,
					      message
					      );
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}



void cgrid_set_busy(gboolean busy) {
  GList *actions_children, *tmp_child;
  struct _ResponseData { gint response_id; };

  plugin_is_busy = busy;

  gtk_dialog_set_response_sensitive (GTK_DIALOG(cgrid_window_main), GTK_RESPONSE_CLOSE, !busy);
  gtk_dialog_set_response_sensitive (GTK_DIALOG(cgrid_window_main), GTK_RESPONSE_HELP, !busy);

  gtk_widget_set_sensitive(panel_options, !busy);
}
