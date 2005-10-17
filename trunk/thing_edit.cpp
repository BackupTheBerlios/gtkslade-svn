
#include "main.h"
#include "map.h"
#include "line_edit.h"
#include "thing_edit.h"
#include "tex_box.h"
#include "misc.h"
#include "editor_window.h"
#include "tex_browser.h"
#include "special_select.h"
#include "checks.h"

tex_box_t *thing_tbox = NULL;

struct tedit_data_t
{
	bool angle_consistent;
	int angle;
	bool type_consistent;
	int type;
	bool tid_consistent;
	int tid;
	bool special_consistent;
	int special;

	GtkWidget* entry_angle;
	GtkWidget* entry_type;
	GtkWidget* label_type;
	GtkWidget* entry_tid;
	GtkWidget* entry_special;
};

tedit_data_t tedit_data;

extern GtkWidget *editor_window;
extern Map map;
extern vector<int> set_flags;
extern vector<int> unset_flags;
extern vector<int> selected_items;
extern int hilight_item;

void tedit_change_type_clicked(GtkWidget *widget, gpointer data)
{
	int type = open_ttype_select_dialog(tedit_data.type);
	gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_type), parse_string("%d", type).c_str());
}

void tedit_browse_type_clicked(GtkWidget *widget, gpointer data)
{
	string name = open_texture_browser(false, false, true, thing_tbox->texture);
	int type = get_thing_type_from_name(name)->type;
	gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_type), parse_string("%d", type).c_str());
}

void tedit_change_special_clicked(GtkWidget *widget, gpointer data)
{
	int special = open_special_select_dialog(tedit_data.special);
	gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_special), parse_string("%d", special).c_str());
}

void tedit_find_tid_clicked(GtkWidget *widget, gpointer data)
{
	int tid = get_free_tid();
	gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_tid), parse_string("%d", tid).c_str());
}

void tedit_type_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry* entry = (GtkEntry*)editable;
	string text = gtk_entry_get_text(entry);

	if (text == "")
	{
		tedit_data.type_consistent = false;

		thing_tbox->change_texture("", 3);
		gtk_label_set_text(GTK_LABEL(tedit_data.label_type), "");
	}
	else
	{
		tedit_data.type = atoi(text.c_str());
		tedit_data.type_consistent = true;

		thing_type_t* ttype = get_thing_type(tedit_data.type);
		thing_tbox->change_texture(ttype->spritename, 3, 2.0f, true);
		gtk_label_set_text(GTK_LABEL(tedit_data.label_type), ttype->name.c_str());
	}
}

void tedit_angle_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry* entry = (GtkEntry*)editable;
	string text = gtk_entry_get_text(entry);

	if (text == "")
		tedit_data.angle_consistent = false;
	else
	{
		tedit_data.angle = atoi(text.c_str());
		tedit_data.angle_consistent = true;
	}
}

void tedit_tid_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry* entry = (GtkEntry*)editable;
	string text = gtk_entry_get_text(entry);

	if (text == "")
		tedit_data.tid_consistent = false;
	else
	{
		tedit_data.tid = atoi(text.c_str());
		tedit_data.tid_consistent = true;
	}
}

void tedit_special_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry* entry = (GtkEntry*)editable;
	string text = gtk_entry_get_text(entry);

	if (text == "")
		tedit_data.special_consistent = false;
	else
	{
		tedit_data.special = atoi(text.c_str());
		tedit_data.special_consistent = true;
	}
}

void angle_button_toggled(GtkToggleButton *button, gpointer data)
{
	int angle = (int)data;
	tedit_data.angle = angle;
	gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_angle), parse_string("%d", tedit_data.angle).c_str());
}

GtkWidget* setup_angle_button(GtkRadioButton* group, int angle)
{
	GtkWidget *button;

	if (group)
		button = gtk_radio_button_new_from_widget(group);
	else
		button = gtk_radio_button_new(NULL);

	gtk_button_set_alignment(GTK_BUTTON(button), 0.5, 0.5);
	g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(angle_button_toggled), (gpointer)angle);

	if (angle == tedit_data.angle && tedit_data.angle_consistent)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), false);

	return button;
}

GtkWidget* setup_thing_edit()
{
	// Get thing properties
	tedit_data.angle_consistent = true;
	tedit_data.type_consistent = true;
	tedit_data.tid_consistent = true;
	tedit_data.special_consistent = true;

	if (selected_items.size() == 0)
	{
		tedit_data.angle = map.things[hilight_item]->angle;
		tedit_data.type = map.things[hilight_item]->type;
		tedit_data.tid = map.things[hilight_item]->tid;
		tedit_data.special = map.things[hilight_item]->special;
	}
	else
	{
		tedit_data.angle = map.things[selected_items[0]]->angle;
		tedit_data.type = map.things[selected_items[0]]->type;
		tedit_data.tid = map.things[selected_items[0]]->tid;
		tedit_data.special = map.things[selected_items[0]]->special;

		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.things[selected_items[a]]->angle != tedit_data.angle)
				tedit_data.angle_consistent = false;

			if (map.things[selected_items[a]]->type != tedit_data.type)
				tedit_data.type_consistent = false;

			if (map.things[selected_items[a]]->tid != tedit_data.tid)
				tedit_data.tid_consistent = false;

			if (map.things[selected_items[a]]->special != tedit_data.special)
				tedit_data.special_consistent = false;
		}
	}

	if (thing_tbox)
	{
		delete thing_tbox;
		thing_tbox = NULL;
	}

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// FLAGS FRAME
	GtkWidget *frame = gtk_frame_new("Flags");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, true, true, 0);

	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Easy", 2, THING_EASY), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Medium", 2, THING_MEDIUM), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Hard", 2, THING_HARD), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Deaf (Only wakes on sight)", 2, THING_DEAF), false, false, 0);
	if (map.hexen)
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Dormant", 2, THING_DORMANT), false, false, 0);
	else
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Multiplayer", 2, THING_MULTI), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

	if (map.hexen)
	{
		vbox = gtk_vbox_new(false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Fighter", 2, THING_FIGHTER), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Cleric", 2, THING_CLERIC), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Mage", 2, THING_MAGE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Single Player", 2, THING_SINGLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Cooperative", 2, THING_COOPERATIVE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

		vbox = gtk_vbox_new(false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Deathmatch", 2, THING_DEATHMATCH), false, false, 0);
	}

	if (map.zdoom)
	{
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Translucent", 2, THING_TRANSLUCENT), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Invisible", 2, THING_INVISIBLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Friendly", 2, THING_FRIENDLY), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Stands Still", 2, THING_STANDSTILL), false, false, 0);
	}

	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	hbox = gtk_hbox_new(false, 0);

	// TYPE FRAME
	frame = gtk_frame_new("Type");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	GtkWidget *table = gtk_table_new(4, 5, true);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_container_set_border_width(GTK_CONTAINER(table), 4);
	gtk_container_add(GTK_CONTAINER(frame), table);
	gtk_box_pack_start(GTK_BOX(hbox), frame, true, true, 0);

	// Texbox
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	thing_tbox = new tex_box_t("", 3, 2.0f, rgba_t(180, 180, 180, 255, 0));
	gtk_container_add(GTK_CONTAINER(frame), thing_tbox->widget);
	gtk_table_attach_defaults(GTK_TABLE(table), frame, 0, 3, 0, 3);

	// Entry
	tedit_data.entry_type = gtk_entry_new();
	gtk_widget_set_size_request(tedit_data.entry_type, 32, -1);
	g_signal_connect(G_OBJECT(tedit_data.entry_type), "changed", G_CALLBACK(tedit_type_entry_changed), NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), tedit_data.entry_type, 3, 5, 0, 1);

	// Change button
	GtkWidget *button = gtk_button_new_with_label("Change");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tedit_change_type_clicked), NULL);
	gtk_table_attach(GTK_TABLE(table), button, 3, 5, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

	// Browse button
	button = gtk_button_new_with_label("Browse");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tedit_browse_type_clicked), NULL);
	gtk_table_attach(GTK_TABLE(table), button, 3, 5, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);

	// Label
	tedit_data.label_type = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), tedit_data.label_type, 0, 5, 3, 4);

	if (tedit_data.type_consistent)
	{
		thing_type_t* ttype = get_thing_type(tedit_data.type);

		thing_tbox->change_texture(ttype->spritename, 3, 2.0f);
		gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_type), parse_string("%d", ttype->type).c_str());
		gtk_label_set_text(GTK_LABEL(tedit_data.label_type), ttype->name.c_str());
	}

	// ANGLE FRAME
	frame = gtk_frame_new("Angle");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(hbox), frame, false, false, 0);

	tedit_data.entry_angle = gtk_entry_new();
	g_signal_connect(G_OBJECT(tedit_data.entry_angle), "changed", G_CALLBACK(tedit_angle_entry_changed), NULL);

	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	frame = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, true);
	table = gtk_table_new(5, 5, true);
	gtk_container_add(GTK_CONTAINER(frame), table);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(vbox), frame, true, true, 0);

	GtkWidget* btn_angle_unknown = gtk_radio_button_new(NULL);

	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 90), 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 45), 3, 4, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 0), 4, 5, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 315), 3, 4, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 270), 2, 3, 4, 5);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 225), 1, 2, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 180), 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), setup_angle_button(GTK_RADIO_BUTTON(btn_angle_unknown), 135), 1, 2, 1, 2);

	gtk_widget_set_size_request(tedit_data.entry_angle, 32, -1);
	gtk_box_pack_start(GTK_BOX(vbox), tedit_data.entry_angle, false, false, 4);

	if (tedit_data.angle_consistent)
		gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_angle), parse_string("%d", tedit_data.angle).c_str());

	gtk_box_pack_start(GTK_BOX(main_vbox), hbox, true, true, 0);

	if (map.hexen)
	{
		GtkWidget* hbox2 = gtk_hbox_new(true, 0);

		// TID FRAME
		frame = gtk_frame_new("TID");
		gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
		gtk_box_pack_start(GTK_BOX(hbox2), frame, true, true, 0);
		hbox = gtk_hbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
		gtk_container_add(GTK_CONTAINER(frame), hbox);

		// Entry
		tedit_data.entry_tid = gtk_entry_new();
		gtk_widget_set_size_request(tedit_data.entry_tid, 32, -1);
		g_signal_connect(G_OBJECT(tedit_data.entry_tid), "changed", G_CALLBACK(tedit_tid_entry_changed), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), tedit_data.entry_tid, true, true, 0);

		// Button
		button = gtk_button_new_with_label("Find Unused");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tedit_find_tid_clicked), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 4);

		if (tedit_data.tid_consistent)
			gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_tid), parse_string("%d", tedit_data.tid).c_str());


		// SPECIAL FRAME
		frame = gtk_frame_new("Action Special");
		gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
		gtk_box_pack_start(GTK_BOX(hbox2), frame, true, true, 0);
		hbox = gtk_hbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
		gtk_container_add(GTK_CONTAINER(frame), hbox);

		// Entry
		tedit_data.entry_special = gtk_entry_new();
		gtk_widget_set_size_request(tedit_data.entry_special, 32, -1);
		g_signal_connect(G_OBJECT(tedit_data.entry_special), "changed", G_CALLBACK(tedit_special_entry_changed), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), tedit_data.entry_special, true, true, 0);

		// Button
		button = gtk_button_new_with_label("Change");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tedit_change_special_clicked), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 4);

		if (tedit_data.special_consistent)
			gtk_entry_set_text(GTK_ENTRY(tedit_data.entry_special), parse_string("%d", tedit_data.special).c_str());

		gtk_box_pack_start(GTK_BOX(main_vbox), hbox2, false, false, 0);
	}

	return main_vbox;
}

void apply_thing_edit()
{
	// Get things to be changed
	vector<thing_t*> edit_things;
	if (selected_items.size() == 0)
		edit_things.push_back(map.things[hilight_item]);
	else
	{
		for (int a = 0; a < selected_items.size(); a++)
			edit_things.push_back(map.things[selected_items[a]]);
	}

	// Set changed flags
	for (int a = 0; a < set_flags.size(); a++)
	{
		for (int a = 0; a < edit_things.size(); a++)
			edit_things[a]->set_flag(set_flags[a]);
	}

	// Unset changed flags
	for (int a = 0; a < unset_flags.size(); a++)
	{
		for (int a = 0; a < edit_things.size(); a++)
			edit_things[a]->clear_flag(unset_flags[a]);
	}

	// Type
	if (tedit_data.type_consistent)
	{
		for (int a = 0; a < edit_things.size(); a++)
		{
			edit_things[a]->type = tedit_data.type;
			edit_things[a]->ttype = get_thing_type(tedit_data.type);
		}
	}

	// Angle
	if (tedit_data.angle_consistent)
	{
		for (int a = 0; a < edit_things.size(); a++)
			edit_things[a]->angle = tedit_data.angle;
	}

	if (map.hexen)
	{
		// TID
		if (tedit_data.tid_consistent)
		{
			for (int a = 0; a < edit_things.size(); a++)
				edit_things[a]->tid = tedit_data.tid;
		}

		// Action Special
		if (tedit_data.special_consistent)
		{
			for (int a = 0; a < edit_things.size(); a++)
				edit_things[a]->special = tedit_data.special;
		}
	}
}

void open_thing_edit()
{
	set_flags.clear();
	unset_flags.clear();

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Thing",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_thing_edit());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	//gtk_window_set_resizable(GTK_WINDOW(dialog), false);
	//gtk_widget_set_size_request(dialog, 320, 320);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		apply_thing_edit();
		force_map_redraw(true, false);
	}

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
