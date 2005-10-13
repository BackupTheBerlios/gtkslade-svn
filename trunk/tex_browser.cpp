
#include "main.h"
#include "textures.h"
#include "tex_box.h"

string selected_tex = "";

CVAR(Int, browser_columns, 4, CVAR_SAVE)

extern GtkWidget *editor_window;
extern vector<Texture*> textures;
extern vector<Texture*> flats;
extern vector<Texture*> sprites;

GtkWidget* setup_texture_browser(vector<string> tex_names)
{
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	int rows = tex_names.size() / browser_columns;

	GtkWidget *table = gtk_table_new(rows, browser_columns, true);

	int tex = 0;

	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < browser_columns; col++)
		{
			if (tex > tex_names.size())
				continue;

			GtkWidget *frame = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, true);
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);

			tex_box_t* tbox = new tex_box_t(tex_names[tex], 0, 5.0f, rgba_t(0, 0, 0, 255, 0));
			tbox->set_size(64, 64);
			gtk_container_add(GTK_CONTAINER(frame), tbox->widget);

			gtk_table_attach_defaults(GTK_TABLE(table), frame, col, col+1, row, row+1);
			tex++;
		}
	}

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(s_window), table);

	return s_window;
}

string open_texture_browser(bool tex, bool flat, bool sprite)
{
	vector<string> tex_names;

	if (tex)
	{
		for (int a = 0; a < textures.size(); a++)
			tex_names.push_back(textures[a]->name);
	}

	if (flat)
	{
		for (int a = 0; a < flats.size(); a++)
			tex_names.push_back(flats[a]->name);
	}

	if (sprite)
	{
		for (int a = 0; a < sprites.size(); a++)
			tex_names.push_back(sprites[a]->name);
	}

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Textures",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_texture_browser(tex_names));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 400);
	gtk_widget_show_all(dialog);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));

	return "";
}
