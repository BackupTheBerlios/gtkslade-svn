
#include "main.h"
#include "console.h"

GtkWidget *console_window;

extern GtkTextBuffer *console_log;

void setup_console_window()
{
	console_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	GtkWidget *history_view = gtk_text_view_new_with_buffer(console_log);
	gtk_widget_modify_font(history_view, pango_font_description_from_string("Courier New 12"));
	gtk_box_pack_start(GTK_BOX(vbox), history_view, true, true, 0);

	GtkWidget *cmd_entry = gtk_entry_new();
	gtk_widget_modify_font(cmd_entry, pango_font_description_from_string("Courier New 12"));
	gtk_box_pack_start(GTK_BOX(vbox), cmd_entry, false, false, 4);

	gtk_container_add(GTK_CONTAINER(console_window), vbox);
}

void popup_console()
{
	gtk_widget_show_all(console_window);
	gtk_window_present(GTK_WINDOW(console_window));
}
