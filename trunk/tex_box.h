
/*
struct tbox_data_t
{
	string	texture;
	BYTE	textype;
	float	scale;

	tbox_data_t(string texture = "_notex", BYTE textype = 0, float scale = 1.0f)
	{
		this->texture = texture;
		this->textype = textype;
		this->scale = scale;
	}
};
*/

struct tex_box_t
{
	string		texture;
	BYTE		textype;
	float		scale;
	GtkWidget	*widget;
	rgba_t		back_colour;

	tex_box_t(string texture = "_notex", BYTE textype = 0, float scale = 1.0f, rgba_t back_colour = rgba_t(0, 0, 0, 255, 0));
	~tex_box_t();

	void setup_widget();
	void change_texture(string newtex, BYTE newtype = 0, float newscale = 1.0f, bool redraw = true);
	void set_size(int width, int height) { gtk_widget_set_size_request(widget, width, height); }
};

//GtkWidget *tex_box_new(int width, int height, tbox_data_t tdata);
