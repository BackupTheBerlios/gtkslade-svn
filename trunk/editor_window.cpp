
#include "main.h"
#include "map.h"
#include "draw.h"
#include "edit.h"
#include "editor_window.h"
#include "keybind.h"
#include "input.h"
#include "linedraw.h"

GtkWidget	*editor_window = NULL;
GtkWidget	*map_area = NULL;
GdkPixmap	*pixmap = NULL;

rect_t	sel_box(-1, -1, 0, 0, 0);
point2_t mouse;

int vid_width;
int vid_height;

extern Map map;
extern int hilight_item;
extern BindList binds;
extern bool line_draw;

// init_opengl: Initialises OpenGL
// ---------------------------- >>
bool init_opengl()
{
	glViewport(0, 0, vid_width, vid_height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, vid_width, vid_height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

// realize_main: Called when map_area is realized
// ------------------------------------------- >>
static void realize_main(GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();

	gdk_gl_drawable_gl_end (gldrawable);
}

/*
// realize_sub: Called when other context-sharing gl widgets are realized
// ------------------------------------------------------------------- >>
static void realize_sub(GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();

	gdk_gl_drawable_gl_end(gldrawable);
}
*/

// expose_event: Called when a part of the map_area needs to be redrawn
// ----------------------------------------------------------------- >>
gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return FALSE;

	draw_map();

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end (gldrawable);

	return false;
}

// force_map_redraw: Forces the map area to be redrawn
// ------------------------------------------------ >>
void force_map_redraw(bool map, bool grid)
{
	if (grid)
		update_grid();

	if (map)
		update_map();

	gdk_window_invalidate_rect(map_area->window, &map_area->allocation, FALSE);
}

// configure_event: Called when the map area is resized or initialised
// ---------------------------------------------------------------- >>
gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return FALSE;

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);
	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();

	gdk_gl_drawable_gl_end (gldrawable);

	return true;
}

// motion_notify_event: When the mouse pointer is moved over the map area
// ------------------------------------------------------------------- >>
static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	bool redraw_map = false;
	bool update_map = false;
	GdkModifierType state;
	
	if (event->is_hint)
	{
		gdk_window_get_pointer(event->window, &x, &y, &state);
	}
	else
	{
		x = event->x;
		y = event->y;
		state = (GdkModifierType)(event->state);
	}

	//if (state & GDK_BUTTON1_MASK && pixmap != NULL)
		//draw_brush (widget, x, y);

	mouse.set(x, y);

	if (state & GDK_BUTTON3_MASK)
	{
		move_items();
		redraw_map = true;
		update_map = true;
	}

	if (sel_box.x1() != -1)
	{
		sel_box.br.set(x, y);

		if (line_draw)
			line_drawbox();

		redraw_map = true;
	}
	else
	{
		if (line_draw)
			force_map_redraw();
		else
		{
			int old_hilight = hilight_item;
			get_hilight_item(x, y);

			if (hilight_item != old_hilight)
				redraw_map = true;
		}
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// button_press_event: When a mouse button is pressed in the map area
// --------------------------------------------------------------- >>
static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	bool redraw_map = false;
	bool update_map = false;

	if (event->button == 1)
	{
		if (event->state & GDK_SHIFT_MASK)
			sel_box.set(event->x, event->y, event->x, event->y);
		else
		{
			if (line_draw)
			{
				line_drawpoint();
				redraw_map = true;
			}
			else
			{
				select_item();
				redraw_map = true;
				update_map = true;
			}
		}
	}

	if (event->button == 3)
	{
		if (line_draw)
		{
			line_undrawpoint();
			redraw_map = true;
		}
		else
		{
			add_move_items();
			redraw_map = update_map = true;
		}
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// button_release_event: When a mouse button is released in the map area
// ------------------------------------------------------------------ >>
static gboolean button_release_event(GtkWidget *widget, GdkEventButton *event)
{
	bool redraw_map = false;
	bool update_map = false;

	if (event->button == 1)
	{
		if (sel_box.x1() != -1)
		{
			select_items_box(sel_box);
			sel_box.set(-1, -1, -1, -1);
			redraw_map = update_map = true;
		}
	}

	if (event->button == 3)
	{
		if (!line_draw)
		{
			clear_move_items();
			redraw_map = update_map = true;
		}
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// key_press_event: When a keyboard key is pressed
// -------------------------------------------- >>
gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	//printf("\"%s\"\n", key.c_str());
	binds.set(key);

	keys_edit();

	return false;
}

// key_release_event: When a keyboard key is released
// ----------------------------------------------- >>
gboolean key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	//printf("\"%s\"\n", key.c_str());
	binds.unset(key);

	return false;
}

// setup_editor_window: Sets up the main editor window
// ------------------------------------------------ >>
void setup_editor_window()
{
	GdkGLConfig *glconfig;
	GdkGLContext *glcontext;

	// Setup OpenGL
	glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB|GDK_GL_MODE_DEPTH|GDK_GL_MODE_DOUBLE));

	if (glconfig == NULL)
	{
		glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB|GDK_GL_MODE_DEPTH));

		if (glconfig == NULL)
			exit(1);
	}

	editor_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(editor_window, 640, 480);
	gtk_widget_set_double_buffered(editor_window, true);

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(editor_window), main_vbox);

	// Menu
	GtkWidget *temp = gtk_label_new("Menu goes here");
	gtk_box_pack_start(GTK_BOX(main_vbox), temp, false, false, 0);

	// Map area
	map_area = gtk_drawing_area_new();

	gtk_widget_set_gl_capability(map_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

	gtk_widget_set_events(map_area, GDK_EXPOSURE_MASK
									| GDK_LEAVE_NOTIFY_MASK
									| GDK_BUTTON_PRESS_MASK
									| GDK_BUTTON_RELEASE_MASK
									| GDK_POINTER_MOTION_MASK
									| GDK_POINTER_MOTION_HINT_MASK);

	g_signal_connect(G_OBJECT(map_area), "expose-event", G_CALLBACK(expose_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "configure-event", G_CALLBACK(configure_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "button_press_event", G_CALLBACK(button_press_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "button_release_event", G_CALLBACK(button_release_event), NULL);
	g_signal_connect(G_OBJECT(editor_window), "key-press-event", G_CALLBACK(key_press_event), NULL);
	g_signal_connect(G_OBJECT(editor_window), "key-release-event", G_CALLBACK(key_release_event), NULL);
	g_signal_connect_after(G_OBJECT(map_area), "realize", G_CALLBACK (realize_main), NULL);
	gtk_box_pack_start(GTK_BOX(main_vbox), map_area, true, true, 0);

	// Setup widgets that need to share the context
	gtk_widget_realize(map_area);
	glcontext = gtk_widget_get_gl_context(map_area);

	// Info area
	temp = gtk_label_new("Info here");
	gtk_widget_set_size_request(temp, -1, 128);
	gtk_box_pack_start(GTK_BOX(main_vbox), temp, false, false, 0);

	gtk_widget_show_all(editor_window);
}
