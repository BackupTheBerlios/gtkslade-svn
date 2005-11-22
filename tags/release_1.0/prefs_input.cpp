
#include "main.h"
#include "keybind.h"

GtkWidget*	key_bind_entry = NULL;
GtkWidget*	key_set_accept = NULL;
GtkWidget*	key_default_cancel = NULL;
bool		key_set = false;
int			key_index = -1;

extern BindList binds;

void setup_keys_store(GtkListStore* store)
{
	GtkTreeIter iter;

	for (int a = 0; a < binds.n_keys(); a++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, binds.get_name(a).c_str(), -1);
	}
}

void keys_list_changed(GtkWidget *w, gpointer data)
{
	GtkTreePath *path;
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(w), &path, NULL);
	key_index = atoi(gtk_tree_path_to_string(path));

	gtk_entry_set_text(GTK_ENTRY(key_bind_entry), binds.get_bind(key_index).c_str());
}

void key_set_clicked(GtkWidget *w, gpointer data)
{
	if (key_index == -1)
		return;

	if (!key_set)
	{
		gtk_button_set_label(GTK_BUTTON(key_set_accept), "Accept");
		gtk_button_set_label(GTK_BUTTON(key_default_cancel), "Cancel");
		gtk_entry_set_text(GTK_ENTRY(key_bind_entry), "");
		gtk_widget_set_sensitive(key_bind_entry, true);
	}
	else
	{
		binds.change(binds.get_name(key_index), gtk_entry_get_text(GTK_ENTRY(key_bind_entry)));
		gtk_button_set_label(GTK_BUTTON(key_set_accept), "Set");
		gtk_button_set_label(GTK_BUTTON(key_default_cancel), "Default");
		gtk_widget_set_sensitive(key_bind_entry, false);
	}

	key_set = !key_set;
}

void key_default_clicked(GtkWidget *w, gpointer data)
{
	if (key_index == -1)
		return;

	if (key_set)
	{
		key_set = false;
		gtk_button_set_label(GTK_BUTTON(key_set_accept), "Set");
		gtk_button_set_label(GTK_BUTTON(key_default_cancel), "Default");
		gtk_widget_set_sensitive(key_bind_entry, false);
		gtk_entry_set_text(GTK_ENTRY(key_bind_entry), binds.get_bind(key_index).c_str());
	}
	else
	{
		binds.change_default(binds.get_name(key_index));
		gtk_entry_set_text(GTK_ENTRY(key_bind_entry), binds.get_bind(key_index).c_str());
	}
}

void restore_default_keys_clicked(GtkWidget *w, gpointer data)
{
	binds.set_defaults();
	
	if (key_index != -1)
		gtk_entry_set_text(GTK_ENTRY(key_bind_entry), binds.get_bind(key_index).c_str());
}

void load_keys_clicked(GtkWidget *w, gpointer data)
{
	string filename = file_browser("*.cfg", "/config/keys");

	if (filename != "")
	{
		Tokenizer tz;
		tz.open_file(filename, 0, 0);
		tz.get_token();
		binds.load(&tz);
	}
}

void save_keys_clicked(GtkWidget *w, gpointer data)
{
	string name = entry_box("Enter a name for the configuration:");

	if (name != "")
	{
		string path = "config/keys/";
		path += name;
		path += ".cfg";
		FILE *fp = fopen(path.c_str(), "wt");
		binds.save(fp);
		fclose(fp);
	}
}

gboolean keybind_entry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (key_set)
	{
		int n_entries = 0;
		guint *vals = 0;

		gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
											event->hardware_keycode,
											NULL,
											&vals,
											&n_entries);

		string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
		gtk_entry_set_text(GTK_ENTRY(key_bind_entry), key.c_str());
	}

	return true;
}

GtkWidget *setup_input_prefs()
{
	GtkWidget* hbox = gtk_hbox_new(true, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

	// Left side (keys list)
	GtkWidget* frame = gtk_frame_new("Keys");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(hbox), frame, true, true, 0);

	// Scrolled window
	GtkWidget* s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_container_set_border_width(GTK_CONTAINER(s_window), 4);
	gtk_container_add(GTK_CONTAINER(frame), s_window);

	// List
	GtkListStore* keys_store = gtk_list_store_new(1, G_TYPE_STRING);
	setup_keys_store(keys_store);

	GtkWidget* tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(keys_store));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Key", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), false);
	g_signal_connect(G_OBJECT(tree_view), "cursor-changed", G_CALLBACK(keys_list_changed), NULL);
	gtk_container_add(GTK_CONTAINER(s_window), tree_view);

	// Right side
	GtkWidget* vbox = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 0);

	// Key binding frame
	frame = gtk_frame_new("Binding");
	gtk_box_pack_start(GTK_BOX(vbox), frame, false, false, 0);
	GtkWidget* vbox2 = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	// Entry
	key_bind_entry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(key_bind_entry), false);
	gtk_entry_set_alignment(GTK_ENTRY(key_bind_entry), 0.5f);
	gtk_widget_set_size_request(key_bind_entry, 32, -1);
	gtk_widget_set_sensitive(key_bind_entry, false);
	g_signal_connect(G_OBJECT(key_bind_entry), "key_press_event", G_CALLBACK(keybind_entry_keypress), NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), key_bind_entry, false, false, false);

	// Set/Accept, Default/Cancel buttons
	GtkWidget *hbox2 = gtk_hbox_new(true, 4);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, false, false, 0);

	key_set_accept = gtk_button_new_with_label("Set");
	g_signal_connect(G_OBJECT(key_set_accept), "clicked", G_CALLBACK(key_set_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(hbox2), key_set_accept, true, true, 0);

	key_default_cancel = gtk_button_new_with_label("Default");
	g_signal_connect(G_OBJECT(key_default_cancel), "clicked", G_CALLBACK(key_default_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(hbox2), key_default_cancel, true, true, 0);

	// Config frame
	frame = gtk_frame_new("Config");
	gtk_box_pack_start(GTK_BOX(vbox), frame, false, false, 0);
	vbox2 = gtk_vbox_new(false, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	// Defaults button
	GtkWidget* button = gtk_button_new_with_label("Restore Defaults");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(restore_default_keys_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), button, false, false, 0);

	// Load config button
	button = gtk_button_new_with_label("Load Key Config");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(load_keys_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), button, false, false, 0);

	// Save config button
	button = gtk_button_new_with_label("Save Key Config");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(save_keys_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), button, false, false, 0);

	return hbox;
}
