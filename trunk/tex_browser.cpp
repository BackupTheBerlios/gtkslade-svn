
#include "main.h"
#include "textures.h"
#include "tex_box.h"
#include "thing_type.h"
#include "draw.h"

bool browse_sprites = false;
string selected_tex = "";
int rows = 0;
vector<string> tex_names;
vector<string> browsesprites;
GtkWidget *browse_vscroll;

CVAR(Int, browser_columns, 4, CVAR_SAVE)

extern GtkWidget *editor_window;
extern vector<Texture*> textures;
extern vector<Texture*> flats;
extern vector<Texture*> sprites;
extern GdkGLConfig *glconfig;
extern GdkGLContext *glcontext;

extern rgba_t col_selbox;
extern rgba_t col_selbox_line;

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
		int row = (gtk_range_get_value(GTK_RANGE(browse_vscroll)) + event->y) / width;
		int col = event->x / width;
		int index = (row * browser_columns) + col;
		selected_tex = tex_names[index];
		gdk_window_invalidate_rect(widget->window, &widget->allocation, false);
	}

	return true;
}

GtkWidget* setup_texture_browser()
{
	GtkWidget *hbox = gtk_hbox_new(false, 0);

	GtkWidget *draw_area = gtk_drawing_area_new();
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

	return hbox;
}

string open_texture_browser(bool tex, bool flat, bool sprite, string init_tex)
{
	tex_names.clear();
	browsesprites.clear();
	selected_tex = init_tex;
	browse_sprites = sprite;

	if (!sprite)
	{
		tex_names.push_back("-");

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
	}
	else
	{
		get_ttype_names(&tex_names);

		for (int a = 0; a < tex_names.size(); a++)
			browsesprites.push_back(get_thing_type_from_name(tex_names[a])->spritename);
	}

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Textures",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_texture_browser());
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 480);
	gtk_widget_show_all(dialog);

	string ret = init_tex;
	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
		ret = selected_tex;

	gtk_widget_destroy(dialog);
	gtk_window_present(GTK_WINDOW(editor_window));

	return ret;
}
