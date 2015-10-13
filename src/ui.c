/* 
 */

#include "config.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "main.h"
#include "ui.h"
#include "cgrid_utils.h"
#include "plugin-intl.h"


/*  Constants  */

#define SCALE_WIDTH        180
#define SPIN_BUTTON_WIDTH   75
#define RANDOM_SEED_WIDTH  100


/*  Local function prototypes  */

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

/*static const gchar* progressbar_init_hidden ();
static void progressbar_start_hidden (const gchar *message, gboolean cancelable, gpointer user_data);
static void progressbar_end_hidden (gpointer user_data);
static void progressbar_settext_hidden (const gchar *message, gpointer user_data);
static void progressbar_setvalue_hidden (double percent, gpointer user_data);
static void cgrid_progress_bar_set(double fraction, char* text);
*/

static void cgrid_set_busy(gboolean busy);

void cgrid_show_error_dialog(char* message, GtkWidget* parent);

/*  Local variables  */

static PlugInUIVals *ui_state = NULL;


GtkWidget *panel_options;
GtkWidget *popmenu_add, *popmenu_edit, *popmenu_addfiles, *popmenu_removefiles;
GtkWidget *treeview_files;
//GtkWidget* progressbar_visible;
enum /* TreeView stuff... */
{
  LIST_ITEM = 0,
  N_COLUMNS
};

//const gchar* progressbar_data;

/*  Public functions  */

gboolean dialog (PlugInVals *vals, PlugInUIVals *ui_vals) {
  gboolean return_val = FALSE;
  GtkWidget* vbox_main;
    
  gimp_ui_init (PLUG_IN_BINARY, FALSE);
    
  cgrid_window_main = 
    gimp_dialog_new(
                    PLUG_IN_FULLNAME,
                    PLUG_IN_BINARY,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    GTK_STOCK_ABOUT, GTK_RESPONSE_HELP,
                    GTK_STOCK_OK,     GTK_RESPONSE_OK,
                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL
                    );
    
  gimp_window_set_transient (GTK_WINDOW(cgrid_window_main));
  gtk_widget_set_size_request (cgrid_window_main, MAIN_WINDOW_W, MAIN_WINDOW_H);
  gtk_window_set_resizable (GTK_WINDOW(cgrid_window_main), FALSE);
  gtk_window_set_position(GTK_WINDOW(cgrid_window_main), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(cgrid_window_main), 5);
    
  // Forces the visualization of label AND images on buttons (especially for Windows)
  GtkSettings *default_settings = gtk_settings_get_default();
  g_object_set(default_settings, "gtk-button-images", TRUE, NULL);
    
  vbox_main = gtk_vbox_new(FALSE, 10);
  panel_options = option_panel_new();
  popmenus_init();
  
  /*  
  progressbar_visible = gtk_progress_bar_new();
  gtk_widget_set_size_request (progressbar_visible, PROGRESSBAR_W, PROGRESSBAR_H);
  progressbar_data = progressbar_init_hidden();
  */
    
  gtk_box_pack_start(GTK_BOX(vbox_main), panel_options, FALSE, FALSE, 0);
  //gtk_box_pack_start(GTK_BOX(vbox_main), progressbar_visible, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(cgrid_window_main)->vbox), vbox_main);
  gtk_widget_show_all(cgrid_window_main);
  //gtk_widget_hide(button_preview);
    
  cgrid_set_busy(FALSE);
  
  gboolean retval=TRUE;
  while(TRUE) {
        gint run = gimp_dialog_run (GIMP_DIALOG(cgrid_window_main));
        if (run == GTK_RESPONSE_OK) {
          if (g_slist_length(cgrid_input_filenames) == 0) {
            cgrid_show_error_dialog(_("The file list is empty!"), cgrid_window_main);
          }
          else {
            /* Set options */
            vals->input_filenames = cgrid_input_filenames;
            //g_printf("processing files...\n\a");
            vals->input_nodes = NULL;
            retval = TRUE;
            break;
          }
        }
        else if (run == GTK_RESPONSE_HELP) {
          /* open_about();*/ 
        }
        else if (run == GTK_RESPONSE_CANCEL) {
            retval = FALSE;
            break;
        }
    }
  gtk_widget_destroy (cgrid_window_main);
  return retval;
}


gboolean dialog0 (PlugInVals *vals, PlugInUIVals *ui_vals) {
  GtkWidget *dlg;
  GtkWidget *vbox_main;
  GtkWidget *frame;
  GtkWidget *coordinates;
  GtkWidget *combo;
  gboolean   run = FALSE;
  GimpUnit   unit;

  ui_state = ui_vals;
  gimp_ui_init (PLUGIN_NAME, TRUE);

  dlg = gimp_dialog_new (_("GIMP Collection Grid Maker"), PLUGIN_NAME,
                         NULL, 0,
			 gimp_standard_help_func, "collection-grid-maker",

			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 GTK_STOCK_OK,     GTK_RESPONSE_OK,

			 NULL);

  vbox_main = gtk_vbox_new (FALSE, 12);

  panel_options = option_panel_new();

  /*
  progressbar_visible = gtk_progress_bar_new();
  gtk_widget_set_size_request (progressbar_visible, PROGRESSBAR_W, PROGRESSBAR_H);
  progressbar_data = progressbar_init_hidden();
  */

  gtk_box_pack_start(GTK_BOX(vbox_main), panel_options, FALSE, FALSE, 0);
  //gtk_box_pack_start(GTK_BOX(vbox_main), progressbar_visible, FALSE, FALSE, 0);

  gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dlg)->vbox), vbox_main);

  /*  gimp_scale_entry_new() examples  */

  frame = gimp_frame_new (_("ScaleEntry Examples"));
  gtk_box_pack_start (GTK_BOX (vbox_main), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  Show the main containers  */

  gtk_widget_show (vbox_main);
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
static gboolean dialog_image_constraint_func (gint32 image_id, gpointer  data) {
  return (gimp_image_base_type (image_id) == GIMP_RGB);
}



/* builds and returns the panel with file list and options */
static GtkWidget* option_panel_new() {
    GtkWidget *panel, *hbox;
    GtkWidget *hbox_buttons;
    GtkWidget *vbox_input;
    GtkWidget *scroll_input;
    GtkWidget *button_add, *button_remove;
    
    GtkWidget *vbox_useroptions, *hbox_outfolder;
    GtkWidget *label_chooser;
    
    panel = gtk_frame_new(_("Input files and options"));
    gtk_widget_set_size_request (panel, OPTION_PANEL_W, OPTION_PANEL_H);
    hbox = gtk_hbox_new(FALSE, 5);
    
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
    
    button_add = gtk_button_new_with_label(_("Add images"));
    gtk_widget_set_size_request(button_add, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
    button_remove = gtk_button_new_with_label(_("Remove images"));
    gtk_widget_set_size_request(button_remove, FILE_LIST_BUTTON_W, FILE_LIST_BUTTON_H);
    
    /* Sub-panel for options */
    vbox_useroptions = gtk_vbox_new(FALSE, 3);
    gtk_widget_set_size_request(vbox_useroptions, USEROPTIONS_PANEL_W, USEROPTIONS_PANEL_H);
    
    hbox_outfolder = gtk_hbox_new(FALSE, 3);
    label_chooser = gtk_label_new(_("Output folder"));
    
    
    /* All together */
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_remove, FALSE, FALSE, 0);
    
    gtk_container_add (GTK_CONTAINER(scroll_input), treeview_files);
        
    gtk_box_pack_start(GTK_BOX(vbox_input), scroll_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_input), hbox_buttons, FALSE, FALSE, 0);
    
   
    gtk_box_pack_start(GTK_BOX(vbox_useroptions), hbox_outfolder, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox), vbox_input, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_useroptions, FALSE, FALSE, 10);
    
    gtk_container_add(GTK_CONTAINER(panel), hbox);

    g_signal_connect(G_OBJECT(button_add), "clicked", G_CALLBACK(open_addfiles_popupmenu), NULL);
    g_signal_connect(G_OBJECT(button_remove), "clicked", G_CALLBACK(open_removefiles_popupmenu), NULL);
    g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_files))), "changed", G_CALLBACK(select_filename), NULL);
    
    init_fileview();
    refresh_fileview();

    return panel;
}




/* following: functions that modify the file list widget (addfile/addfolder/remove/removeall)  */

static void add_input_file(char* filename) 
{
    if (g_slist_find_custom(cgrid_input_filenames, filename, (GCompareFunc)strcmp) == NULL) {
        cgrid_input_filenames = g_slist_append(cgrid_input_filenames, filename);
        refresh_fileview();
    }
}

/* Recursive function to add all files from the hierarchy if desired */
static void add_input_folder_r(char* folder, gboolean with_subdirs) 
{
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
        cgrid_show_error_dialog(g_strdup_printf(_("Couldn't read into \"%s\" directory."), folder), cgrid_window_main);
    }
}


static void add_input_folder(char* folder, gpointer with_subdirs) 
{
    add_input_folder_r(folder, (gboolean)GPOINTER_TO_INT(with_subdirs));
    refresh_fileview();
}


/* returns the list of currently selected filenames (NULL of none) */
static GSList* get_treeview_selection() 
{
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


static void remove_input_file(GtkWidget *widget, gpointer data) 
{
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

static void remove_all_input_files(GtkWidget *widget, gpointer data) 
{
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

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (treeview_files)));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_files));

    /* clear all rows in list */
    if (gtk_tree_model_get_iter_first(model, &treeiter) == TRUE) {
        gtk_list_store_clear(store);
    }
    
    GSList *iter;
    if (g_slist_length(cgrid_input_filenames) > 0) {
        iter = cgrid_input_filenames;
        for (; iter; iter = iter->next) {
            add_to_fileview(iter->data);
        }
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
        _("Select images"), 
        NULL, 
        GTK_FILE_CHOOSER_ACTION_OPEN, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, 
        GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser), TRUE);
    
    filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all,_("All supported types"));
    
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
        _("Select folders containing images"), 
        NULL, 
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CLOSE, 
        GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(folder_chooser), TRUE);

    /* Add checkbox to select the depth of file search */
    GtkWidget* with_subdirs = gtk_check_button_new_with_label(_("Add files from the whole hierarchy"));
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
    
    menuitem = gtk_menu_item_new_with_label(_("Add single images..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(open_file_chooser), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Add folders..."));
    g_signal_connect(menuitem, "activate", G_CALLBACK(open_folder_chooser), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    //menuitem = gtk_menu_item_new_with_label(_("Add all opened images"));
    //g_signal_connect(menuitem, "activate", G_CALLBACK(add_opened_files), NULL);
    //gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_addfiles), menuitem);
    
    /* menu to remove files to the list */
    popmenu_removefiles = gtk_menu_new();
    
    menuitem = gtk_menu_item_new_with_label(_("Remove selected"));
    g_signal_connect(menuitem, "activate", G_CALLBACK(remove_input_file), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(popmenu_removefiles), menuitem);
    menuitem = gtk_menu_item_new_with_label(_("Remove all"));
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





/* suppress progress popup by installing progress handlers that do nothing */
/*
static const gchar* progressbar_init_hidden ()
{    
    GimpProgressVtable vtable = { 0, };

    vtable.start     = progressbar_start_hidden;
    vtable.end       = progressbar_end_hidden;
    vtable.set_text  = progressbar_settext_hidden;
    vtable.set_value = progressbar_setvalue_hidden;
  
    return gimp_progress_install_vtable (&vtable, NULL);
}
static void progressbar_start_hidden (const gchar *message, gboolean cancelable, gpointer user_data) { }
static void progressbar_end_hidden (gpointer user_data) { }
static void progressbar_settext_hidden (const gchar *message, gpointer user_data) { }
static void progressbar_setvalue_hidden (double percent, gpointer user_data) { }

void cgrid_progress_bar_set(double fraction, char* text) {
    if (fraction > 1.0) fraction = 1.0;
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar_visible), fraction);
    if (text != NULL) {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar_visible), text);
    }
}
*/

void cgrid_set_busy(gboolean busy) {
    GList *actions_children, *tmp_child;
    struct _ResponseData { gint response_id; };
    
    plugin_is_busy = busy;

    gtk_dialog_set_response_sensitive (GTK_DIALOG(cgrid_window_main), GTK_RESPONSE_CLOSE, !busy);
    gtk_dialog_set_response_sensitive (GTK_DIALOG(cgrid_window_main), GTK_RESPONSE_HELP, !busy);
    
    /* procedure that hides and shows some widgets in the dialog's action area. Compatible with GTK+ 2.16 */

    /*    
    GtkWidget* actions = gtk_dialog_get_action_area (GTK_DIALOG(cgrid_window_main));
    actions_children = gtk_container_get_children (GTK_CONTAINER (actions));
    tmp_child = actions_children;
    while (tmp_child != NULL)
    {
        GtkWidget *widget = tmp_child->data;
        struct _ResponseData *rd = g_object_get_data (G_OBJECT (widget), "gtk-dialog-response-data");

        if (rd && rd->response_id == GTK_RESPONSE_APPLY) {
            if (busy) {
                gtk_widget_hide (widget);
            } else {
                gtk_widget_show (widget);
            }
        }
        else if (rd && rd->response_id == GTK_RESPONSE_CANCEL) {
            if (!busy) {
                gtk_widget_hide (widget);
            } else {
                gtk_widget_show (widget);
            }
        }

        tmp_child = g_list_next (tmp_child);
    }
    g_list_free (actions_children);
    */
    
    gtk_widget_set_sensitive(panel_options, !busy);
}
