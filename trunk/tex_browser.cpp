
#include "main.h"
#include "textures.h"
#include "tex_box.h"

string selected_tex = "";

CVAR(Int, browser_columns, 8, CVAR_SAVE)

extern GtkWidget *editor_window;
extern vector<Texture*> textures;
extern vector<Texture*> flats;
extern vector<Texture*> sprites;

GtkWidget* setup_texture_browser(vector<string> tex_names)
{
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	int rows = tex_names.size() / browser_columns;

	GtkWidget *table = gtk_table_new(rows, browser_columns, true);
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
