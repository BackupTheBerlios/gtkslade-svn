
class Texture
{
private:
	BYTE		bpp;
	BYTE		*data;
	bool		has_alpha;
	bool		gl_tex_generated;
	int			gl_filter;
	GLuint		gl_id;
	WORD		rwidth;
	WORD		rheight;
	bool		sprite;

public:
	WORD		width;
	WORD		height;
	string		name;
	float		t_x, t_y;

	Texture();
	~Texture();

	void setup(string name, BYTE bpp, int width, int height, bool has_alpha = true, bool sprite = false);

	bool load_file(string name, string filename, int filter = 2);

	void add_pixel(int x, int y, BYTE colour)
	{
		if (x < 0 || x >= rwidth)
			return;

		if (y < 0 || y >= rheight)
			return;

		DWORD p = (y * (rwidth)) + x;

		data[p] = colour;
	}

	void add_pixel(int x, int y, rgba_t colour)
	{
		if (x < 0 || x >= rwidth)
			return;

		if (y < 0 || y >= rheight)
			return;

		DWORD p = ((y * (rwidth)) + x) * 4;

		data[p++] = colour.r;
		data[p++] = colour.g;
		data[p++] = colour.b;
		data[p++] = colour.a;
	}

	GLuint get_gl_id();
	//GdkPixbuf* get_pbuf();
	//GdkPixbuf* get_pbuf_scale(float scale = 1.0f, GdkInterpType interp = GDK_INTERP_BILINEAR);
	//GdkPixbuf* get_pbuf_scale_fit(int w, int h, float scaling = 1.0f, GdkInterpType interp = GDK_INTERP_BILINEAR);
};

void load_textures();
Texture* get_texture(string name, int type = 0, bool force = false);
void init_textures();
void load_flats();
void load_sprites();
void load_tx_textures();
void load_hirestex_textures();

#define TEXTURES_WALLS		1
#define	TEXTURES_FLATS		2
#define TEXTURES_SPRITES	3
#define TEXTURES_EDITOR		4

