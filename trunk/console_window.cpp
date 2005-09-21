
#include "main.h"
#include "console.h"

GtkWidget *console_window;
GtkWidget *s_window;

extern GtkTextBuffer *console_log;
extern string cmd_line;

void entry_activate(GtkWidget *w, gpointer data)
{
	cmd_line = gtk_entry_get_text(GTK_ENTRY(w));
	gtk_entry_set_text(GTK_ENTRY(w), "");
	console_parsecommand();
	wait_gtk_events();

	GtkWidget *sbar = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(s_window));
	double upper = GTK_ADJUSTMENT(GTK_RANGE(sbar)->adjustment)->upper;
	gtk_range_set_value(GTK_RANGE(sbar), upper);
}

void setup_console_window()
{
	console_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(console_window), "SLADE Console");
	gtk_widget_set_size_request(console_window, 500, 300);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Console history
	s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *history_view = gtk_text_view_new_with_buffer(console_log);
	gtk_container_add(GTK_CONTAINER(s_window), history_view);
	gtk_widget_modify_font(history_view, pango_font_description_from_string("Courier 11"));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(history_view), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(history_view), false);

	gtk_box_pack_start(GTK_BOX(vbox), s_window, true, true, 0);

	// Command entry box
	GtkWidget *cmd_entry = gtk_entry_new();
	gtk_widget_modify_font(cmd_entry, pango_font_description_from_string("Courier 11"));
	g_signal_connect(G_OBJECT(cmd_entry), "activate", G_CALLBACK(entry_activate), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), cmd_entry, false, false, 4);

	gtk_container_add(GTK_CONTAINER(console_window), vbox);
}

void popup_console()
{
	gtk_widget_show_all(console_window);
	gtk_window_present(GTK_WINDOW(console_window));
}
