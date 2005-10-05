
// Includes ------------------------------ >>
#include "main.h"
#include "misc.h"
#include "game_config.h"
#include "map.h"
#include "editor_window.h"
#include "textures.h"
#include "console_window.h"

// Variables ----------------------------- >>
GtkListStore	*wads_store;
GtkListStore	*maps_store;
Wad*			selected_wad;
GtkWidget		*wad_manager_window;
bool			game_changed = true;

CVAR(String, game_config, "Doom 2", CVAR_SAVE)

// External Variables -------------------- >>
extern WadList wads;
extern vector<string> game_config_paths;
extern vector<string> game_config_names;
extern Map map;
extern Wad *edit_wad;

void populate_wad_list()
{
	GtkTreeIter iter;

	gtk_list_store_clear(wads_store);

	gtk_list_store_append (wads_store, &iter);
	gtk_list_store_set (wads_store, &iter,
						0, wads.get_iwad()->path.c_str(),
						1, -1,
						-1);

	for (int i = 0; i < wads.n_wads; i++)
	{
		gtk_list_store_append (wads_store, &iter);
		gtk_list_store_set (wads_store, &iter,
							0, wads.get_wad(i)->path.c_str(),
							1, i,
							-1);
	}
}

void game_combo_changed(GtkWidget *w, gpointer data)
{
	int index = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

	if (!load_game_config(index))
		message_box(parse_string("Error loading game configuration \"%s\"", game_config_paths[index].c_str()),
					GTK_MESSAGE_ERROR);
	else
	{
		game_config = game_config_names[index];
		populate_wad_list();
	}

	game_changed = true;
}

static void destroy(GtkWidget *widget, gpointer data)
{
    //gtk_main_quit();
}

bool setup_game_combo(GtkWidget *combo)
{

	GDir *dir = g_dir_open("./games/", 0, NULL);

	if (dir)
	{
		int index = 0;
		string def = game_config;
		const gchar* dir_name = g_dir_read_name(dir);

		while (dir_name)
		{
			if (g_str_has_suffix(dir_name, ".cfg"))
			{
				Tokenizer tokenizer;
				string filename = "games/";
				filename += dir_name;
				tokenizer.open_file(filename, 0, 0);

				string temp = tokenizer.get_token();

				if (temp == "game") // If it's a valid SLADE game configuration
				{
					// Add the file path to the game config list
					game_config_paths.push_back(filename);

					// And add the game name to the combo
					string name = tokenizer.get_token();
					gtk_combo_box_append_text(GTK_COMBO_BOX(combo), name.c_str());
					game_config_names.push_back(name);
	
					if (name == def)
						index = game_config_names.size() - 1;
				}
			}

			dir_name = g_dir_read_name(dir);
		}

		g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(game_combo_changed), NULL);
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);

		return true;
	}
	else
		message_box("Failed opening \"games\" directory!\nAre you sure SLADE is installed properly?\n", GTK_MESSAGE_ERROR);

	return false;
}

void populate_map_list(Wad *wad)
{
	GtkTreeIter iter;

	gtk_list_store_clear(maps_store);

	for (int i = 0; i < wad->available_maps.size(); i++)
	{
		gtk_list_store_append(maps_store, &iter);
		gtk_list_store_set(maps_store, &iter,
							0, wad->available_maps[i].c_str(),
							1, i,
							-1);
	}
}

void wads_list_changed(GtkWidget *w, gpointer data)
{
	GtkTreePath *path;
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(w), &path, NULL);
	int index = atoi(gtk_tree_path_to_string(path));

	if (index == 0)
		selected_wad = wads.get_iwad();
	else
		selected_wad = wads.get_wad(index - 1);

	populate_map_list(selected_wad);
	
	gtk_tree_path_free(path);
}

void open_wad_click()
{
	string wadfile = file_browser("*.wad");

	if (wadfile != "")
	{
		if (!wads.open_wad(wadfile))
			message_box(parse_string("Failed opening wadfile \"%s\"", wadfile.c_str()), GTK_MESSAGE_ERROR);
	}

	populate_wad_list();
	game_changed = true;
}

void close_wad_click()
{
	wads.close_wad(selected_wad->path);
	populate_wad_list();
}

void close_all_click()
{
	wads.close_all();
	populate_wad_list();
}

void new_standalone_click()
{
	string mapname = entry_box("Enter map name");

	if (mapname != "")
	{
		open_map(NULL, mapname);
		gtk_widget_hide(wad_manager_window);

		if (game_changed)
		{
			popup_console();
			load_flats();
			load_textures();
			load_sprites();
			game_changed = false;
			hide_console();
		}
	}
}

void setup_wad_list(GtkWidget *box)
{
	// Setup the list model
	wads_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	populate_wad_list();

	// Setup the list view
	GtkWidget *wads_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(wads_store));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Path", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(wads_view), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(wads_view), false);
	g_signal_connect(G_OBJECT(wads_view), "cursor-changed", G_CALLBACK(wads_list_changed), NULL);

	// Vbox
	GtkWidget *vbox = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Setup the scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(s_window), wads_view);
	gtk_box_pack_start(GTK_BOX(vbox), s_window, true, true, 0);

	// Setup the button box
	GtkWidget *hbox = gtk_hbox_new(false, 4);

	// Open button
	GtkWidget *button = gtk_button_new_with_label("Open Wad");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(open_wad_click), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	// Close button
	button = gtk_button_new_with_label("Close Wad");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_wad_click), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	// Close All button
	button = gtk_button_new_with_label("Close All");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_all_click), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	// New Standalone button
	button = gtk_button_new_with_label("New Standalone Map");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(new_standalone_click), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

	// Setup the frame
	GtkWidget *frame = gtk_frame_new("Open Wads");
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(box), frame, true, true, 0);
}

void maps_list_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data)
{
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(maps_store), &iter, path);

	gchar* mapname;
	gtk_tree_model_get(GTK_TREE_MODEL(maps_store), &iter, 0, &mapname, -1);

	open_map(selected_wad, mapname);
	gtk_widget_hide(wad_manager_window);
	
	if (game_changed)
	{
		popup_console();
		load_flats();
		load_textures();
		load_sprites();
		game_changed = false;
		hide_console();
	}

	g_free(mapname);
}

void setup_map_list(GtkWidget *box)
{
	// Setup the list model
	maps_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	// Setup the list view
	GtkWidget *maps_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(maps_store));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Map Name", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(maps_view), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(maps_view), false);
	g_signal_connect(G_OBJECT(maps_view), "row-activated", G_CALLBACK(maps_list_activated), NULL);

	// Vbox
	GtkWidget *vbox = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// Setup the scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(s_window), maps_view);
	gtk_box_pack_start(GTK_BOX(vbox), s_window, true, true, 0);

	// Setup the button
	GtkWidget *button = gtk_button_new_with_label("New Map");
	gtk_box_pack_start(GTK_BOX(vbox), button, false, false, 0);

	// Setup the frame
	GtkWidget *frame = gtk_frame_new("Available Maps");
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(box), frame, false, false, 0);
}

void setup_main_window()
{
	wad_manager_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(wad_manager_window), "SLADE");
	g_signal_connect(G_OBJECT(wad_manager_window), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	//gtk_widget_hide_on_delete(wad_manager_window);
	gtk_widget_set_size_request(wad_manager_window, -1, 300);
	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// Game combo
	GtkWidget *frame = gtk_frame_new("Game Configuration:");
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	GtkWidget *game_combo = gtk_combo_box_new_text();
	setup_game_combo(game_combo);

	gtk_box_pack_start(GTK_BOX(vbox), game_combo, true, true, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);

	// Wad and Map list
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	setup_wad_list(hbox);
	setup_map_list(hbox);
	gtk_box_pack_start(GTK_BOX(main_vbox), hbox, true, true, 0);

	gtk_container_add(GTK_CONTAINER(wad_manager_window), main_vbox);
}

void open_main_window()
{
	/*
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "SLADE");
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_widget_hide_on_delete()), NULL);

	if (setup_main_window(window))
		gtk_widget_show_all(window);
		*/

	gtk_widget_show_all(wad_manager_window);
	gtk_window_present(GTK_WINDOW(wad_manager_window));
}
