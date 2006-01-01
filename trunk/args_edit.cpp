
#include "main.h"
#include "args.h"

GtkWidget* arg_labels[5];
GtkWidget* arg_entrys[5];

BYTE arg_value_dialog_val = 0;
vector<string> aedit_arg_types;
BYTE* aedit_args = NULL;
bool* aedit_args_consistent = NULL;

extern GtkWidget *editor_window;

void aedit_arg_value_radio_selected(GtkWidget *widget, gpointer data)
{
	int val = (int)data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		arg_value_dialog_val = val;
}

void aedit_arg_flag_cbox_toggled(GtkWidget *widget, gpointer data)
{
	int val = (int)data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		arg_value_dialog_val |= val;
	else
		arg_value_dialog_val = (arg_value_dialog_val & ~val);
}

int open_arg_value_dialog(int val, argtype_t *at)
{
	arg_value_dialog_val = val;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Arg",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);

	GtkWidget *values_frame = gtk_frame_new("Values");
	gtk_container_set_border_width(GTK_CONTAINER(values_frame), 4);
	gtk_box_pack_start(GTK_BOX(main_vbox), values_frame, false, false, 0);
	GtkWidget *values_hbox = gtk_hbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(values_frame), values_hbox);
	GtkWidget *values_vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(values_vbox), 4);
	gtk_box_pack_start(GTK_BOX(values_hbox), values_vbox, false, false, 0);

	GtkWidget *flags_frame = gtk_frame_new("Flags");
	gtk_container_set_border_width(GTK_CONTAINER(flags_frame), 4);
	GtkWidget *flags_vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(flags_vbox), 4);
	gtk_container_add(GTK_CONTAINER(flags_frame), flags_vbox);

	if (at->has_flags)
		gtk_box_pack_start(GTK_BOX(main_vbox), flags_frame, false, false, 0);

	int rows = 0;
	GtkWidget *radgroup = gtk_radio_button_new(NULL);
	for (int a = 0; a < at->values.size(); a++)
	{
		if (!at->values[a].flag)
		{
			GtkWidget *button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radgroup), at->values[a].name.c_str());
			g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(aedit_arg_value_radio_selected), (gpointer)at->values[a].value);

			if (at->values[a].value == val)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);

			gtk_box_pack_start(GTK_BOX(values_vbox), button, false, false, 0);

			rows++;
			if (rows == 15)
			{
				values_vbox = gtk_vbox_new(false, 0);
				gtk_container_set_border_width(GTK_CONTAINER(values_vbox), 4);
				gtk_box_pack_start(GTK_BOX(values_hbox), values_vbox, false, false, 0);
				rows = 0;
			}
		}
		else
		{
			GtkWidget *button = gtk_check_button_new_with_label(at->values[a].name.c_str());
			g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(aedit_arg_flag_cbox_toggled), (gpointer)at->values[a].value);

			if (at->values[a].value & val)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);

			gtk_box_pack_start(GTK_BOX(flags_vbox), button, false, false, 0);
		}
	}

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), main_vbox);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 300, -1);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	if (response == GTK_RESPONSE_ACCEPT)
		return arg_value_dialog_val;
	else
		return val;
}

void arg_entry_changed(GtkEditable *editable, gpointer data)
{
	GtkEntry* entry = (GtkEntry*)editable;
	int arg = (int)data;
	argtype_t* arg_type = get_arg_type(aedit_arg_types[arg]);
	string text = gtk_entry_get_text(entry);

	if (text != "")
	{
		aedit_args[arg] = atoi(text.c_str());
		aedit_args_consistent[arg] = true;
	}
	else
		aedit_args_consistent[arg] = false;

	if (arg_type)
	{
		string val_name = arg_type->get_name(atoi(gtk_entry_get_text(entry)));
		gtk_label_set_text(GTK_LABEL(arg_labels[arg]), val_name.c_str());
		gtk_misc_set_alignment(GTK_MISC(arg_labels[arg]), 0.0, 0.5);
	}
}

void aedit_change_button_click(GtkWidget *widget, gpointer data)
{
	int arg = (int)data;

	argtype_t* at = get_arg_type(aedit_arg_types[arg]);
	
	if (at)
	{
		int val = open_arg_value_dialog(aedit_args[arg], at);
		gtk_entry_set_text(GTK_ENTRY(arg_entrys[arg]), parse_string("%d", val).c_str());
	}
}

GtkWidget* get_args_editor(BYTE* args, string* argnames, string* argtypes, bool* consistent)
{
	GtkWidget *vbox = gtk_vbox_new(true, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	aedit_arg_types.clear();
	aedit_args = args;
	aedit_args_consistent = consistent;

	for (int a = 0; a < 5; a++)
	{
		aedit_arg_types.push_back(argtypes[a]);

		GtkWidget *hbox = gtk_hbox_new(false, 0);

		// Labels
		GtkWidget *label = gtk_label_new(parse_string("%s:", argnames[a].c_str()).c_str());
		gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);

		// Label
		arg_labels[a] = gtk_label_new(NULL);
		gtk_box_pack_start(GTK_BOX(hbox), arg_labels[a], true, true, 4);

		// Entry
		arg_entrys[a] = gtk_entry_new();
		gtk_widget_set_size_request(GTK_WIDGET(arg_entrys[a]), 32, -1);
		g_signal_connect(G_OBJECT(arg_entrys[a]), "changed", G_CALLBACK(arg_entry_changed), (gpointer)a);
		gtk_box_pack_start(GTK_BOX(hbox), arg_entrys[a], false, false, 4);
		if (consistent[a])
			gtk_entry_set_text(GTK_ENTRY(arg_entrys[a]), parse_string("%d", args[a]).c_str());

		// Button
		GtkWidget *button = gtk_button_new_with_label("Change");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(aedit_change_button_click), (gpointer)a);
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 4);

		gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	}

	return vbox;
}

void open_args_edit(BYTE* args, string* argnames, string* argtypes, bool* consistent)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Args",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), get_args_editor(args, argnames, argtypes, consistent));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 300, -1);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));
}
