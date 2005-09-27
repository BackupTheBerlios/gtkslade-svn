
#include "main.h"
#include "info_bar.h"
#include "map.h"
#include "misc.h"

GtkWidget *v_frame_main = NULL;
GtkWidget *v_label_x = NULL;
GtkWidget *v_label_y = NULL;

extern Map map;

GtkWidget *get_vertex_info_bar()
{
	// Main frame
	v_frame_main = gtk_frame_new("No Vertex Hilighted");
	setup_label(&v_label_x, "X Position:");
	setup_label(&v_label_y, "Y Position:");

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), v_label_x, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), v_label_y, false, false, 0);
	gtk_container_add(GTK_CONTAINER(v_frame_main), vbox);

	return v_frame_main;
}

void update_vertex_info_bar(int vertex)
{
	if (vertex == -1)
	{
		gtk_frame_set_label(GTK_FRAME(v_frame_main), "No Vertex Hilighted");
		widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(v_frame_main)), "Sans Bold 10");
		gtk_label_set_text(GTK_LABEL(v_label_x), "X Position:");
		gtk_label_set_text(GTK_LABEL(v_label_y), "Y Position:");

		return;
	}

	gtk_frame_set_label(GTK_FRAME(v_frame_main), parse_string("Vertex #%d", vertex).c_str());
	widget_set_font(gtk_frame_get_label_widget(GTK_FRAME(v_frame_main)), "Sans Bold 10");

	gtk_label_set_text(GTK_LABEL(v_label_x), parse_string("X Position: %d", map.verts[vertex]->x).c_str());
	gtk_label_set_text(GTK_LABEL(v_label_y), parse_string("Y Position: %d", map.verts[vertex]->y).c_str());
}
