
#include "main.h"

#ifdef WIN32
vector<DEVMODE> win32_modes;
#endif

CVAR(Int, vid_width_3d, -1, CVAR_SAVE)
CVAR(Int, vid_height_3d, -1, CVAR_SAVE)
CVAR(Int, vid_bpp_3d, -1, CVAR_SAVE)
CVAR(Int, vid_refresh_3d, -1, CVAR_SAVE)

EXTERN_CVAR(Bool, render_fog)
EXTERN_CVAR(Bool, render_fullbright)
EXTERN_CVAR(Bool, render_hilight)
EXTERN_CVAR(Int, render_things)
EXTERN_CVAR(Float, move_speed_3d)
EXTERN_CVAR(Float, mouse_speed_3d)
EXTERN_CVAR(Int, key_delay_3d)
EXTERN_CVAR(Bool, invert_mouse_3d)

void cbox_render_fog_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	render_fog = gtk_toggle_button_get_active(tb);
}

void cbox_render_fullbright_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	render_fullbright = gtk_toggle_button_get_active(tb);
}

void cbox_render_hilight_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	render_hilight = gtk_toggle_button_get_active(tb);
}

void cbox_render_things_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);

	if (gtk_toggle_button_get_active(tb))
	{
		gtk_widget_set_sensitive(GTK_WIDGET(data), true);
		GtkComboBox *combo = GTK_COMBO_BOX(data);
		render_things = gtk_combo_box_get_active(combo) + 1;
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(data), false);
		render_things = 0;
	}
}

void cbox_use_deskres_click(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gtk_widget_set_sensitive(GTK_WIDGET(data), false);
		vid_width_3d = -1;
		vid_height_3d = -1;
		vid_bpp_3d = -1;
		vid_refresh_3d = -1;
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(data), true);
		gtk_combo_box_set_active(GTK_COMBO_BOX(data), 0);
	}
}

void combo_resolution_changed(GtkWidget *widget, gpointer data)
{
	int index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

#ifdef WIN32
	vid_width_3d = win32_modes[index].dmPelsWidth;
	vid_height_3d = win32_modes[index].dmPelsHeight;
	vid_bpp_3d = win32_modes[index].dmBitsPerPel;
	vid_refresh_3d = win32_modes[index].dmDisplayFrequency;
#endif
}

void combo_thing_render_changed(GtkWidget *widget, gpointer data)
{
	GtkComboBox *combo = GTK_COMBO_BOX(widget);
	render_things = render_things = gtk_combo_box_get_active(combo) + 1;
}

void scale_move_speed_changed(GtkWidget *widget, gpointer data)
{
	GtkRange *range = GTK_RANGE(widget);
	double val = gtk_range_get_value(range);
	move_speed_3d = val;
}

void scale_mouse_speed_changed(GtkWidget *widget, gpointer data)
{
	GtkRange *range = GTK_RANGE(widget);
	double val = gtk_range_get_value(range);
	mouse_speed_3d = val;
}

void entry_key_delay_changed(GtkWidget *widget, gpointer data)
{
	int val = atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
	key_delay_3d = val;
}

void cbox_invert_mouse_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	invert_mouse_3d = gtk_toggle_button_get_active(tb);
}

GtkWidget* setup_3dmode_prefs()
{
	GtkWidget *mvbox = gtk_vbox_new(false, 0);

	// Rendering options
	GtkWidget *frame = gtk_frame_new("Rendering");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(mvbox), frame, false, false, 0);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	GtkWidget *check_button = gtk_check_button_new_with_label("Render Fog");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_render_fog_click), NULL);
	if (render_fog)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Fullbright");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_render_fullbright_click), NULL);
	if (render_fullbright)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Draw Hilighted Wall/Flat");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_render_hilight_click), NULL);
	if (render_hilight)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	GtkWidget* hbox = gtk_hbox_new(false, 10);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	GtkWidget* combo = gtk_combo_box_new_text();

	check_button = gtk_check_button_new_with_label("Draw Things");
	gtk_box_pack_start(GTK_BOX(hbox), check_button, false, false, 0);
	if (render_things > 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);
	else
		gtk_widget_set_sensitive(combo, false);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_render_things_click), combo);

	gtk_box_pack_start(GTK_BOX(hbox), combo, true, true, 0);
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Sprites Only");
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Sprites + Floor Boxes");
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Sprites + Full Boxes");
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_thing_render_changed), NULL);

	if (render_things > 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), render_things - 1);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);



	// Input options
	frame = gtk_frame_new("Input");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(mvbox), frame, false, false, 0);

	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	// Move speed
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	GtkWidget *label = gtk_label_new("Movement Speed:");
	gtk_widget_set_size_request(label, 128, -1);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *hscale = gtk_hscale_new_with_range(0.1, 0.5, 0.05);
	g_signal_connect(G_OBJECT(hscale), "value-changed", G_CALLBACK(scale_move_speed_changed), NULL);
	gtk_range_set_value(GTK_RANGE(hscale), move_speed_3d);
	gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), hscale, true, true, 4);

	// Mouse speed
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("Mouse Speed:");
	gtk_widget_set_size_request(label, 128, -1);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	hscale = gtk_hscale_new_with_range(0.5, 2.0, 0.1);
	g_signal_connect(G_OBJECT(hscale), "value-changed", G_CALLBACK(scale_mouse_speed_changed), NULL);
	gtk_range_set_value(GTK_RANGE(hscale), mouse_speed_3d);
	gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), hscale, true, true, 4);

	// Invert mouse
	check_button = gtk_check_button_new_with_label("Invert Y");
	gtk_box_pack_start(GTK_BOX(hbox), check_button, false, false, 0);
	if (invert_mouse_3d)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_invert_mouse_click), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), check_button, true, true, 4);

	// Key delay
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("Key Repeat Delay:");
	gtk_widget_set_size_request(label, 128, -1);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	int val = key_delay_3d;
	gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", val).c_str());
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(entry_key_delay_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, true, true, 4);

	// Resolution (windows only for now)
#ifdef WIN32

	DEVMODE dm;

	frame = gtk_frame_new("Resolution");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(mvbox), frame, false, false, 0);

	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	combo = gtk_combo_box_new_text();
	check_button = gtk_check_button_new_with_label("Use desktop resolution");
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_use_deskres_click), combo);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_resolution_changed), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), combo, false, false, 0);

	int i = 0;
	int index = -1;
	bool done = false;
	win32_modes.clear();
	while (!done)
	{
		if (EnumDisplaySettings(0, i, &dm))
		{
			// Only allow modes with a specific refresh, and >= 640x400x16
			if (dm.dmBitsPerPel >= 16 && dm.dmDisplayFrequency > 0 && dm.dmPelsWidth >= 640 && dm.dmPelsHeight >= 400)
			{
				win32_modes.push_back(dm);

				// Get a string describing the mode
				string mode = parse_string("%dx%d, %dbpp, %dHz",
											dm.dmPelsWidth, dm.dmPelsHeight,
											dm.dmBitsPerPel, dm.dmDisplayFrequency);
				
				//log_message("%s\n", mode.c_str());
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo), mode.c_str());

				if (vid_width_3d == dm.dmPelsWidth &&
					vid_height_3d == dm.dmPelsHeight &&
					vid_bpp_3d == dm.dmBitsPerPel &&
					vid_refresh_3d == dm.dmDisplayFrequency)
					index = win32_modes.size() - 1;
			}

			i++;
		}
		else
			done = true;
	}

	if (index == -1)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);
		gtk_widget_set_sensitive(combo, false);
	}
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);

#endif

	return mvbox;
}
