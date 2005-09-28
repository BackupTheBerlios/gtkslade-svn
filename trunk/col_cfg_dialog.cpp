
#include "main.h"
#include "colours.h"
#include "misc.h"
#include "editor_window.h"

string cur_ccfg;

vector<GtkWidget*>	buttons;
vector<GtkWidget*>	cboxes;
vector<rgba_t*>		colours;

extern GtkWidget *editor_window;

extern rgba_t col_background;
extern rgba_t col_hilight;
extern rgba_t col_selection;
extern rgba_t col_moving;
extern rgba_t col_tagged;
extern rgba_t col_vertex;
extern rgba_t col_line_solid;
extern rgba_t col_line_2s;
extern rgba_t col_line_monster;
extern rgba_t col_selbox;
extern rgba_t col_selbox_line;
extern rgba_t col_grid;
extern rgba_t col_64grid;
extern rgba_t col_3d_crosshair;
extern rgba_t col_3d_hilight;
extern rgba_t col_3d_hilight_line;

extern vector<col_config_t> colour_configs;

EXTERN_CVAR(String, col_config)

void config_changed(GtkWidget *w, gpointer data)
{
	set_colour_config(gtk_combo_box_get_active_text(GTK_COMBO_BOX(w)));
	force_map_redraw(true, true);

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
	GdkColor *gdk_col;

	gtk_color_button_get_color(GTK_COLOR_BUTTON(w), gdk_col);
	int alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(w));
	col->set((BYTE)(gdk_col->red / 255), (BYTE)(gdk_col->green / 255),
			(BYTE)(gdk_col->blue / 255), (BYTE)(alpha / 255), col->blend);

	force_map_redraw(true, true);
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

GtkWidget *setup_ccfg_dialog()
{
	GtkWidget *ret_vbox = gtk_vbox_new(false, 0);

	// Selection combo
	GtkWidget *frame = gtk_frame_new("Colour Configuration");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	GtkWidget *combo = gtk_combo_box_new_text();
	int index = 0;

	for (int a = 0; a < colour_configs.size(); a++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), colour_configs[a].name.c_str());
		string ccfg = col_config;

		if (ccfg == colour_configs[a].name)
		{
			index = a;
			cur_ccfg = ccfg;
		}
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(config_changed), NULL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), combo, false, false, 0);
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

	return ret_vbox;
}

void open_colour_select_dialog()
{
	buttons.clear();
	colours.clear();
	cboxes.clear();

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Colours Configuration",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													NULL);
	
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_ccfg_dialog());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), -1, 400);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
}
