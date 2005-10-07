#include "main.h"
#include "map.h"
#include "misc.h"
#include "checks.h"
#include "editor_window.h"
#include "tex_box.h"

struct side_data
{
	bool xoff_consistent;
	bool yoff_consistent;
	int x_offset;
	int y_offset;
	string tex_middle;
	string tex_upper;
	string tex_lower;
	tex_box_t *tbox_upper;
	tex_box_t *tbox_middle;
	tex_box_t *tbox_lower;

	vector<GtkWidget*> widgets;

	side_data()
	{
		x_offset = y_offset = -1;
		tex_middle = tex_upper = tex_lower = "";
		xoff_consistent = yoff_consistent = true;
	}
};

side_data side1;
side_data side2;

//GtkWidget	*side1_edit = NULL;
//GtkWidget	*side2_edit = NULL;

extern Map map;
extern GtkWidget *editor_window;
extern vector<int> selected_items;
extern int hilight_item;

GtkWidget* setup_side_edit(int side)
{
	GtkWidget *widget = NULL;

	int line = -1;
	bool side_exists = false;
	side_data* sdat = NULL;

	// Setup data
	if (side == 1)
		sdat = &side1;
	else
		sdat = &side2;

	sdat->widgets.clear();
	sdat->xoff_consistent = true;
	sdat->yoff_consistent = true;

	if (selected_items.size() == 0)
	{
		if (map.l_getside(hilight_item, side))
		{
			sdat->x_offset = map.l_getside(hilight_item, side)->x_offset;
			sdat->y_offset = map.l_getside(hilight_item, side)->y_offset;
			sdat->tex_lower = map.l_getside(hilight_item, side)->tex_lower;
			sdat->tex_middle = map.l_getside(hilight_item, side)->tex_middle;
			sdat->tex_upper = map.l_getside(hilight_item, side)->tex_upper;
			line = hilight_item;
			side_exists = true;
		}
	}
	else
	{
		// Find the first selected line with an existing side
		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.l_getside(selected_items[a], side))
			{
				sdat->x_offset = map.l_getside(selected_items[a], side)->x_offset;
				sdat->y_offset = map.l_getside(selected_items[a], side)->y_offset;
				sdat->tex_lower = map.l_getside(selected_items[a], side)->tex_lower;
				sdat->tex_middle = map.l_getside(selected_items[a], side)->tex_middle;
				sdat->tex_upper = map.l_getside(selected_items[a], side)->tex_upper;
				side_exists = true;
				line = selected_items[a];
				break;
			}
		}

		// Check consistency
		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.l_getside(selected_items[a], side))
			{
				if (sdat->xoff_consistent && sdat->x_offset != map.l_getside(selected_items[a], side)->x_offset)
					sdat->xoff_consistent = false;

				if (sdat->yoff_consistent && sdat->y_offset != map.l_getside(selected_items[a], side)->y_offset)
					sdat->yoff_consistent = false;

				if (sdat->tex_middle != "" && sdat->tex_middle != map.l_getside(selected_items[a], side)->tex_middle)
					sdat->tex_middle = "";

				if (sdat->tex_upper != "" && sdat->tex_upper != map.l_getside(selected_items[a], side)->tex_upper)
					sdat->tex_upper = "";

				if (sdat->tex_lower != "" && sdat->tex_lower != map.l_getside(selected_items[a], side)->tex_lower)
					sdat->tex_lower = "";
			}
		}
	}

	widget = gtk_vbox_new(false, 0);

	// Textures frame
	GtkWidget *frame = gtk_frame_new("Textures");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	GtkWidget *hbox = gtk_hbox_new(true, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	delete sdat->tbox_upper;
	delete sdat->tbox_middle;
	delete sdat->tbox_lower;

	// Upper tex
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	sdat->tbox_upper = new tex_box_t("", 1, 2.0f, rgba_t(180, 180, 180, 255, 0));
	sdat->tbox_upper->set_size(96, 96);
	gtk_box_pack_start(GTK_BOX(vbox), sdat->tbox_upper->widget, true, true, 0);
	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 96, -1);
	sdat->widgets.push_back(entry);
	gtk_entry_set_text(GTK_ENTRY(entry), sdat->tex_upper.c_str());
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 0);

	// Middle tex
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	sdat->tbox_middle = new tex_box_t("", 1, 2.0f, rgba_t(180, 180, 180, 255, 0));
	sdat->tbox_middle->set_size(96, 96);
	gtk_box_pack_start(GTK_BOX(vbox), sdat->tbox_middle->widget, true, true, 0);
	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 96, -1);
	sdat->widgets.push_back(entry);
	gtk_entry_set_text(GTK_ENTRY(entry), sdat->tex_middle.c_str());
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 4);

	// Lower tex
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	sdat->tbox_lower = new tex_box_t("", 1, 2.0f, rgba_t(180, 180, 180, 255, 0));
	sdat->tbox_lower->set_size(96, 96);
	gtk_box_pack_start(GTK_BOX(vbox), sdat->tbox_lower->widget, true, true, 0);
	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 96, -1);
	sdat->widgets.push_back(entry);
	gtk_entry_set_text(GTK_ENTRY(entry), sdat->tex_lower.c_str());
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 4);

	if (line != -1)
	{
		if (map.l_needsuptex(line, side) || sdat->tex_upper != "-")
			sdat->tbox_upper->change_texture(sdat->tex_upper, 1, 2.0f, false);

		if (map.l_needsmidtex(line, side) || sdat->tex_middle != "-")
			sdat->tbox_middle->change_texture(sdat->tex_middle, 1, 2.0f, false);

		if (map.l_needslotex(line, side) || sdat->tex_lower != "-")
			sdat->tbox_lower->change_texture(sdat->tex_lower, 1, 2.0f, false);
	}

	gtk_box_pack_start(GTK_BOX(widget), frame, true, true, 0);

	
	// Offsets
	GtkWidget *hbox2 = gtk_hbox_new(true, 0);
	frame = gtk_frame_new("Offsets");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	// X
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("X:"), false, false, 0);
	entry = gtk_entry_new();
	sdat->widgets.push_back(entry);
	gtk_widget_set_size_request(entry, 32, -1);
	if (sdat->x_offset != -1)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sdat->x_offset).c_str());
	gtk_box_pack_start(GTK_BOX(hbox), entry, true, true, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

	// Y
	hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Y:"), false, false, 0);
	entry = gtk_entry_new();
	sdat->widgets.push_back(entry);
	gtk_widget_set_size_request(entry, 32, -1);
	if (sdat->y_offset != -1)
		gtk_entry_set_text(GTK_ENTRY(entry), parse_string("%d", sdat->y_offset).c_str());
	gtk_box_pack_start(GTK_BOX(hbox), entry, true, true, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 4);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_box_pack_start(GTK_BOX(hbox2), frame, true, true, 0);

	// Side (create/delete)
	frame = gtk_frame_new("Side");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

	GtkWidget *button_create = gtk_button_new_with_label("Create");
	gtk_box_pack_start(GTK_BOX(vbox), button_create, false, false, 0);

	GtkWidget *button_delete = gtk_button_new_with_label("Delete");
	sdat->widgets.push_back(button_delete);
	gtk_box_pack_start(GTK_BOX(vbox), button_delete, false, false, 0);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_box_pack_start(GTK_BOX(hbox2), frame, true, true, 0);


	gtk_box_pack_start(GTK_BOX(widget), hbox2, false, false, 0);


	if (!side_exists)
	{
		for (int a = 0; a < sdat->widgets.size(); a++)
			gtk_widget_set_sensitive(sdat->widgets[a], false);

		gtk_widget_set_sensitive(button_create, true);
	}
	else
		gtk_widget_set_sensitive(button_create, false);


	return widget;
}
