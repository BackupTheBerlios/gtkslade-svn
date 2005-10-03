
#include "main.h"
#include "textures.h"

extern GtkWidget *editor_window;

string open_texture_browser()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Textures",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	//gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_prefs_dialog());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 400);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));

	return "";
}
