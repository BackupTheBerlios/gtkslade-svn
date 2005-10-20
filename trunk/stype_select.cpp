
#include "main.h"
#include "map.h"

int current_stype = 0;

int stype_damage = 0;
bool stype_flags[3];

extern GtkWidget *editor_window;
extern Map map;

void stype_tree_view_changed(GtkTreeView *view, gpointer data)
{
	GtkTreeIter			iter;
	GtkTreeSelection	*selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	GtkTreeModel		*model = gtk_tree_view_get_model(view);

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		int index;
		gtk_tree_model_get(model, &iter, 1, &index, -1);

		if (index >= 0)
			current_stype = index;
	}
}

void stype_damage_combo_changed(GtkWidget *widget, gpointer data)
{
	int index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	int val = index * 32;

	if (map.hexen)
		val = val * 8;

	stype_damage = val;
}

void stype_flag_changed(GtkWidget *widget, gpointer data)
{
	int flag = (int)data;
	stype_flags[flag] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

GtkWidget* setup_stype_select(int type)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// Type frame
	GtkWidget *frame = gtk_frame_new("Type");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, true, true, 0);

	// Setup tree model
	GtkListStore *list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	populate_list_store_stypes(list_store);

	// Setup scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(s_window), 4);
	gtk_container_add(GTK_CONTAINER(frame), s_window);

	// Setup tree view
	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), false);
	g_signal_connect(G_OBJECT(tree_view), "cursor-changed", G_CALLBACK(stype_tree_view_changed), NULL);
	gtk_container_add(GTK_CONTAINER(s_window), tree_view);


	// Generic flags
	if (map.boom)
	{
		// Damage frame
		frame = gtk_frame_new("Damage");
		gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
		gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);

		GtkWidget *vbox = gtk_vbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
		gtk_container_add(GTK_CONTAINER(frame), vbox);

		GtkWidget *combo = gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "None");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "5 Health/Second");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "10 Health/Second");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "20 Health/Second");
		g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(stype_damage_combo_changed), NULL);

		gtk_box_pack_start(GTK_BOX(vbox), combo, true, true, 0);

		int mult = 0;
		if (map.hexen) mult = 8;
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

		if (type & (32 * mult) && type & (64 * mult))
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 3);
		else if (type & (32 * mult))
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 1);
		else if (type & (64 * mult))
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);

		// Flags frame
		frame = gtk_frame_new("Flags");
		gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
		gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);

		vbox = gtk_vbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
		gtk_container_add(GTK_CONTAINER(frame), vbox);

		GtkWidget *cbox = gtk_check_button_new_with_label("Secret Area");
		g_signal_connect(G_OBJECT(cbox), "toggled", G_CALLBACK(stype_flag_changed), (gpointer)0);
		gtk_box_pack_start(GTK_BOX(vbox), cbox, false, false, 0);
		if (type & (128 * mult))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);

		cbox = gtk_check_button_new_with_label("Friction Enabled");
		g_signal_connect(G_OBJECT(cbox), "toggled", G_CALLBACK(stype_flag_changed), (gpointer)1);
		gtk_box_pack_start(GTK_BOX(vbox), cbox, false, false, 0);
		if (type & (256 * mult))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);

		cbox = gtk_check_button_new_with_label("Wind Enabled");
		g_signal_connect(G_OBJECT(cbox), "toggled", G_CALLBACK(stype_flag_changed), (gpointer)2);
		gtk_box_pack_start(GTK_BOX(vbox), cbox, false, false, 0);
		if (type & (512 * mult))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);
	}

	return main_vbox;
}

int open_stype_select_dialog(int type)
{
	current_stype = type;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Sector Type",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_stype_select(type));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_set_size_request(dialog, 320, 360);
	gtk_widget_show_all(dialog);

	int ret = type;
	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		ret = current_stype;

		if (map.boom)
		{
			ret += stype_damage;
			
			if (stype_flags[0])
			{
				if (map.hexen)
					ret += 1024;
				else
					ret += 128;
			}

			if (stype_flags[1])
			{
				if (map.hexen)
					ret += 2048;
				else
					ret += 256;
			}

			if (stype_flags[2])
			{
				if (map.hexen)
					ret += 4096;
				else
					ret += 512;
			}
		}
	}

	gtk_widget_destroy(dialog);

	return ret;
}
