
#include "main.h"
#include "map.h"

extern Map map;
extern GtkWidget *editor_window;

GtkWidget* setup_flag_checkbox(string name, int flag)
{
	GtkWidget *cbox = gtk_check_button_new_with_label(name.c_str());

	return cbox;
}

GtkWidget* setup_line_edit_props()
{
	GtkWidget *props_page = gtk_vbox_new(false, 0);

	// Flags
	GtkWidget *frame = gtk_frame_new("Flags");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(props_page), frame, true, true, 0);

	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Impassible", LINE_IMPASSIBLE), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Monsters", LINE_BLOCKMONSTERS), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Two Sided", LINE_TWOSIDED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Upper Texture Unpegged", LINE_UPPERUNPEGGED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Lower Texture Unpegged", LINE_LOWERUNPEGGED), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Secret (Shown as 1S on Map)", LINE_SECRET), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

	vbox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Sound", LINE_BLOCKSOUND), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Doesn't Show On Map", LINE_NOTONMAP), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Starts On Map", LINE_STARTONMAP), false, false, 0);
	if (map.hexen)
	{
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Repeatable Special", LINE_REPEATABLE), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Monsters Can Activate", LINE_MONSTERCANACT), false, false, 0);
		gtk_box_pack_start(GTK_BOX(vbox), setup_flag_checkbox("Blocks Everything", LINE_BLOCKALL), false, false, 0);
	}
	gtk_box_pack_start(GTK_BOX(hbox), vbox, true, false, 0);

	gtk_container_add(GTK_CONTAINER(frame), hbox);



	// Properties
	frame = gtk_frame_new("Properties");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_box_pack_start(GTK_BOX(props_page), frame, true, true, 0);

	vbox = gtk_vbox_new(false, 0);

	// Special
	hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

	GtkWidget *label = gtk_label_new("Special:");
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 32, -1);
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);

	GtkWidget *button = gtk_button_new_with_label("Change");
	gtk_widget_set_size_request(button, 96, -1);
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

	if (map.hexen)
	{
		// Args
	}
	else
	{
		// Sector Tag
		hbox = gtk_hbox_new(false, 0);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);

		GtkWidget *label = gtk_label_new("Sector Tag:");
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 0);

		GtkWidget *entry = gtk_entry_new();
		gtk_widget_set_size_request(entry, 32, -1);
		gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 4);

		GtkWidget *button = gtk_button_new_with_label("Find Unused");
		gtk_widget_set_size_request(button, 96, -1);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);

		gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	}

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return props_page;
}

GtkWidget* setup_line_edit()
{
	GtkWidget *tabs = gtk_notebook_new();

	gtk_notebook_append_page(GTK_NOTEBOOK(tabs),
								setup_line_edit_props(),
								gtk_label_new("Properties"));

	GtkWidget *temp = gtk_label_new("Front Side");
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), temp, gtk_label_new("Side 1"));

	temp = gtk_label_new("Back Side");
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), temp, gtk_label_new("Side 2"));

	return tabs;
}

void open_line_edit()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Line",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_line_edit());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
}
