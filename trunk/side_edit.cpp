#include "main.h"
#include "map.h"
#include "misc.h"
#include "checks.h"
#include "editor_window.h"
#include "tex_box.h"
#include "tex_browser.h"

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

static gboolean tex_box_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkEntry *entry = (GtkEntry*)data;
	string tex;

	if (event->button == 1)
		tex = open_texture_browser(true, false, false, gtk_entry_get_text(entry));

	gtk_entry_set_text(entry, tex.c_str());

	return true;
}

void tex_upper_changed(GtkEditable *editable, gpointer data)
{
	side_data *sdat = (side_data*)data;
	GtkEntry *entry = (GtkEntry*)editable;
	string tex = str_upper(gtk_entry_get_text(entry));
	gtk_entry_set_text(entry, tex.c_str());
	sdat->tex_upper = tex;

	if (tex == "-")
		tex = "";

	sdat->tbox_upper->change_texture(tex, sdat->tbox_upper->textype, sdat->tbox_upper->scale);
}

void tex_middle_changed(GtkEditable *editable, gpointer data)
{
	side_data *sdat = (side_data*)data;
	GtkEntry *entry = (GtkEntry*)editable;
	string tex = str_upper(gtk_entry_get_text(entry));
	gtk_entry_set_text(entry, tex.c_str());
	sdat->tex_middle = tex;

	if (tex == "-")
		tex = "";

	sdat->tbox_middle->change_texture(tex, sdat->tbox_middle->textype, sdat->tbox_middle->scale);
}

void tex_lower_changed(GtkEditable *editable, gpointer data)
{
	side_data *sdat = (side_data*)data;
	GtkEntry *entry = (GtkEntry*)editable;
	string tex = str_upper(gtk_entry_get_text(entry));
	gtk_entry_set_text(entry, tex.c_str());
	sdat->tex_lower = tex;

	if (tex == "-")
		tex = "";

	sdat->tbox_lower->change_texture(tex, sdat->tbox_lower->textype, sdat->tbox_lower->scale);
}

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
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(tex_upper_changed), sdat);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 0);
	gtk_widget_set_events(sdat->tbox_upper->widget, GDK_BUTTON_PRESS_MASK);
	if (side_exists)
		g_signal_connect(G_OBJECT(sdat->tbox_upper->widget), "button_press_event", G_CALLBACK(tex_box_clicked), entry);

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
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(tex_middle_changed), sdat);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 4);
	gtk_widget_set_events(sdat->tbox_middle->widget, GDK_BUTTON_PRESS_MASK);
	if (side_exists)
		g_signal_connect(G_OBJECT(sdat->tbox_middle->widget), "button_press_event", G_CALLBACK(tex_box_clicked), entry);

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
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(tex_lower_changed), sdat);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, true, 4);
	gtk_widget_set_events(sdat->tbox_lower->widget, GDK_BUTTON_PRESS_MASK);
	if (side_exists)
		g_signal_connect(G_OBJECT(sdat->tbox_lower->widget), "button_press_event", G_CALLBACK(tex_box_clicked), entry);

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

void apply_side_edit()
{
	vector<sidedef_t*> side1s;
	vector<sidedef_t*> side2s;

	// Get sidedefs that are to be modified
	if (selected_items.size() == 0)
	{
		if (map.l_getside(hilight_item, 1))
			vector_add_nodup(side1s, map.l_getside(hilight_item, 1));
		if (map.l_getside(hilight_item, 2))
			vector_add_nodup(side2s, map.l_getside(hilight_item, 2));
	}
	else
	{
		for (int a = 0; a < selected_items.size(); a++)
		{
			if (map.l_getside(selected_items[a], 1))
				vector_add_nodup(side1s, map.l_getside(selected_items[a], 1));
			if (map.l_getside(selected_items[a], 2))
				vector_add_nodup(side2s, map.l_getside(selected_items[a], 2));
		}
	}

	// Upper texture
	if (side1.tex_upper != "")
	{
		for (int a = 0; a < side1s.size(); a++)
			side1s[a]->tex_upper = side1.tex_upper;
	}

	if (side2.tex_upper != "")
	{
		for (int a = 0; a < side2s.size(); a++)
			side2s[a]->tex_upper = side2.tex_upper;
	}

	// Middle texture
	if (side1.tex_middle != "")
	{
		for (int a = 0; a < side1s.size(); a++)
			side1s[a]->tex_middle = side1.tex_middle;
	}

	if (side2.tex_middle != "")
	{
		for (int a = 0; a < side2s.size(); a++)
			side2s[a]->tex_middle = side2.tex_middle;
	}

	// Lower texture
	if (side1.tex_lower != "")
	{
		for (int a = 0; a < side1s.size(); a++)
			side1s[a]->tex_lower = side1.tex_lower;
	}

	if (side2.tex_lower != "")
	{
		for (int a = 0; a < side2s.size(); a++)
			side2s[a]->tex_lower = side2.tex_lower;
	}

	// X Offset
	if (side1.xoff_consistent)
	{
		for (int a = 0; a < side1s.size(); a++)
			side1s[a]->x_offset = side1.x_offset;
	}

	if (side2.xoff_consistent)
	{
		for (int a = 0; a < side2s.size(); a++)
			side2s[a]->x_offset = side2.x_offset;
	}

	// Y Offset
	if (side1.yoff_consistent)
	{
		for (int a = 0; a < side1s.size(); a++)
			side1s[a]->y_offset = side1.y_offset;
	}

	if (side2.yoff_consistent)
	{
		for (int a = 0; a < side2s.size(); a++)
			side2s[a]->y_offset = side2.y_offset;
	}
}