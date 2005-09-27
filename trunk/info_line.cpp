
#include "main.h"
#include "info_bar.h"
#include "map.h"
#include "misc.h"

GtkWidget *frame_main = NULL;
GtkWidget *frame_side1 = NULL;
GtkWidget *frame_side2 = NULL;

// Main frame
GtkWidget *label_length = NULL;
GtkWidget *label_side1 = NULL;
GtkWidget *label_side2 = NULL;
GtkWidget *label_special = NULL;
GtkWidget *label_tag = NULL;

extern Map map;

GtkWidget *get_line_info_bar()
{
	// Main frame
	frame_main = gtk_frame_new("No Line Hilighted");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_main)), "Sans Bold 10");
	setup_label(&label_length, "Length:");
	setup_label(&label_side1, "Front Side:");
	setup_label(&label_side2, "Back Side:");
	setup_label(&label_special, "Special:");
	setup_label(&label_tag, "Sector Tag:");

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_container_add(GTK_CONTAINER(frame_main), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), label_length, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_side1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_side2, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_special, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_tag, false, false, 0);

	// Front Side Frame
	frame_side1 = gtk_frame_new("No Front Side");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");
	gtk_widget_set_size_request(frame_side1, 200, 0);

	// Back Side Frame
	frame_side2 = gtk_frame_new("No Back Side");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");
	gtk_widget_set_size_request(frame_side2, 200, 0);


	// Stuff
	GtkWidget *main_hbox = gtk_hbox_new(false, 0);
	//gtk_container_set_border_width(GTK_CONTAINER(main_hbox), 4);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_main, true, true, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_side1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_side2, false, false, 0);

	return main_hbox;
}

void update_line_info_bar(int line)
{
	if (line == -1)
	{
		gtk_frame_set_label(GTK_FRAME(frame_main), "No Line Hilighted");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_main)), "Sans Bold 10");
		gtk_frame_set_label(GTK_FRAME(frame_side1), "No Front Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");
		gtk_frame_set_label(GTK_FRAME(frame_side2), "No Back Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");

		gtk_label_set_text(GTK_LABEL(label_length), "Length:");
		gtk_label_set_text(GTK_LABEL(label_side1), "Front Side:");
		gtk_label_set_text(GTK_LABEL(label_side2), "Back Side:");
		gtk_label_set_text(GTK_LABEL(label_special), "Special:");
		gtk_label_set_text(GTK_LABEL(label_tag), "Sector Tag:");

		return;
	}

	rect_t rect = map.l_getrect(line);

	// Main frame
	gtk_frame_set_label(GTK_FRAME(frame_main), parse_string("Line #%d", line).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_main)), "Sans Bold 10");
	gtk_label_set_text(GTK_LABEL(label_length),
					parse_string("Length: %d", (int)distance(rect.x1(), rect.y1(), rect.x2(), rect.y2())).c_str());
	gtk_label_set_text(GTK_LABEL(label_side1), parse_string("Front Side: %d", map.lines[line]->side1).c_str());
	gtk_label_set_text(GTK_LABEL(label_side2), parse_string("Back Side: %d", map.lines[line]->side2).c_str());
	gtk_label_set_text(GTK_LABEL(label_special), parse_string("Special: %d", map.lines[line]->type).c_str());
	gtk_label_set_text(GTK_LABEL(label_tag), parse_string("Sector Tag: %d", map.lines[line]->sector_tag).c_str());

	// Front Side Frame
	if (map.lines[line]->side1 != -1)
	{
		gtk_frame_set_label(GTK_FRAME(frame_side1), parse_string("Front Side (#%d)", map.lines[line]->side1).c_str());
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");
	}
	else
	{
		gtk_frame_set_label(GTK_FRAME(frame_side1), "No Front Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");
	}

	// Back Side Frame
	if (map.lines[line]->side2 != -1)
	{
		gtk_frame_set_label(GTK_FRAME(frame_side2), parse_string("Back Side (#%d)", map.lines[line]->side2).c_str());
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");
	}
	else
	{
		gtk_frame_set_label(GTK_FRAME(frame_side2), "No Front Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");
	}
}
