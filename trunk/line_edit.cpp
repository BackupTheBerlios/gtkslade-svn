
#include "main.h"
#include "map.h"
#include "checks.h"
#include "editor_window.h"
#include "line_edit.h"
#include "special_select.h"
#include "action_special.h"
#include "args_edit.h"

linedef_t line;

vector<int> set_flags;
vector<int> unset_flags;
int			special = -1;
int			tag = -1;

BYTE	args[5];
bool	arg_consistent[5];

extern Map map;
extern GtkWidget *editor_window;
extern vector<int> selected_items;
extern int hilight_item;

void special_changed(GtkWidget *w, gpointer data)
{
	string text = gtk_entry_get_text(GTK_ENTRY(w));

	if (text != "")
		special = atoi(text.c_str());
	else
		special = -1;
}

void tag_changed(GtkWidget *w, gpointer data)
{
	string text = gtk_entry_get_text(GTK_ENTRY(w));

	if (text != "")
		tag = atoi(text.c_str());
	else
		tag = -1;
}

void find_unused_click(GtkWidget *w, gpointer data)
{
	GtkWidget *entry = GTK_WIDGET(data);
	gtk_entry_set_text(GTK_ENTRY(data), parse_string("%d", get_free_tag()).c_str());
}

void change_special_click(GtkWidget *w, gpointer data)
{
	GtkEntry *entry = (GtkEntry*)data;
	int spec = atoi(gtk_entry_get_text(entry));

	int newspec = open_special_select_dialog(spec);
	gtk_entry_set_text(entry, parse_string("%d", newspec).c_str());
}

void edit_line_args_click(GtkWidget *w, gpointer data)
{
	if (special == -1)
		return;

	action_special_t* aspec = get_special(special);
	open_args_edit(args, aspec->args, aspec->arg_types, arg_consistent);
}

void flag_click(GtkWidget *w, gpointer data)
{
	int flag = (int)data;

	if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(w)))
		gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(w), false);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{
		vector_add_nodup(set_flags, flag);

		if (vector_exists(unset_flags, flag))
			unset_flags.erase(find(unset_flags.begin(), unset_flags.end(), flag));
	}
	else
	{
		vector_add_nodup(unset_flags, flag);
		
		if (vector_exists(set_flags, flag))
			set_flags.erase(find(set_flags.begin(), set_flags.end(), flag));
	}
}

GtkWidget* setup_flag_checkbox(string name, int type, int flag)
{
	GtkWidget *cbox = gtk_check_button_new_with_label(name.c_str());

	if (selected_items.size() == 0 && hilight_item != -1)
	{
		if (type == 1)
		{
			if (map.lines[hilight_item]->flags & flag)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);
		}

		if (type == 2)
		{
			if (map.things[hilight_item]->flags & flag)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);
		}
	}
	else
	{
		int count = 0;

		for (int a = 0; a < selected_items.size(); a++)
		{
			if (type == 1)
			{
				if (map.lines[selected_items[a]]->flags & flag)
					count++;
			}

			if (type == 2)
			{
				if (map.things[selected_items[a]]->flags & flag)
					count++;
			}
		}

		if (count > 0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), true);

		if (count != selected_items.size())
			gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(cbox), true);
	}

	g_signal_connect(G_OBJECT(cbox), "clicked", G_CALLBACK(flag_click), (gpointer)flag);

	return cbox;
}

GtkWidget* setup_line_edit_props()
{
	memset(arg_consistent, 1, 5);

	if (selected_items.size() == 0 && hilight_item != -1)
	{
		special = map.lines[hilight_item]->type;
		tag = map.lines[hilight_item]->sector_tag;
		memcpy(args, map.lines[hilight_item]->args, 5);
	}
	else
	{
		special = map.lines[selected_items[0]]->type;
		tag = map.lines[selected_items[0]]->sector_tag;
		memcpy(args, map.lines[selected_items[0]]->args, 5);

		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.lines[selected_items[a]]->type != special)
				special = -1;

			if (map.lines[selected_items[a]]->sector_tag != tag)
				tag = -1;

			for (int b = 0; b < 5; b++)
			{
				if (map.lines[selected_items[a]]->args[b] != args[b])
					arg_consistent[b] = false;
			}
		}
	}

	GtkWidget *props_page = gtk_vbox_new(false, 0);

	// Flags
	GtkWidget *frame = gtk_frame_new("Flags");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(props_page), frame, true, true, 0);

	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Impassible", 1, LINE_IMPASSIBLE), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Monsters", 1, LINE_BLOCKMONSTERS), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Two Sided", 1, LINE_TWOSIDED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Upper Texture Unpegged", 1, LINE_UPPERUNPEGGED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Lower Texture Unpegged", 1, LINE_LOWERUNPEGGED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Secret (Shown as 1S on Map)", 1, LINE_SECRET), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

	vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Sound", 1, LINE_BLOCKSOUND), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Doesn't Show On Map", 1, LINE_NOTONMAP), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Starts On Map", 1, LINE_STARTONMAP), false, false, 0);
	if (map.hexen)
	{
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Repeatable Special", 1, LINE_REPEATABLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Monsters Can Activate", 1, LINE_MONSTERCANACT), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Everything", 1, LINE_BLOCKALL), false, false, 0);
	}
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, false, 0);

	gtk_container_add(GTK_CONTAINER(frame), hbox);



	// Properties
	frame = gtk_frame_new("Properties");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(props_page), frame, true, true, 0);

	vbox = gtk_vbox_new(false, 0);

	// Special
	hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

	GtkWidget *label = gtk_label_new("Special:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

	GtkWidget *entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(special_changed), NULL);
	gtk_widget_set_size_request(entry, 32, -1);
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);

	GtkWidget *button = gtk_button_new_with_label("Change");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(change_special_click), entry);
	gtk_widget_set_size_request(button, 96, -1);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	if (map.hexen)
	{
		button = gtk_button_new_with_label("Edit Args");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(edit_line_args_click), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 4);
	}

	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

	if (selected_items.size() == 0 && hilight_item != -1)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", map.lines[hilight_item]->type).c_str());

	if (!map.hexen)
	{
		// Sector Tag
		hbox = gtk_hbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

		GtkWidget *label = gtk_label_new("Sector Tag:");
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

		GtkWidget *entry = gtk_entry_new();
		g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(tag_changed), NULL);
		gtk_widget_set_size_request(entry, 32, -1);
		gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);

		GtkWidget *button = gtk_button_new_with_label("Find Unused");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(find_unused_click), entry);
		gtk_widget_set_size_request(button, 96, -1);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

		gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

		if (selected_items.size() == 0 && hilight_item != -1)
			gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", map.lines[hilight_item]->sector_tag).c_str());
	}

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return props_page;
}

GtkWidget* setup_line_edit()
{
	GtkWidget *tabs = gtk_notebook_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(tabs),
								setup_line_edit_props(),
								gtk_label_new("Properties"));

	//GtkWidget *temp = gtk_label_new("Front Side");
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), setup_side_edit(1), gtk_label_new("Side 1"));

	//temp = gtk_label_new("Back Side");
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), setup_side_edit(2), gtk_label_new("Side 2"));

	return tabs;
}

void apply_line_edit()
{
	// Set changed flags
	for (int a = 0; a < set_flags.size(); a++)
	{
		if (selected_items.size() == 0 && hilight_item != -1)
			map.lines[hilight_item]->set_flag(set_flags[a]);
		else if (selected_items.size() > 0)
		{
			for (int i = 0; i < selected_items.size(); i++)
				map.lines[selected_items[i]]->set_flag(set_flags[a]);
		}
	}

	// Unset changed flags
	for (int a = 0; a < unset_flags.size(); a++)
	{
		if (selected_items.size() == 0 && hilight_item != -1)
			map.lines[hilight_item]->clear_flag(unset_flags[a]);
		else if (selected_items.size() > 0)
		{
			for (int i = 0; i < selected_items.size(); i++)
				map.lines[selected_items[i]]->clear_flag(unset_flags[a]);
		}
	}

	// Special
	if (special != -1)
	{
		if (selected_items.size() == 0 && hilight_item != -1)
			map.lines[hilight_item]->type = special;
		else if (selected_items.size() > 0)
		{
			for (int i = 0; i < selected_items.size(); i++)
				map.lines[selected_items[i]]->type = special;
		}
	}

	// Sector tag
	if (tag != -1)
	{
		if (selected_items.size() == 0 && hilight_item != -1)
			map.lines[hilight_item]->sector_tag = tag;
		else if (selected_items.size() > 0)
		{
			for (int i = 0; i < selected_items.size(); i++)
				map.lines[selected_items[i]]->sector_tag = tag;
		}
	}

	// Args
	for (int a = 0; a < 5; a++)
	{
		if (arg_consistent[a])
		{
			if (selected_items.size() == 0 && hilight_item != -1)
				map.lines[hilight_item]->args[a] = args[a];
			else if (selected_items.size() > 0)
			{
				for (int i = 0; i < selected_items.size(); i++)
					map.lines[selected_items[i]]->args[a] = args[a];
			}
		}
	}
}

void open_line_edit()
{
	set_flags.clear();
	unset_flags.clear();
	special = -1;
	tag = -1;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Line",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_line_edit());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		apply_line_edit();
		apply_side_edit();
		force_map_redraw(true, false);
	}

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
