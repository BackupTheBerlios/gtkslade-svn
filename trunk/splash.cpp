
#include "main.h"

GtkWidget* splash_window;
GtkWidget* splash_pbar;

extern GtkWidget* editor_window;

void setup_splash()
{
	splash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(splash_window), false);
	gtk_window_set_has_frame(GTK_WINDOW(splash_window), false);
	gtk_window_set_transient_for(GTK_WINDOW(splash_window), GTK_WINDOW(editor_window));
	gtk_window_set_keep_above(GTK_WINDOW(splash_window), true);
	gtk_window_set_position(GTK_WINDOW(splash_window), GTK_WIN_POS_CENTER);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(splash_window), true);

	// Frame
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	
	// Image
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	GtkWidget *image = gtk_image_new_from_file("res/logo.png");
	gtk_widget_set_size_request(image, 589, 230);
	gtk_box_pack_start(GTK_BOX(vbox), image, true, true, 0);

	// Progress bar
	splash_pbar = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), splash_pbar, false, false, 0);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_add(GTK_CONTAINER(splash_window), frame);

	gtk_widget_set_size_request(splash_window, -1, splash_pbar->allocation.height + 230);
}

void splash(string message)
{
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(splash_pbar), message.c_str());
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(splash_pbar), 0);
	gtk_window_set_position(GTK_WINDOW(splash_window), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(splash_window);
	gtk_window_present(GTK_WINDOW(splash_window));
	wait_gtk_events();
}

void splash_hide()
{
	gtk_widget_hide(splash_window);
}

void splash_progress(double prog)
{
	gtk_window_set_position(GTK_WINDOW(splash_window), GTK_WIN_POS_CENTER);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(splash_pbar), prog);
	wait_gtk_events();
}
