
#include "main.h"
#include "map.h"
#include "tex_box.h"
#include "tex_browser.h"
#include "checks.h"

tex_box_t *sedit_tbox_floor = NULL;
tex_box_t *sedit_tbox_ceil = NULL;

struct sedit_data_t
{
	short	f_height;
	bool	f_height_consistent;
	short	c_height;
	bool	c_height_consistent;
	string	f_tex;
	bool	f_tex_consistent;
	string	c_tex;
	bool	c_tex_consistent;
	short	light;
	bool	light_consistent;
	short	special;
	bool	special_consistent;
	short	tag;
	bool	tag_consistent;
};

sedit_data_t sedit_data;

extern Map map;
extern GtkWidget *editor_window;
extern vector<int> selected_items;
extern int hilight_item;

void sedit_fheight_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.f_height_consistent = true;
		sedit_data.f_height = atoi(text.c_str());
	}
	else
		sedit_data.f_height_consistent = false;
}

void sedit_cheight_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.c_height_consistent = true;
		sedit_data.c_height = atoi(text.c_str());
	}
	else
		sedit_data.c_height_consistent = false;
}

void sedit_ftex_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.f_tex_consistent = true;
		sedit_data.f_tex = text.c_str();
		sedit_tbox_floor->change_texture(text, 2, 2.0f);
	}
	else
		sedit_data.f_tex_consistent = false;
}

void sedit_ctex_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.c_tex_consistent = true;
		sedit_data.c_tex = text.c_str();
		sedit_tbox_ceil->change_texture(text, 2, 2.0f);
	}
	else
		sedit_data.c_tex_consistent = false;
}

void sedit_special_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.special_consistent = true;
		sedit_data.special = atoi(text.c_str());
	}
	else
		sedit_data.special_consistent = false;
}

void sedit_tag_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.tag_consistent = true;
		sedit_data.tag = atoi(text.c_str());
	}
	else
		sedit_data.tag_consistent = false;
}

void sedit_light_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(editable);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		sedit_data.light_consistent = true;
		sedit_data.light = atoi(text.c_str());
	}
	else
		sedit_data.light_consistent = false;
}

gboolean sedit_ftex_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == 1)
	{
		string tex = open_texture_browser(false, true, false, sedit_data.f_tex);
		gtk_entry_set_text(GTK_ENTRY(data), tex.c_str());
	}

	return false;
}

gboolean sedit_ctex_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == 1)
	{
		string tex = open_texture_browser(false, true, false, sedit_data.c_tex);
		gtk_entry_set_text(GTK_ENTRY(data), tex.c_str());
	}

	return false;
}

void sedit_find_tag_clicked(GtkWidget *widget, gpointer data)
{
	int tag = get_free_tag();
	gtk_entry_set_text(GTK_ENTRY(data), parse_string("%d", tag).c_str());
}

GtkWidget* setup_sector_edit()
{
	sector_t* sector;
	if (selected_items.size() == 0)
		sector = map.sectors[hilight_item];
	else
		sector = map.sectors[selected_items[0]];

	// Init data
	sedit_data.f_height = sector->f_height;
	sedit_data.c_height = sector->c_height;
	sedit_data.f_tex = sector->f_tex;
	sedit_data.c_tex = sector->c_tex;
	sedit_data.light = sector->light;
	sedit_data.special = sector->special;
	sedit_data.tag = sector->tag;
	sedit_data.f_height_consistent = true;
	sedit_data.c_height_consistent = true;
	sedit_data.f_tex_consistent = true;
	sedit_data.c_tex_consistent = true;
	sedit_data.light_consistent = true;
	sedit_data.special_consistent = true;
	sedit_data.tag_consistent = true;

	// Check for inconsistency
	if (selected_items.size() > 1)
	{
		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.sectors[selected_items[a]]->f_height != sedit_data.f_height)
				sedit_data.f_height_consistent = false;

			if (map.sectors[selected_items[a]]->c_height != sedit_data.c_height)
				sedit_data.c_height_consistent = false;

			if (map.sectors[selected_items[a]]->f_tex != sedit_data.f_tex)
				sedit_data.f_tex_consistent = false;

			if (map.sectors[selected_items[a]]->c_tex != sedit_data.c_tex)
				sedit_data.c_tex_consistent = false;

			if (map.sectors[selected_items[a]]->light != sedit_data.light)
				sedit_data.light_consistent = false;

			if (map.sectors[selected_items[a]]->special != sedit_data.special)
				sedit_data.special_consistent = false;

			if (map.sectors[selected_items[a]]->tag != sedit_data.tag)
				sedit_data.tag_consistent = false;
		}
	}

	if (sedit_tbox_floor)
	{
		delete sedit_tbox_floor;
		sedit_tbox_floor = NULL;
	}

	if (sedit_tbox_ceil)
	{
		delete sedit_tbox_ceil;
		sedit_tbox_ceil = NULL;
	}

	GtkWidget* main_vbox = gtk_vbox_new(false, 0);
	GtkWidget* hbox = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(main_vbox), hbox, true, true, 0);

	// FLOOR FRAME
	GtkWidget *frame = gtk_frame_new("Floor");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(hbox), frame, true, true, 0);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	GtkWidget *entry = gtk_entry_new();

	// Texture box
	sedit_tbox_floor = new tex_box_t(sedit_data.f_tex, 2, 2.0f, rgba_t(180, 180, 180, 255, 0));
	sedit_tbox_floor->set_size(128, 128);
	frame = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, true);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(frame), sedit_tbox_floor->widget);
	gtk_widget_set_events(sedit_tbox_floor->widget, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(sedit_tbox_floor->widget), "button_press_event", G_CALLBACK(sedit_ftex_clicked), entry);
	gtk_box_pack_start(GTK_BOX(vbox), frame, true, true, 0);

	// Entry
	gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_ftex_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false , 4);
	if (sedit_data.f_tex_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), sedit_data.f_tex.c_str());

	// Height
	GtkWidget *hbox2 = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, false, false, 4);
	GtkWidget *label = gtk_label_new("Height: ");
	gtk_box_pack_start(GTK_BOX(hbox2), label, false, false, 0);

	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_fheight_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox2), entry, true, true, 0);
	if (sedit_data.f_height_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sedit_data.f_height).c_str());


	// CEILING FRAME
	frame = gtk_frame_new("Ceiling");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(hbox), frame, true, true, 0);
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	entry = gtk_entry_new();

	// Texture box
	sedit_tbox_ceil = new tex_box_t(sedit_data.c_tex, 2, 2.0f, rgba_t(180, 180, 180, 255, 0));
	sedit_tbox_ceil->set_size(128, 128);
	frame = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, true);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_set_events(sedit_tbox_ceil->widget, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(sedit_tbox_ceil->widget), "button_press_event", G_CALLBACK(sedit_ctex_clicked), entry);
	gtk_container_add(GTK_CONTAINER(frame), sedit_tbox_ceil->widget);
	gtk_box_pack_start(GTK_BOX(vbox), frame, true, true, 0);

	// Entry
	gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_ctex_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false , 4);
	if (sedit_data.c_tex_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), sedit_data.c_tex.c_str());

	// Height
	hbox2 = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, false, false, 4);
	label = gtk_label_new("Height: ");
	gtk_box_pack_start(GTK_BOX(hbox2), label, false, false, 0);

	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_cheight_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox2), entry, true, true, 0);
	if (sedit_data.c_height_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sedit_data.c_height).c_str());


	// PROPERTIES FRAME
	frame = gtk_frame_new("Properties");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, false, false, 0);
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	// Special
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("Special: ");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_special_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);
	if (sedit_data.special_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sedit_data.special).c_str());

	GtkWidget *button = gtk_button_new_with_label("Change");
	gtk_widget_set_size_request(button, 96, -1);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	// Tag
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 4);
	label = gtk_label_new("Tag: ");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_tag_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);
	if (sedit_data.tag_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sedit_data.tag).c_str());

	button = gtk_button_new_with_label("Find Unused");
	gtk_widget_set_size_request(button, 96, -1);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	// Light
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("Light Level: ");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 48, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sedit_light_entry_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 0);
	if (sedit_data.light_consistent)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sedit_data.light).c_str());

	return main_vbox;
}

void apply_sector_edit()
{
	// Get sectors to modify
	vector<sector_t*> edit_sectors;
	if (selected_items.size() == 0)
		edit_sectors.push_back(map.sectors[hilight_item]);
	else
	{
		for (int a = 0; a < selected_items.size(); a++)
			edit_sectors.push_back(map.sectors[selected_items[a]]);
	}
	
	for (int a = 0; a < edit_sectors.size(); a++)
	{
		if (sedit_data.f_height_consistent)
			edit_sectors[a]->f_height = sedit_data.f_height;

		if (sedit_data.c_height_consistent)
			edit_sectors[a]->c_height = sedit_data.c_height;

		if (sedit_data.f_tex_consistent)
			edit_sectors[a]->f_tex = sedit_data.f_tex;

		if (sedit_data.c_tex_consistent)
			edit_sectors[a]->c_tex = sedit_data.c_tex;

		if (sedit_data.special_consistent)
			edit_sectors[a]->special = sedit_data.special;

		if (sedit_data.tag_consistent)
			edit_sectors[a]->tag = sedit_data.tag;

		if (sedit_data.light_consistent)
			edit_sectors[a]->light = sedit_data.light;
	}
}

void open_sector_edit()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Sector",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_sector_edit());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
		apply_sector_edit();

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
