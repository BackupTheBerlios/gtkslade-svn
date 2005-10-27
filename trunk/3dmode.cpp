
#include "main.h"
#include "input.h"
#include "keybind.h"
#include "render.h"

extern GtkWidget *editor_window;
extern GdkGLConfig *glconfig;
extern GdkGLContext *glcontext;

extern BindList binds;

gboolean window3d_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
	GdkGLContext *context = gtk_widget_get_gl_context(w);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(w);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return false;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_3d_view();

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);

	return false;
}

void window3d_realize_event(GtkWidget *widget, gpointer data)
{
	GdkGLContext *context = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return;

	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0, 1);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 2.0f);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 40.0f);
	glEnable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gdk_gl_drawable_gl_end(gldrawable);
}

gboolean window3d_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkGLContext *context = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return FALSE;

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gdk_gl_drawable_gl_end(gldrawable);

	return true;
}

// key_press_event: When a keyboard key is pressed
// -------------------------------------------- >>
gboolean key_3d_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	binds.set(key);

	if (!keys_3d())
		gtk_widget_destroy(GTK_WIDGET(data));

	return false;
}

// key_release_event: When a keyboard key is released
// ----------------------------------------------- >>
gboolean key_3d_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	binds.unset(key);

	return false;
}

void start_3d_mode()
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(window), true);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(editor_window));
	gtk_window_fullscreen(GTK_WINDOW(window));

	GtkWidget *draw_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), draw_area);
	gtk_widget_set_gl_capability(draw_area, glconfig, glcontext, true, GDK_GL_RGBA_TYPE);

	gtk_widget_add_events(draw_area, GDK_EXPOSURE_MASK
									| GDK_LEAVE_NOTIFY_MASK
									| GDK_BUTTON_PRESS_MASK
									| GDK_BUTTON_RELEASE_MASK
									| GDK_KEY_PRESS_MASK
									| GDK_KEY_RELEASE_MASK
									| GDK_POINTER_MOTION_MASK
									| GDK_POINTER_MOTION_HINT_MASK);

	GTK_WIDGET_SET_FLAGS(draw_area, GTK_CAN_FOCUS);

	g_signal_connect(G_OBJECT(draw_area), "expose-event", G_CALLBACK(window3d_expose_event), NULL);
	g_signal_connect(G_OBJECT(draw_area), "configure-event", G_CALLBACK(window3d_configure_event), NULL);
	g_signal_connect_after(G_OBJECT(draw_area), "realize", G_CALLBACK(window3d_realize_event), NULL);
	g_signal_connect(G_OBJECT(draw_area), "key_press_event", G_CALLBACK(key_3d_press_event), window);
	g_signal_connect(G_OBJECT(draw_area), "key_release_event", G_CALLBACK(key_3d_release_event), NULL);

	gtk_widget_show_all(window);
	gtk_widget_grab_focus(draw_area);
}
