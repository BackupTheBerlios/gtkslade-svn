
#include "main.h"
#include "colours.h"
#include "editor_window.h"

col_config_t	*current;
string			cur_ccfg;

GtkWidget*		ccfg_combo;

vector<GtkWidget*>	buttons;
vector<GtkWidget*>	cboxes;
vector<rgba_t*>		colours;

extern rgba_t col_background;
extern rgba_t col_hilight;
extern rgba_t col_selection;
extern rgba_t col_moving;
extern rgba_t col_tagged;
extern rgba_t col_vertex;
extern rgba_t col_line_solid;
extern rgba_t col_line_2s;
extern rgba_t col_line_monster;
extern rgba_t col_line_special;
extern rgba_t col_selbox;
extern rgba_t col_selbox_line;
extern rgba_t col_grid;
extern rgba_t col_64grid;
extern rgba_t col_3d_crosshair;
extern rgba_t col_3d_hilight;
extern rgba_t col_3d_hilight_line;
extern rgba_t col_linedraw;

extern vector<col_config_t> colour_configs;

EXTERN_CVAR(String, col_config)

void config_changed(GtkWidget *w, gpointer data)
{
	set_colour_config(gtk_combo_box_get_active_text(GTK_COMBO_BOX(w)));
	force_map_redraw(true, true);
	col_config = gtk_combo_box_get_active_text(GTK_COMBO_BOX(w));
	current = &colour_configs[gtk_combo_box_get_active(GTK_COMBO_BOX(w))];

	for (int a = 0; a < buttons.size(); a++)
	{
		GdkColor gdk_col;
		gdk_col.red = colours[a]->r * 255;
		gdk_col.green = colours[a]->g * 255;
		gdk_col.blue = colours[a]->b * 255;
		gtk_color_button_set_color(GTK_COLOR_BUTTON(buttons[a]), &gdk_col);
		gtk_color_button_set_alpha(GTK_COLOR_BUTTON(buttons[a]), colours[a]->a * 255);

		if (colours[a]->blend == 1)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cboxes[a]), true);
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cboxes[a]), false);
	}
}

void colour_changed(GtkWidget *w, gpointer data)
{
	rgba_t *col = (rgba_t*)data;
	GdkColor gdk_col;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(w), &gdk_col);

	int alpha = (gtk_color_button_get_alpha(GTK_COLOR_BUTTON(w)) / 255);
	if (alpha > 255)
		alpha = 255;

	int red = (gdk_col.red / 255);
	if (red > 255)
		red = 255;

	int green = (gdk_col.green / 255);
	if (green > 255)
		green = 255;

	int blue = (gdk_col.blue / 255);
	if (blue > 255)
		blue = 255;

	col->set((BYTE)red, (BYTE)green, (BYTE)blue, (BYTE)alpha);

	force_map_redraw(true, true);
}

void additive_toggled(GtkWidget *w, gpointer data)
{
	rgba_t *col = (rgba_t*)data;
	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

	if (active)
		col->blend = 1;
	else
		col->blend = 0;
}

GtkWidget *setup_colour_editor(string col_name, rgba_t *col, string name)
{
	GtkWidget *hbox = gtk_hbox_new(false, 0);

	// Label
	GtkWidget *label = gtk_label_new(name.c_str());
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

	// Colour button
	GtkWidget *c_button = gtk_color_button_new();
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(c_button), true);
	gtk_color_button_set_title(GTK_COLOR_BUTTON(c_button), name.c_str());

	GdkColor gdk_col;
	gdk_col.red = col->r * 255;
	gdk_col.green = col->g * 255;
	gdk_col.blue = col->b * 255;
	gtk_color_button_set_color(GTK_COLOR_BUTTON(c_button), &gdk_col);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(c_button), col->a * 255);
	g_signal_connect(G_OBJECT(c_button), "color-set", G_CALLBACK(colour_changed), col);

	// Additive checkbox
	GtkWidget *cbox_add = gtk_check_button_new_with_label("Additive");
	g_signal_connect(G_OBJECT(cbox_add), "toggled", G_CALLBACK(additive_toggled), col);

	if (col->blend == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox_add), true);

	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);
	gtk_box_pack_start(GTK_BOX(hbox), c_button, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), cbox_add, false, false, 4);

	buttons.push_back(c_button);
	cboxes.push_back(cbox_add);
	colours.push_back(col);

	return hbox;
}

void populate_config_combo(GtkWidget *combo)
{
	int index = 0;

	// clear the combo box
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));

	while (gtk_tree_model_get_iter_first(model, &iter))
	{
		gtk_combo_box_remove_text(GTK_COMBO_BOX(combo), index);
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
	}

	index = 0;
	for (int a = 0; a < colour_configs.size(); a++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), colour_configs[a].name.c_str());
		string ccfg = col_config;

		if (ccfg == colour_configs[a].name)
		{
			index = a;
			current = &colour_configs[a];
		}
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
}

void new_config_clicked(GtkWidget *w, gpointer data)
{
	col_config_t ccfg;
	ccfg.name = entry_box("Enter Configuration Name");

	if (ccfg.name == "")
		return;

	ccfg.add("background", col_background);
	ccfg.add("hilight", col_hilight);
	ccfg.add("selection", col_selection);
	ccfg.add("moving", col_moving);
	ccfg.add("tagged", col_tagged);
	ccfg.add("vertex", col_vertex);
	ccfg.add("line_solid", col_line_solid);
	ccfg.add("line_2s", col_line_2s);
	ccfg.add("line_monster", col_line_monster);
	ccfg.add("line_special", col_line_special);
	ccfg.add("linedraw", col_linedraw);
	ccfg.add("selbox", col_selbox);
	ccfg.add("selbox_line", col_selbox_line);
	ccfg.add("grid", col_grid);
	ccfg.add("64grid", col_64grid);
	ccfg.add("3d_crosshair", col_3d_crosshair);
	ccfg.add("3d_hilight", col_3d_hilight);
	ccfg.add("3d_hilight_line", col_3d_hilight_line);

	ccfg.path = parse_string("config/colours/%s.cfg", ccfg.name.c_str());
	colour_configs.push_back(ccfg);
	current = &colour_configs.back();
	col_config = ccfg.name;

	populate_config_combo(ccfg_combo);
}

void save_config_clicked(GtkWidget *w, gpointer data)
{
	if (current->name == "Default")
	{
		message_box("You cannot overwrite the default colour scheme.", GTK_MESSAGE_INFO);
		return;
	}

	current->save();
}

GtkWidget *setup_colours_prefs()
{
	buttons.clear();
	cboxes.clear();
	colours.clear();

	GtkWidget *ret_vbox = gtk_vbox_new(false, 0);

	// Selection combo
	GtkWidget *frame = gtk_frame_new("Colour Configuration");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	ccfg_combo = gtk_combo_box_new_text();
	populate_config_combo(ccfg_combo);
	g_signal_connect(G_OBJECT(ccfg_combo), "changed", G_CALLBACK(config_changed), NULL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), ccfg_combo, false, false, 0);
	gtk_box_pack_start(GTK_BOX(ret_vbox), frame, false, false, 0);

	// Colour frames
	frame = gtk_frame_new("Colours");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(s_window), 4);
	vbox = gtk_vbox_new(true, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("background", &col_background, "Background"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("hilight", &col_hilight, "Hilighted Item"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("selection", &col_selection, "Selected Items"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("moving", &col_moving, "Moving Items"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("tagged", &col_tagged, "Tagged Items"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("vertex", &col_vertex, "Vertices"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("line_solid", &col_line_solid, "Solid Lines"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("line_2s", &col_line_2s, "2 Sided Lines"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("line_monster", &col_line_monster, "Monster Blocking Lines"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("line_special", &col_line_special, "Special Lines"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("linedraw", &col_linedraw, "Line-Draw Lines"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("selbox", &col_selbox, "Selection Box"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("selbox_line", &col_selbox_line, "Selection Box Outline"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("grid", &col_grid, "Grid"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("64grid", &col_64grid, "64 Grid"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("3d_crosshair", &col_3d_crosshair, "3d Mode Crosshair"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("3d_hilight", &col_3d_hilight, "3d Mode Hilight"), true, true, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_colour_editor("3d_hilight_line", &col_3d_hilight_line, "3d Mode Hilight Outline"), true, true, 0);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(s_window), vbox);
	gtk_container_add(GTK_CONTAINER(frame), s_window);
	gtk_box_pack_start(GTK_BOX(ret_vbox), frame, true, true, 0);

	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(ret_vbox), hbox, false, false, 0);

	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(""), true, true, 0);

	GtkWidget *button = gtk_button_new_with_label("New Config");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(new_config_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
	button = gtk_button_new_with_label("Save Config");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(save_config_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 4);

	return ret_vbox;
}
