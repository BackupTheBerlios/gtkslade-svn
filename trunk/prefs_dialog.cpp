
#include "main.h"
#include "prefs_dialog.h"

extern GtkWidget *editor_window;

GtkWidget* setup_prefs_dialog()
{
	GtkWidget *notebook = gtk_notebook_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Visual prefs here"), gtk_label_new("Visual"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup_colours_prefs(), gtk_label_new("Colours"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Node Builder prefs here"), gtk_label_new("Node Builder"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Misc prefs here"), gtk_label_new("Misc"));

	return notebook;
}

void open_prefs_dialog()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Preferences",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_prefs_dialog());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 400);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
