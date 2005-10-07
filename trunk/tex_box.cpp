
#include "main.h"
#include "textures.h"
#include "tex_box.h"

//vector<tex_box_t*> tboxes;

void redraw_tbox(tex_box_t* tbox)
{
	GtkWidget *w = tbox->widget;

	// Fill background
	GdkGC* gc = gdk_gc_new(w->window);
	gdk_gc_set_rgb_fg_color(gc, &tbox->back_colour.to_gdk_color());
	gdk_draw_rectangle(w->window, gc, true, 0, 0, w->allocation.width, w->allocation.height);
	g_object_unref(gc);

	if (tbox->texture == "")
		return;

	// Draw texture
	Texture *tex = get_texture(tbox->texture, tbox->textype);
	GdkPixbuf *pbuf = tex->get_pbuf_scale_fit(w->allocation.width, w->allocation.height, tbox->scale);

	int x_off = 0;
	int y_off = 0;

	if (w->allocation.width > gdk_pixbuf_get_width(pbuf))
	{
		int diff = difference(w->allocation.width, gdk_pixbuf_get_width(pbuf));
		x_off = diff / 2;
	}

	if (w->allocation.height > gdk_pixbuf_get_height(pbuf))
	{
		int diff = difference(w->allocation.height, gdk_pixbuf_get_height(pbuf));
		y_off = diff / 2;
	}

	gdk_draw_pixbuf(w->window, w->style->fg_gc[GTK_WIDGET_STATE(w)],
					pbuf,
					0, 0,
					x_off, y_off,
					-1, -1,
					GDK_RGB_DITHER_NONE,
					0, 0);

	g_object_unref(pbuf);
}

gboolean tbox_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
	tex_box_t* tbox = (tex_box_t*)data;

	redraw_tbox(tbox);

	return true;
}

tex_box_t::tex_box_t(string texture, BYTE textype, float scale, rgba_t back_colour)
{
	this->texture = texture;
	this->textype = textype;
	this->scale = scale;
	this->back_colour = back_colour;

	widget = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(tbox_expose_event), (gpointer)this);
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
		redraw_tbox(this);
}

tex_box_t::~tex_box_t()
{
	//gtk_widget_destroy(widget);
}
