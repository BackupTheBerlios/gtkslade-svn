
#include "main.h"
#include "info_bar.h"
#include "map.h"
#include "misc.h"
#include "textures.h"

GtkWidget *t_frame_main = NULL;
GtkWidget *t_frame_sprite = NULL;

GtkWidget *t_label_type = NULL;
GtkWidget *t_label_x = NULL;
GtkWidget *t_label_y = NULL;
GtkWidget *t_label_angle = NULL;
GtkWidget *t_label_diff = NULL;
GtkWidget *t_label_deaf = NULL;
GtkWidget *t_label_multi = NULL;

GtkWidget *t_box_sprite = NULL;
GtkWidget *t_image_sprite = NULL;

extern Map map;

GtkWidget *get_thing_info_bar()
{
	GtkWidget *hbox = gtk_hbox_new(false, 0);

	// Main frame
	t_frame_main = gtk_frame_new("No Thing Hilighted");
	setup_label(&t_label_type, "Type:");
	setup_label(&t_label_x, "X Position:");
	setup_label(&t_label_y, "Y Position:");
	setup_label(&t_label_angle, "Angle:");
	setup_label(&t_label_diff, "Difficulty:");
	setup_label(&t_label_deaf, "Deaf:");
	setup_label(&t_label_multi, "Multiplayer:");

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_type, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_x, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_y, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_angle, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_diff, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_deaf, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_label_multi, false, false, 0);
	gtk_container_add(GTK_CONTAINER(t_frame_main), vbox);
	gtk_box_pack_start(GTK_BOX(hbox), t_frame_main, true, true, 0);

	// Sprite frame
	t_frame_sprite = gtk_frame_new("Sprite");
	gtk_widget_set_size_request(t_frame_sprite, 144, -1);
	gtk_box_pack_start(GTK_BOX(hbox), t_frame_sprite, false, false, 0);

	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_container_add(GTK_CONTAINER(t_frame_sprite), frame);

	GdkColor color;
	color.red = 30000;
	color.green = 30000;
	color.blue = 30000;
	t_box_sprite = gtk_event_box_new();
	gtk_widget_modify_bg(t_box_sprite, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(frame), t_box_sprite);

	return hbox;
}

void update_thing_info_bar(int thing)
{
	GdkPixbuf *pbuf = gtk_image_get_pixbuf(GTK_IMAGE(t_image_sprite));

	gtk_widget_destroy(t_image_sprite);

	if (pbuf)
		g_object_unref(pbuf);

	t_image_sprite = gtk_image_new();
	gtk_widget_set_size_request(t_image_sprite, 96, 96);
	gtk_container_add(GTK_CONTAINER(t_box_sprite), t_image_sprite);
	gtk_widget_show(t_image_sprite);

	if (thing == -1)
	{
		gtk_frame_set_label(GTK_FRAME(t_frame_main), "No Thing Hilighted");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(t_frame_main)), "Sans Bold 10");

		gtk_label_set_text(GTK_LABEL(t_label_type), "Type:");
		gtk_label_set_text(GTK_LABEL(t_label_x), "X Position:");
		gtk_label_set_text(GTK_LABEL(t_label_y), "Y Position:");
		gtk_label_set_text(GTK_LABEL(t_label_angle), "Angle:");
		gtk_label_set_text(GTK_LABEL(t_label_diff), "Difficulty:");
		gtk_label_set_text(GTK_LABEL(t_label_deaf), "Deaf:");
		gtk_label_set_text(GTK_LABEL(t_label_multi), "Multiplayer:");

		return;
	}

	thing_t *t = map.things[thing];

	const char* ttype = get_thing_type(t->type)->name.c_str();
	gtk_frame_set_label(GTK_FRAME(t_frame_main), parse_string("Thing #%d: %s", thing, ttype).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(t_frame_main)), "Sans Bold 10");

	gtk_label_set_text(GTK_LABEL(t_label_type), parse_string("Type: %d", t->type).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_x), parse_string("X Position: %d", t->x).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_y), parse_string("Y Position: %d", t->y).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_angle), parse_string("Angle: %d (%s)", t->angle, t->angle_string().c_str()).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_diff), parse_string("Difficulty: %s", t->difficulty_string().c_str()).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_deaf), parse_string("Deaf: %s", bool_yesno(t->flags & THING_DEAF).c_str()).c_str());
	gtk_label_set_text(GTK_LABEL(t_label_multi), parse_string("Multiplayer: %s", bool_yesno(t->flags & THING_MULTI).c_str()).c_str());

	gtk_image_set_from_pixbuf(GTK_IMAGE(t_image_sprite), get_texture(t->ttype->spritename, 3)->get_pbuf_scale_fit(96, 96, 1.5f));
}
