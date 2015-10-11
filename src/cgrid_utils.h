#ifndef __CGRID_UTILS_H__
#define __CGRID_UTILS_H__

#include <string.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>

char* str_replace(char*, char*, char*);
char* comp_get_filename(char*);
char* comp_get_filefolder(char*);
gboolean str_contains_cins(char*, char*);
gboolean file_has_extension(char*, char*);
GimpParamDef pdb_proc_get_param_info(gchar*, gint);
char* get_user_dir(void); 
char* get_cgrid_localedir(void);
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

#endif
