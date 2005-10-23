
#include "main.h"
#include "map.h"
#include "misc.h"

GtkTextBuffer	*script_buffer;
GtkWidget		*script_editor = NULL;

extern Map map;
extern Wad *edit_wad;

void script_editor_compile_click(GtkWidget *w, gpointer data)
{
	remove("sladetemp.o");

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(script_buffer, &start, &end);

	// Save the script
	map.scripts->LoadData((guint8*)gtk_text_buffer_get_text(script_buffer, &start, &end, false),
							gtk_text_buffer_get_char_count(script_buffer));
	map.scripts->DumpToFile("sladetemp.acs");

	// Compile it
	system("acc sladetemp.acs sladetemp.o");

	// Open the output
	FILE* fp = fopen("sladetemp.o", "rb");

	// If output doesn't exist some kind of error occurred
	if (!fp)
	{
		fp = fopen("acs.err", "rt");

		if (!fp)
			message_box("Compile failed", GTK_MESSAGE_ERROR);
		else
		{
			// Open a dialog with the contents of acs.err
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			gchar* text = (gchar*)malloc(len);
			fseek(fp, 0, SEEK_SET);
			fread(text, len, 1, fp);
			string error = text;
			free(text);

			GtkWidget *dialog = gtk_dialog_new_with_buttons("ACC Error",
															GTK_WINDOW(script_editor),
															GTK_DIALOG_MODAL,
															GTK_STOCK_OK,
															GTK_RESPONSE_ACCEPT,
															NULL);

			GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
			gtk_text_buffer_set_text(buffer, error.c_str(), -1);
			GtkWidget *text_view = gtk_text_view_new_with_buffer(buffer);
			widget_set_font(text_view, "Monospace", 10);
			GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(s_window), GTK_SHADOW_IN);
			gtk_container_set_border_width(GTK_CONTAINER(s_window), 4);
			gtk_container_add(GTK_CONTAINER(s_window), text_view);
			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), s_window);

			gtk_widget_show_all(dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			fclose(fp);
		}
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		BYTE* data = (BYTE*)malloc(len);
		fseek(fp, 0, SEEK_SET);
		fread(data, len, 1, fp);

		map.behavior->LoadData(data, len);

		fclose(fp);
		remove("sladetemp.o");
	}
}

void script_editor_save_click(GtkWidget *w, gpointer data)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(script_buffer, &start, &end);

	map.scripts->LoadData((guint8*)gtk_text_buffer_get_text(script_buffer, &start, &end, false),
							gtk_text_buffer_get_char_count(script_buffer));
}

gboolean script_editor_destroy(GtkWidget *w, gpointer data)
{
	gtk_widget_hide(w);
	return true;
}

GtkWidget* setup_script_edit()
{
	script_buffer = gtk_text_buffer_new(NULL);

	script_editor = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(script_editor), "Script Editor");
	gtk_window_set_default_size(GTK_WINDOW(script_editor), 500, 500);
	g_signal_connect(G_OBJECT(script_editor), "delete_event", G_CALLBACK(script_editor_destroy), NULL);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(script_editor), vbox);

	// Scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(s_window), GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(s_window), 4);
	gtk_box_pack_start(GTK_BOX(vbox), s_window, true, true, 0);

	// Text view
	GtkWidget *script_view = gtk_text_view_new_with_buffer(script_buffer);
	widget_set_font(script_view, "Monospace", 10);
	gtk_container_add(GTK_CONTAINER(s_window), script_view);

	// Tab stops
	// This is the most insane way to define tab stops I've ever seen, why isn't there anything in gtk to
	// simply set it to '4 spaces' instead of the 'standard 8 spaces'?
	/*
	PangoTabArray *tab_array = pango_tab_array_new(512, false);
	for (int a = 0; a < 512; a++)
		pango_tab_array_set_tab(tab_array, a, PANGO_TAB_LEFT, a * 4);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(script_view), tab_array);
	*/

	// Buttons
	GtkWidget *bbox = gtk_hbutton_box_new();
	gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, false, false, 0);

	GtkWidget *button = gtk_button_new_with_label("Save Changes");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(script_editor_save_click), NULL);
	gtk_box_pack_start(GTK_BOX(bbox), button, false, false, 0);

	button = gtk_button_new_with_label("Compile Script");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(script_editor_compile_click), NULL);
	gtk_box_pack_start(GTK_BOX(bbox), button, false, false, 0);

	return vbox;
}

void open_script_edit()
{
	if (!script_editor)
		setup_script_edit();

	if (map.opened && map.hexen)
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(script_buffer), (const gchar*)map.scripts->Data(), map.scripts->Size());

	gtk_widget_show_all(script_editor);
}
