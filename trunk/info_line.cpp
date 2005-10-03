
#include "main.h"
#include "info_bar.h"
#include "map.h"
#include "misc.h"
#include "textures.h"
#include "action_special.h"

GtkWidget *frame_main = NULL;
GtkWidget *frame_side1 = NULL;
GtkWidget *frame_side2 = NULL;

// Main frame
GtkWidget *label_length = NULL;
GtkWidget *label_side1 = NULL;
GtkWidget *label_side2 = NULL;
GtkWidget *label_special = NULL;
GtkWidget *label_tag = NULL;

// Side1 frame
GtkWidget *image_uptex1 = NULL;
GtkWidget *image_lotex1 = NULL;
GtkWidget *image_midtex1 = NULL;
GtkWidget *box_uptex1 = NULL;
GtkWidget *box_lotex1 = NULL;
GtkWidget *box_midtex1 = NULL;
GtkWidget *label_uptex1 = NULL;
GtkWidget *label_lotex1 = NULL;
GtkWidget *label_midtex1 = NULL;

// Side2 frame
GtkWidget *image_uptex2 = NULL;
GtkWidget *image_lotex2 = NULL;
GtkWidget *image_midtex2 = NULL;
GtkWidget *box_uptex2 = NULL;
GtkWidget *box_lotex2 = NULL;
GtkWidget *box_midtex2 = NULL;
GtkWidget *label_uptex2 = NULL;
GtkWidget *label_lotex2 = NULL;
GtkWidget *label_midtex2 = NULL;

extern Map map;
extern vector<Texture*> textures;

GtkWidget *setup_image_box(GtkWidget **box, string texname, GtkWidget **label)
{
	GdkColor colour;
	colour.red = 30000;
	colour.green = 30000;
	colour.blue = 30000;

	GtkWidget *vbox = gtk_vbox_new(false, 0);

	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(texname.c_str()), false, false, 0);

	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(vbox), frame, true, true, 0);

	*box = gtk_event_box_new();
	gtk_widget_set_size_request(*box, 72, -1);
	gtk_widget_modify_bg(*box, GTK_STATE_NORMAL, &colour);
	gtk_container_add(GTK_CONTAINER(frame), *box);

	*label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), *label, false, false, 0);

	return vbox;
}

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

	GtkWidget *hbox = gtk_hbox_new(true, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_uptex1, "Upper", &label_uptex1), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_midtex1, "Middle", &label_midtex1), false, false, 4);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_lotex1, "Lower", &label_lotex1), false, false, 4);
	gtk_container_add(GTK_CONTAINER(frame_side1), hbox);


	// Back Side Frame
	frame_side2 = gtk_frame_new("No Back Side");
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");
	
	hbox = gtk_hbox_new(true, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_uptex2, "Upper", &label_uptex2), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_midtex2, "Middle", &label_midtex2), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), setup_image_box(&box_lotex2, "Lower", &label_lotex2), false, false, 0);
	gtk_container_add(GTK_CONTAINER(frame_side2), hbox);


	// Stuff
	GtkWidget *main_hbox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_main, true, true, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_side1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), frame_side2, false, false, 0);

	return main_hbox;
}

void process_image(GtkWidget **image, GtkWidget *box)
{
	GdkPixbuf *pbuf = gtk_image_get_pixbuf(GTK_IMAGE(*image));

	gtk_widget_destroy(*image);

	if (pbuf)
		g_object_unref(pbuf);

	*image = gtk_image_new();
	gtk_widget_set_size_request(*image, 72, 72);
	//gtk_box_pack_start(GTK_BOX(box), *image, false, false, 0);
	gtk_container_add(GTK_CONTAINER(box), *image);
	gtk_widget_show(*image);
}

void update_line_info_bar(int line)
{
	process_image(&image_uptex1, box_uptex1);
	process_image(&image_midtex1, box_midtex1);
	process_image(&image_lotex1, box_lotex1);
	process_image(&image_uptex2, box_uptex2);
	process_image(&image_midtex2, box_midtex2);
	process_image(&image_lotex2, box_lotex2);

	gtk_label_set_text(GTK_LABEL(label_uptex1), "");
	gtk_label_set_text(GTK_LABEL(label_midtex1), "");
	gtk_label_set_text(GTK_LABEL(label_lotex1), "");
	gtk_label_set_text(GTK_LABEL(label_uptex2), "");
	gtk_label_set_text(GTK_LABEL(label_midtex2), "");
	gtk_label_set_text(GTK_LABEL(label_lotex2), "");

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
	gtk_label_set_text(GTK_LABEL(label_tag), parse_string("Sector Tag: %d", map.lines[line]->sector_tag).c_str());

	// Special
	const char* special = get_special(map.lines[line]->type)->name.c_str();
	gtk_label_set_text(GTK_LABEL(label_special), parse_string("Special: %d (%s)", map.lines[line]->type, special).c_str());

	// Front Side Frame
	if (map.lines[line]->side1 != -1)
	{
		sidedef_t *side = map.sides[map.lines[line]->side1];
		gtk_frame_set_label(GTK_FRAME(frame_side1), parse_string("Front Side (#%d)", map.lines[line]->side1).c_str());
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");

		if (map.l_needsuptex(line, 1))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_uptex1),
				get_texture(side->tex_upper)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_uptex1), side->tex_upper.c_str());
		}

		if (map.l_needsmidtex(line, 1))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_midtex1),
				get_texture(side->tex_middle)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_midtex1), side->tex_middle.c_str());
		}

		if (map.l_needslotex(line, 1))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_lotex1),
				get_texture(side->tex_lower)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_lotex1), side->tex_lower.c_str());
		}
	}
	else
	{
		gtk_frame_set_label(GTK_FRAME(frame_side1), "No Front Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side1)), "Sans Bold 10");
	}

	// Back Side Frame
	if (map.lines[line]->side2 != -1)
	{
		sidedef_t *side = map.sides[map.lines[line]->side2];
		gtk_frame_set_label(GTK_FRAME(frame_side2), parse_string("Back Side (#%d)", map.lines[line]->side2).c_str());
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");

		if (map.l_needsuptex(line, 2))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_uptex2),
				get_texture(side->tex_upper)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_uptex2), side->tex_upper.c_str());
		}

		if (map.l_needsmidtex(line, 2))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_midtex2),
				get_texture(side->tex_middle)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_midtex2), side->tex_middle.c_str());
		}

		if (map.l_needslotex(line, 2))
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(image_lotex2),
				get_texture(side->tex_lower)->get_pbuf_scale_fit(72, 72));
			gtk_label_set_text(GTK_LABEL(label_lotex2), side->tex_lower.c_str());
		}
	}
	else
	{
		gtk_frame_set_label(GTK_FRAME(frame_side2), "No Front Side");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(frame_side2)), "Sans Bold 10");
	}
}
