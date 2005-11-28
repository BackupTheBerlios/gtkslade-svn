
#include "main.h"
#include "prefs_dialog.h"
#include "editor_window.h"

extern GtkWidget *editor_window;
extern int edit_mode;

EXTERN_CVAR(Bool, grid_dashed)
EXTERN_CVAR(Bool, grid_64grid)
EXTERN_CVAR(Bool, edit_snap_grid)
EXTERN_CVAR(Bool, thing_sprites)
EXTERN_CVAR(Bool, thing_force_angle)
EXTERN_CVAR(Bool, line_aa)
EXTERN_CVAR(Float, line_size)
EXTERN_CVAR(Bool, allow_np2_tex)
EXTERN_CVAR(Int, tex_filter)

void cbox_grid_dashed_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	grid_dashed = gtk_toggle_button_get_active(tb);
	force_map_redraw(false, true);
}

void cbox_grid_64grid_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	grid_64grid = gtk_toggle_button_get_active(tb);
	force_map_redraw(false, true);
}

void cbox_snap_grid_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	edit_snap_grid = gtk_toggle_button_get_active(tb);
}

void cbox_thing_sprites_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	thing_sprites = gtk_toggle_button_get_active(tb);

	if (edit_mode == 3)
		force_map_redraw(true, false);
}

void cbox_thing_force_angle_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	thing_force_angle = gtk_toggle_button_get_active(tb);

	if (edit_mode == 3)
		force_map_redraw(true, false);
}

void cbox_line_aa_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	line_aa = gtk_toggle_button_get_active(tb);
	force_map_redraw(true, false);
}

void scale_line_width_changed(GtkWidget *widget, gpointer data)
{
	GtkRange *range = GTK_RANGE(widget);
	double val = gtk_range_get_value(range);
	line_size = val;
	force_map_redraw(true, false);
}

void cbox_allow_np2_tex_click(GtkWidget *widget, gpointer data)
{
	GtkToggleButton *tb = GTK_TOGGLE_BUTTON(widget);
	allow_np2_tex = gtk_toggle_button_get_active(tb);
}

void combo_tex_filter_changed(GtkComboBox *widget, gpointer data)
{
	int index = gtk_combo_box_get_active(widget);
	tex_filter = index + 1;
}

GtkWidget* setup_visual_prefs()
{
	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// Editor options
	GtkWidget *frame = gtk_frame_new("Editor");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, true, true, 0);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	GtkWidget *check_button = gtk_check_button_new_with_label("Grid dashed");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_grid_dashed_click), NULL);
	if (grid_dashed)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Show 64-unit grid");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_grid_64grid_click), NULL);
	if (grid_64grid)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Snap to grid");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_snap_grid_click), NULL);
	if (edit_snap_grid)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Show things as sprites");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_thing_sprites_click), NULL);
	if (thing_sprites)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	check_button = gtk_check_button_new_with_label("Draw all thing angles");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_thing_force_angle_click), NULL);
	if (thing_force_angle)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	// Line options
	frame = gtk_frame_new("Lines");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	GtkWidget *label = gtk_label_new("Width:");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *hscale = gtk_hscale_new_with_range(1.0, 3.0, 0.05);
	g_signal_connect(G_OBJECT(hscale), "value-changed", G_CALLBACK(scale_line_width_changed), NULL);
	gtk_range_set_value(GTK_RANGE(hscale), line_size);
	gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), hscale, true, true, 4);

	check_button = gtk_check_button_new_with_label("Antialias");
	gtk_box_pack_start(GTK_BOX(hbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_line_aa_click), NULL);
	if (line_aa)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	// Texture options
	frame = gtk_frame_new("Textures");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	check_button = gtk_check_button_new_with_label("Allow non-power-of-two textures");
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	g_signal_connect(G_OBJECT(check_button), "toggled", G_CALLBACK(cbox_allow_np2_tex_click), NULL);
	if (allow_np2_tex)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), true);

	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("Filter: ");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);

	GtkWidget *combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "None");
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Bilinear");
	gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Trilinear");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), tex_filter - 1);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_tex_filter_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), combo, true, true, 0);

	return main_vbox;
}


GtkWidget* setup_prefs_dialog()
{
	GtkWidget *notebook = gtk_notebook_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup_visual_prefs(), gtk_label_new("Visual"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup_colours_prefs(), gtk_label_new("Colours"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup_input_prefs(), gtk_label_new("Input"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup_3dmode_prefs(), gtk_label_new("3d Mode"));
	//gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Node Builder prefs here"), gtk_label_new("Node Builder"));
	//gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Misc prefs here"), gtk_label_new("Misc"));

	return notebook;
}

void open_prefs_dialog()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Preferences",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
	//												GTK_STOCK_CANCEL,
	//												GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_prefs_dialog());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 400);
	gtk_widget_show_all(dialog);

	/*int response = */gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
