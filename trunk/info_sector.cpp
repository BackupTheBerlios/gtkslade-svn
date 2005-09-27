
#include "main.h"
#include "info_bar.h"
#include "map.h"
#include "misc.h"

GtkWidget *s_frame_main = NULL;
GtkWidget *s_frame_floor = NULL;
GtkWidget *s_frame_ceil = NULL;

GtkWidget *s_label_floor = NULL;
GtkWidget *s_label_ceil = NULL;
GtkWidget *s_label_height = NULL;
GtkWidget *s_label_light = NULL;
GtkWidget *s_label_tag = NULL;
GtkWidget *s_label_special = NULL;

extern Map map;

GtkWidget *get_sector_info_bar()
{
	// Main frame
	s_frame_main = gtk_frame_new("No Sector Hilighted");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_main)), "Sans Bold 10");
	setup_label(&s_label_floor, "Floor Height:");
	setup_label(&s_label_ceil, "Ceiling Height:");
	setup_label(&s_label_height, "Sector Height:");
	setup_label(&s_label_light, "Light Level:");
	setup_label(&s_label_tag, "Tag:");
	setup_label(&s_label_special, "Special:");

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_floor, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_ceil, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_height, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_light, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_tag, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), s_label_special, false, false, 0);
	gtk_container_add(GTK_CONTAINER(s_frame_main), vbox);

	// Floor frame
	s_frame_floor = gtk_frame_new("Floor");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_floor)), "Sans Bold 10");
	gtk_widget_set_size_request(s_frame_floor, 144, -1);

	// Ceiling frame
	s_frame_ceil = gtk_frame_new("Ceiling");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_ceil)), "Sans Bold 10");
	gtk_widget_set_size_request(s_frame_ceil, 144, -1);

	// Stuff
	GtkWidget *main_hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), s_frame_main, true, true, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), s_frame_floor, false, false, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), s_frame_ceil, false, false, 0);

	return main_hbox;
}

void update_sector_info_bar(int sector)
{
	if (sector == -1)
	{
		gtk_frame_set_label(GTK_FRAME(s_frame_main), "No Sector Hilighted");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_main)), "Sans Bold 10");
		gtk_frame_set_label(GTK_FRAME(s_frame_floor), "Floor");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_floor)), "Sans Bold 10");
		gtk_frame_set_label(GTK_FRAME(s_frame_ceil), "Ceiling");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_ceil)), "Sans Bold 10");

		gtk_label_set_text(GTK_LABEL(s_label_floor), "Floor Height:");
		gtk_label_set_text(GTK_LABEL(s_label_ceil), "Ceiling Height:");
		gtk_label_set_text(GTK_LABEL(s_label_height), "Sector Height:");
		gtk_label_set_text(GTK_LABEL(s_label_light), "Light Level:");
		gtk_label_set_text(GTK_LABEL(s_label_tag), "Tag:");
		gtk_label_set_text(GTK_LABEL(s_label_special), "Special:");

		return;
	}

	sector_t *s = map.sectors[sector];

	// Main frame
	gtk_frame_set_label(GTK_FRAME(s_frame_main), parse_string("Sector #%d", sector).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_main)), "Sans Bold 10");

	gtk_label_set_text(GTK_LABEL(s_label_floor), parse_string("Floor Height: %d", s->f_height).c_str());
	gtk_label_set_text(GTK_LABEL(s_label_ceil), parse_string("Ceiling Height: %d", s->c_height).c_str());
	gtk_label_set_text(GTK_LABEL(s_label_height), parse_string("Sector Height: %d", s->c_height - s->f_height).c_str());
	gtk_label_set_text(GTK_LABEL(s_label_light), parse_string("Light Level: %d", s->light).c_str());
	gtk_label_set_text(GTK_LABEL(s_label_tag), parse_string("Tag: %d", s->tag).c_str());
	gtk_label_set_text(GTK_LABEL(s_label_special), parse_string("Special: %d", s->special).c_str());

	// Floor frame
	gtk_frame_set_label(GTK_FRAME(s_frame_floor), parse_string("Floor: %s", s->f_tex.c_str()).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_floor)), "Sans Bold 10");

	// Ceiling frame
	gtk_frame_set_label(GTK_FRAME(s_frame_ceil), parse_string("Ceiling: %s", s->c_tex.c_str()).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(s_frame_ceil)), "Sans Bold 10");
}
