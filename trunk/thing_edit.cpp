
#include "main.h"
#include "map.h"
#include "line_edit.h"

extern GtkWidget *editor_window;
extern Map map;
extern vector<int> set_flags;
extern vector<int> unset_flags;

GtkWidget* setup_thing_edit()
{
	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	// Flags
	GtkWidget *frame = gtk_frame_new("Flags");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, true, true, 0);

	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Easy", THING_EASY), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Medium", THING_MEDIUM), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears On Hard", THING_HARD), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Deaf (Only wakes on sight)", THING_DEAF), false, false, 0);
	if (map.hexen)
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Dormant", THING_DORMANT), false, false, 0);
	else
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Multiplayer", THING_MULTI), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

	if (map.hexen)
	{
		vbox = gtk_vbox_new(false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Fighter", THING_FIGHTER), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Cleric", THING_CLERIC), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears To Mage", THING_MAGE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Single Player", THING_SINGLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Cooperative", THING_COOPERATIVE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

		vbox = gtk_vbox_new(false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Appears In Deathmatch", THING_DEATHMATCH), false, false, 0);
	}

	if (map.zdoom)
	{
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Translucent", THING_TRANSLUCENT), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Invisible", THING_INVISIBLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Friendly", THING_FRIENDLY), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Stands Still", THING_STANDSTILL), false, false, 0);
	}

	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	// stuff

	return main_vbox;
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
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	/*
	if (response == GTK_RESPONSE_ACCEPT)
	{
		apply_line_edit();
		force_map_redraw(true, false);
	}
	*/

	gtk_widget_destroy(dialog);
}
