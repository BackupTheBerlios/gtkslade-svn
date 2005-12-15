
#include "main.h"
#include "edit.h"
#include "map.h"

GtkWidget	*status_label_text;
GtkWidget	*status_label_mode;
GtkWidget	*status_label_snap;
GtkWidget	*status_label_mousepos;

int prev_edit_mode = 1;
bool prev_snap;

extern point2_t	mouse;
extern Map map;
extern int edit_mode;

EXTERN_CVAR(Bool, edit_snap_grid)

GtkWidget* setup_status_bar()
{
	GtkWidget *m_hbox = gtk_hbox_new(false, 2);
	gtk_container_set_border_width(GTK_CONTAINER(m_hbox), 1);
	
	// Status text
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(m_hbox), frame, true, true, 0);

	status_label_text = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(status_label_text), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(frame), status_label_text);

	// Edit Mode
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(m_hbox), frame, false, false, 0);

	status_label_mode = gtk_label_new("Mode: Lines");
	gtk_misc_set_alignment(GTK_MISC(status_label_mode), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(frame), status_label_mode);

	// Grid Snap
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(m_hbox), frame, false, false, 0);

	if (edit_snap_grid)
		status_label_snap = gtk_label_new("Snap: ON");
	else
		status_label_snap = gtk_label_new("Snap: OFF");
	gtk_misc_set_alignment(GTK_MISC(status_label_snap), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(frame), status_label_snap);

	prev_snap = edit_snap_grid;


	// Cursor position
	frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(frame, 72, -1);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(m_hbox), frame, false, false, 0);

	status_label_mousepos = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(status_label_mousepos), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(frame), status_label_mousepos);

	return m_hbox;
}

void update_status_bar()
{
	// Mouse x,y
	if (map.opened)
		gtk_label_set_text(GTK_LABEL(status_label_mousepos), parse_string("%d, %d", (int)m_x(mouse.x), -(int)m_y(mouse.y)).c_str());
	else
		gtk_label_set_text(GTK_LABEL(status_label_mousepos), "");

	// Edit mode
	if (edit_mode != prev_edit_mode)
	{
		string text = "Mode: ?????";
		if (edit_mode == 0) text = "Mode: Vertices";
		if (edit_mode == 1) text = "Mode: Lines";
		if (edit_mode == 2) text = "Mode: Sectors";
		if (edit_mode == 3) text = "Mode: Things";
		gtk_label_set_text(GTK_LABEL(status_label_mode), text.c_str());
		prev_edit_mode = edit_mode;
	}

	// Grid snap
	if (edit_snap_grid != prev_snap)
	{
		if (edit_snap_grid)
			gtk_label_set_text(GTK_LABEL(status_label_snap), "Snap: ON");
		else
			gtk_label_set_text(GTK_LABEL(status_label_snap), "Snap: OFF");
		prev_snap = edit_snap_grid;
	}
}
