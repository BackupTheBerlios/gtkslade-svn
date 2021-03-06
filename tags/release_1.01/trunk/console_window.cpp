// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << console_window.cpp - Console window  >>
// << functions                            >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "console.h"
#include "misc.h"

// Variables ----------------------------- >>
GtkWidget	*console_window;
GtkWidget	*s_window;
GtkWidget	*progress;
bool		showprogress = false;

// External Variables -------------------- >>
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

	/*
	GtkAdjustment *a = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(s_window));
	GtkAdjustment *na = GTK_ADJUSTMENT(gtk_adjustment_new(a->upper,
															a->lower,
															a->upper,
															a->step_increment,
															a->page_increment,
															a->page_size));
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(s_window), na);
	*/

	wait_gtk_events();
}

gboolean console_destroy(GtkWidget *w, gpointer data)
{
	gtk_widget_hide(w);
	return true;
}

gboolean console_entry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (event->keyval == 0xFF52)
	{
		console_prevcommand();
		gtk_entry_set_text(GTK_ENTRY(widget), cmd_line.c_str());
		gtk_entry_set_position(GTK_ENTRY(widget), -1);
		return true;
	}

	if (event->keyval == 0xFF54)
	{
		console_nextcommand();
		gtk_entry_set_text(GTK_ENTRY(widget), cmd_line.c_str());
		gtk_entry_set_position(GTK_ENTRY(widget), -1);
		return true;
	}

	return false;
}

void setup_console_window()
{
	console_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(console_window), "Console");
	gtk_widget_set_size_request(console_window, 550, 300);
	g_signal_connect(G_OBJECT(console_window), "delete_event", G_CALLBACK(console_destroy), NULL);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Console history
	s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *history_view = gtk_text_view_new_with_buffer(console_log);
	gtk_container_add(GTK_CONTAINER(s_window), history_view);
	//gtk_widget_modify_font(history_view, pango_font_description_from_string("Courier 8"));
	widget_set_font(history_view, "Courier", 8);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(history_view), false);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(history_view), false);

	gtk_box_pack_start(GTK_BOX(vbox), s_window, true, true, 0);

	// Progress bar
	progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), progress, false, false, 4);

	// Command entry box
	GtkWidget *cmd_entry = gtk_entry_new();
	//gtk_widget_modify_font(cmd_entry, pango_font_description_from_string("Courier 8"));
	widget_set_font(cmd_entry, "Courier", 8);
	g_signal_connect(G_OBJECT(cmd_entry), "activate", G_CALLBACK(entry_activate), NULL);
	g_signal_connect(G_OBJECT(cmd_entry), "key_press_event", G_CALLBACK(console_entry_keypress), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), cmd_entry, false, false, 4);

	gtk_container_add(GTK_CONTAINER(console_window), vbox);
}

void popup_console()
{
	gtk_widget_show_all(console_window);

	if (!showprogress)
		gtk_widget_hide(progress);

	gtk_window_present(GTK_WINDOW(console_window));
}

void hide_console()
{
	gtk_widget_hide(console_window);
}

void show_progress()
{
	showprogress = true;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0);
	gtk_widget_show(progress);
}

void hide_progress()
{
	showprogress = false;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0);
	gtk_widget_hide(progress);
}

void update_progress(double val)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), val);
	wait_gtk_events();
}
