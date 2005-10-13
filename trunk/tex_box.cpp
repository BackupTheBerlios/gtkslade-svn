
#include "main.h"
#include "textures.h"
#include "tex_box.h"
#include "draw.h"

//vector<tex_box_t*> tboxes;

extern GdkGLConfig *glconfig;
extern GdkGLContext *glcontext;

void refresh_tbox(tex_box_t* tbox, int x = 0, int y = 0, int width = -1, int height = -1)
{
	gdk_draw_drawable(tbox->widget->window, tbox->widget->style->fg_gc[GTK_WIDGET_STATE(tbox->widget)],
						tbox->pixmap,
						x, y,
						x, y,
						width, height);
}

void redraw_tbox(tex_box_t* tbox)
{
	GtkWidget *w = GTK_WIDGET(tbox->widget);

	GdkGLContext *context = gtk_widget_get_gl_context(w);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(w);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (tbox->texture != "")
	{
		draw_texture_scale(rect_t(0, 0, w->allocation.width, w->allocation.height, 0),
							tbox->texture, tbox->textype,
							rgba_t(255, 255, 255, 255), tbox->scale);
	}

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);
}

gboolean tbox_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
	tex_box_t* tbox = (tex_box_t*)data;

	/*
	GdkGLContext *context = gtk_widget_get_gl_context(w);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(w);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return FALSE;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (tbox->texture != "")
		draw_texture_scale(rect_t(0, 0, w->allocation.width, w->allocation.height, 0), tbox->texture, tbox->textype);

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);
	*/
	redraw_tbox(tbox);

	return false;
}

gboolean tbox_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	tex_box_t* tbox = (tex_box_t*)data;

	GdkGLContext *context = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, context))
		return FALSE;

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);
	glClearColor(tbox->back_colour.fr(), tbox->back_colour.fg(), tbox->back_colour.fb(), 0.0f);
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

	glOrtho(0.0f, widget->allocation.width, widget->allocation.height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gdk_gl_drawable_gl_end (gldrawable);

	return true;
}

tex_box_t::tex_box_t(string texture, BYTE textype, float scale, rgba_t back_colour)
{
	this->texture = texture;
	this->textype = textype;
	this->scale = scale;
	this->back_colour = back_colour;
	this->pixmap = NULL;

	widget = gtk_drawing_area_new();
	gtk_widget_set_gl_capability(widget, glconfig, glcontext, TRUE, GDK_GL_RGBA_TYPE);
	g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(tbox_expose_event), (gpointer)this);
	g_signal_connect(G_OBJECT(widget), "configure-event", G_CALLBACK(tbox_configure_event), (gpointer)this);
}

void tex_box_t::setup_widget()
{
	//g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(tbox_expose_event), (gpointer)this);
}

void tex_box_t::change_texture(string newtex, BYTE newtype, float newscale, bool redraw)
{
	if (newtex == texture && newtype == textype && newscale == scale)
		return;

	texture = newtex;
	textype = newtype;
	scale = newscale;

	GdkRectangle rect;
	rect.width = widget->allocation.width;
	rect.height = widget->allocation.height;
	rect.x = widget->allocation.x;
	rect.y = widget->allocation.y;

	if (redraw)
	{
		redraw_tbox(this);
		refresh_tbox(this);
	}
}

tex_box_t::~tex_box_t()
{
	//gtk_widget_destroy(widget);
	g_object_unref(pixmap);
}
