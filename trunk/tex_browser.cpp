
#include "main.h"
#include "textures.h"
#include "tex_box.h"
#include "map.h"
#include "draw.h"
#include <gdk/gdkkeysyms.h>

bool browse_sprites = false;
string selected_tex = "";
string tex_search = "";
int rows = 0;
vector<string> tex_names;
vector<string> browsesprites;
GtkWidget *browse_vscroll;
GtkWidget *browser_dialog;

CVAR(Int, browser_columns, 6, CVAR_SAVE)
CVAR(Int, browser_sort, 1, CVAR_SAVE)

extern GtkWidget *editor_window;
extern vector<Texture*> textures;
extern vector<Texture*> flats;
extern vector<Texture*> sprites;
extern GdkGLConfig *glconfig;
extern GdkGLContext *glcontext;

extern rgba_t col_selbox;
extern rgba_t col_selbox_line;

extern bool mix_tex;

extern Map map;

void scroll_to_selected_texture(GtkWidget* w)
{
	int width = (w->allocation.width / browser_columns);
	int top = gtk_range_get_value(GTK_RANGE(browse_vscroll));
	int bottom = top + w->allocation.height;

	int a = 0;
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < browser_columns; x++)
		{
			if (browse_sprites)
			{
				if (browsesprites[a] == selected_tex)
				{
					if (y * width < top)
						gtk_range_set_value(GTK_RANGE(browse_vscroll), y * width);

					if ((y+1) * width > bottom)
						gtk_range_set_value(GTK_RANGE(browse_vscroll), ((y+1) * width) - w->allocation.height);

					return;
				}
			}
			else
			{
				if (tex_names[a] == selected_tex)
				{
					if (y * width < top)
						gtk_range_set_value(GTK_RANGE(browse_vscroll), y * width);

					if ((y+1) * width > bottom)
						gtk_range_set_value(GTK_RANGE(browse_vscroll), ((y+1) * width) - w->allocation.height);

					return;
				}
			}
			a++;

			if (a >= tex_names.size())
				return;
		}
	}
}

gboolean browser_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkGLContext *context = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return false;

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, widget->allocation.width, widget->allocation.height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gdk_gl_drawable_gl_end(gldrawable);

	rows = (tex_names.size() / browser_columns) + 1;

	int width = widget->allocation.width / browser_columns;
	int rows_page = widget->allocation.height / width;
	gtk_range_set_range(GTK_RANGE(browse_vscroll), 0.0, (rows * width) - widget->allocation.height);

	scroll_to_selected_texture(widget);

	return true;
}

gboolean browser_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
	int width = w->allocation.width / browser_columns;
	rows = (tex_names.size() / browser_columns) + 1;
	int top = gtk_range_get_value(GTK_RANGE(browse_vscroll));

	// Set sizes for row and page steps (for the scrollbar)
	int rows_page = w->allocation.height / width;
	gtk_range_set_increments(GTK_RANGE(browse_vscroll), width, rows_page * width);

	GdkGLContext *context = gtk_widget_get_gl_context(w);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(w);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return false;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int sel_index = -1;
	int a = 0;
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < browser_columns; col++)
		{
			if (a >= tex_names.size())
				continue;

			rect_t rect(col * width, (row * width) - top, width, width, 0);

			glLineWidth(2.0f);
			if (selected_tex == tex_names[a])
			{
				draw_rect(rect, rgba_t(0, 180, 255, 150, 0), true);
				draw_rect(rect, rgba_t(100, 220, 255, 255, 0), false);
				sel_index = a;
			}

			glLineWidth(1.0f);
			rect.resize(-8, -8);

			if (((row + 1) * width) > top && (row * width) < (top + w->allocation.height))
			{
				if (!browse_sprites)
				{
					if (tex_names[a] != "-")
						draw_texture_scale(rect, tex_names[a], 0);

					draw_text(rect.x1() + (width/2) - 8, rect.y2() - 4, rgba_t(255, 255, 255, 255, 0), 1, tex_names[a].c_str());
				}
				else
					draw_texture_scale(rect, browsesprites[a], 3);
			}

			a++;
		}
	}

	if (browse_sprites && sel_index != -1)
		draw_text(0, 0, rgba_t(255, 255, 255, 255, 0), 0, tex_names[sel_index].c_str());

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);

	return false;
}

gboolean browse_vscroll_change(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data)
{
	GtkWidget *draw_area = (GtkWidget*)data;
	gdk_window_invalidate_rect(draw_area->window, &draw_area->allocation, false);
	return false;
}

gboolean browser_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	int width = (widget->allocation.width / browser_columns) / 2;
	int top = gtk_range_get_value(GTK_RANGE(browse_vscroll));

	if (event->direction == GDK_SCROLL_UP)
		gtk_range_set_value(GTK_RANGE(browse_vscroll), top - width);
	else if (event->direction = GDK_SCROLL_DOWN)
		gtk_range_set_value(GTK_RANGE(browse_vscroll), top + width);

	gdk_window_invalidate_rect(widget->window, &widget->allocation, false);
	return false;
}

static gboolean browser_click_event(GtkWidget *widget, GdkEventButton *event)
{
	int width = widget->allocation.width / browser_columns;

	if (event->button == 1)
	{
		if (event->type == GDK_2BUTTON_PRESS)
			gtk_dialog_response(GTK_DIALOG(browser_dialog), GTK_RESPONSE_ACCEPT);
		else
		{
			int row = (gtk_range_get_value(GTK_RANGE(browse_vscroll)) + event->y) / width;
			int col = event->x / width;
			int index = (row * browser_columns) + col;

			if (index < tex_names.size())
				selected_tex = tex_names[index];

			gdk_window_invalidate_rect(widget->window, &widget->allocation, false);
		}
	}

	return false;
}

gboolean browser_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	char key = gdk_keyval_name(event->keyval)[0];

	int index = 0;
	for (int a = 0; a < tex_names.size(); a++)
	{
		if (tex_names[a] == selected_tex)
			index = a;
	}

	bool done = false;

	if (event->keyval == GDK_Return)
		gtk_dialog_response(GTK_DIALOG(browser_dialog), GTK_RESPONSE_ACCEPT);

	if (event->keyval == GDK_Up && index - browser_columns >= 0)
	{
		index -= browser_columns;
		done = true;
	}

	if (event->keyval == GDK_Left && index > 0)
	{
		index--;
		done = true;
	}

	if (event->keyval == GDK_Right && index < tex_names.size() - 1)
	{
		index++;
		done = true;
	}

	if (event->keyval == GDK_Down && index < tex_names.size() - browser_columns)
	{
		index += browser_columns;
		done = true;
	}

	if (event->keyval == GDK_BackSpace)
	{
		string search = gtk_entry_get_text(GTK_ENTRY(data));
		gtk_entry_set_text(GTK_ENTRY(data), search.substr(0, search.size() - 1).c_str());
		done = true;
	}

	if (done)
	{
		selected_tex = tex_names[index];
		scroll_to_selected_texture(widget);
		gdk_window_invalidate_rect(widget->window, &widget->allocation, false);
	}
	else
	{
		string search = gtk_entry_get_text(GTK_ENTRY(data));
		search += event->string;
		gtk_entry_set_text(GTK_ENTRY(data), search.c_str());
	}

	return true;
}

void browser_search_entry_changed(GtkWidget *w, gpointer data)
{
	string search = g_strup((gchar*)gtk_entry_get_text(GTK_ENTRY(w)));

	for (int a = 0; a < tex_names.size(); a++)
	{
		if (tex_names[a].size() < search.size())
			continue;

		bool match = true;
		for (int c = 0; c < search.size(); c++)
		{
			if (tex_names[a][c] != search[c])
				match = false;
		}

		if (match)
		{
			selected_tex = tex_names[a];
			scroll_to_selected_texture(GTK_WIDGET(data));
			gdk_window_invalidate_rect(GTK_WIDGET(data)->window, &GTK_WIDGET(data)->allocation, false);
			return;
		}
	}
}

GtkWidget* setup_texture_browser()
{
	GtkWidget *hbox = gtk_hbox_new(false, 0);

	GtkWidget *draw_area = gtk_drawing_area_new();
	GTK_WIDGET_SET_FLAGS(draw_area, GTK_CAN_FOCUS);
	gtk_widget_set_gl_capability(draw_area, glconfig, glcontext, TRUE, GDK_GL_RGBA_TYPE);
	gtk_widget_set_events(draw_area, GDK_EXPOSURE_MASK|GDK_LEAVE_NOTIFY_MASK|GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(draw_area), "expose-event", G_CALLBACK(browser_expose_event), NULL);
	g_signal_connect(G_OBJECT(draw_area), "configure-event", G_CALLBACK(browser_configure_event), NULL);
	g_signal_connect(G_OBJECT(draw_area), "button_press_event", G_CALLBACK(browser_click_event), NULL);
	g_signal_connect(G_OBJECT(draw_area), "scroll-event", G_CALLBACK(browser_scroll_event), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), draw_area, true, true, 0);

	browse_vscroll = gtk_vscrollbar_new(NULL);
	gtk_range_set_range(GTK_RANGE(browse_vscroll), 0.0, 1.0);
	g_signal_connect(G_OBJECT(browse_vscroll), "change-value", G_CALLBACK(browse_vscroll_change), draw_area);
	gtk_box_pack_start(GTK_BOX(hbox), browse_vscroll, false, false, 0);

	GtkWidget *vbox = gtk_vbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);

	// 'search' entry
	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 64, -1);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(browser_search_entry_changed), draw_area);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Search:"), false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry, false, false, 0);

	gtk_widget_grab_focus(draw_area);

	g_signal_connect(G_OBJECT(draw_area), "key_press_event", G_CALLBACK(browser_key_event), entry);

	return hbox;
}

string open_texture_browser(bool tex, bool flat, bool sprite, string init_tex, bool fullscreen)
{
	tex_names.clear();
	browsesprites.clear();
	selected_tex = init_tex;
	browse_sprites = sprite;

	if (!sprite)
	{
		tex_names.push_back("-");

		if (tex || mix_tex)
		{
			for (int a = 0; a < textures.size(); a++)
				tex_names.push_back(textures[a]->name);
		}

		if (flat || mix_tex)
		{
			for (int a = 0; a < flats.size(); a++)
				tex_names.push_back(flats[a]->name);
		}

		// Sort alphabetically
		if (browser_sort == 0)
			sort(tex_names.begin(), tex_names.end());

		// Sort by use
		else if (browser_sort == 1)
		{
			table_t used_textures;
			sort(tex_names.begin(), tex_names.end());

			// Add all textures to table
			for (DWORD t = 0; t < tex_names.size(); t++)
				used_textures.add(tex_names[t], 0);

			// Get texture use frequencies
			if (tex || mix_tex)
			{
				for (DWORD s = 0; s < map.n_sides; s++)
				{
					if (map.sides[s]->tex_lower != "-")
						used_textures.increment(map.sides[s]->tex_lower, 1);

					if (map.sides[s]->tex_upper != "-")
						used_textures.increment(map.sides[s]->tex_upper, 1);

					if (map.sides[s]->tex_middle != "-")
						used_textures.increment(map.sides[s]->tex_middle, 1);
				}
			}

			if (flat || mix_tex)
			{
				for (DWORD s = 0; s < map.n_sectors; s++)
				{
					if (map.sectors[s]->f_tex != "-")
						used_textures.increment(map.sectors[s]->f_tex, 1);

					if (map.sectors[s]->c_tex != "-")
						used_textures.increment(map.sectors[s]->c_tex, 1);
				}
			}

			// Sort textures by frequency
			used_textures.sort_by_values(false);

			// Re-populate browser texture list in frequency order
			tex_names.clear();
			for (int t = 0; t < used_textures.n_rows; t++)
				tex_names.push_back(used_textures.rows[t]->name);
		}
	}
	else
	{
		get_ttype_names(&tex_names);

		for (int a = 0; a < tex_names.size(); a++)
			browsesprites.push_back(get_thing_type_from_name(tex_names[a])->spritename);
	}

	browser_dialog = gtk_dialog_new_with_buttons("Textures",
												GTK_WINDOW(editor_window),
												GTK_DIALOG_MODAL,
												GTK_STOCK_OK,
												GTK_RESPONSE_ACCEPT,
												GTK_STOCK_CANCEL,
												GTK_RESPONSE_REJECT,
												NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(browser_dialog)->vbox), setup_texture_browser());
	gtk_window_set_position(GTK_WINDOW(browser_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(browser_dialog), 640, 480);
	gtk_widget_show_all(browser_dialog);

	if (fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(browser_dialog));

	string ret = init_tex;
	int response = gtk_dialog_run(GTK_DIALOG(browser_dialog));

	if (response == GTK_RESPONSE_ACCEPT)
		ret = selected_tex;

	gtk_widget_destroy(browser_dialog);
	gtk_window_present(GTK_WINDOW(editor_window));

	return ret;
}
