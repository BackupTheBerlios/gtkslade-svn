
// INCLUDES ------------------------------ >>
#include "main.h"

// VARIABLES ----------------------------- >>
GtkListStore	*wads_store;
vector<string>	game_config_paths;

// EXTERNAL VARIABLES -------------------- >>
extern WadList wads;

static void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

bool setup_game_combo(GtkWidget *combo)
{
	GDir *dir = g_dir_open("./games/", 0, NULL);

	if (dir)
	{
		gchar dir_name[256] = "";
		sprintf(dir_name, "%s", g_dir_read_name(dir));

		while (g_ascii_strcasecmp(dir_name, "(null)") != 0)
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
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo), tokenizer.get_token().c_str());
			}

			sprintf(dir_name, "%s", g_dir_read_name(dir));
		}

		return true;
	}
	else
		message_box("Failed opening \"games\" directory!\nAre you sure SLADE is installed properly?\n", GTK_MESSAGE_ERROR);

	return false;
}

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

	// Setup the frame
	GtkWidget *frame = gtk_frame_new("Open Wads");
	gtk_container_add(GTK_CONTAINER(frame), wads_view);
	//gtk_widget_set_size_request(frame, 256, 256);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(box), frame, true, true, 0);
}

void populate_map_list(Wad *wad)
{
}

void setup_map_list(GtkWidget *box)
{
	GtkWidget *frame = gtk_frame_new("Available Maps");
	gtk_widget_set_size_request(frame, 64, 256);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(box), frame, true, true, 0);
}

bool setup_main_window(GtkWidget *window)
{
	gtk_widget_set_size_request(window, 400, 300);

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// Game combo
	GtkWidget *frame = gtk_frame_new("Game Configuration:");
	GtkWidget *game_combo = gtk_combo_box_new_text();
	if (!setup_game_combo(game_combo)) return false;
	gtk_container_add(GTK_CONTAINER(frame), game_combo);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);

	// Wad and Map list
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	setup_wad_list(hbox);
	setup_map_list(hbox);
	gtk_box_pack_start(GTK_BOX(main_vbox), hbox, true, true, 0);

	gtk_container_add(GTK_CONTAINER(window), main_vbox);

	return true;
}

GtkWidget* open_main_window()
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

	if (setup_main_window(window))
	{
		gtk_widget_show_all(window);
		return window;
	}
	else
		return NULL;
}
