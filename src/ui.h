/* 
 */

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

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


GtkWidget* cgrid_window_main;

/*  Public functions  */

gboolean   dialog (
		   PlugInVals         *vals,
		   PlugInUIVals       *ui_vals);


#endif /* __INTERFACE_H__ */
