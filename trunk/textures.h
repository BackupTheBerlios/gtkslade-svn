
class Texture
{
private:
	WORD		width;
	WORD		height;
	BYTE		bpp;
	BYTE		*data;
	//GdkPixbuf	*pbuf;

public:
	string		name;

	Texture();
	~Texture();

	void setup(string name, BYTE bpp, int width, int height)
	{
		this->width = width;
		this->height = height;
		this->bpp = bpp;
		this->name = name;

		if (bpp == 8)
		{
			data = (BYTE*)malloc(width * height);
			memset(data, 0, width * height);
		}

		if (bpp == 32)
		{
			data = (BYTE*)malloc(width * height * 4);
			memset(data, 0, width * height * 4);
		}
	}

	void add_pixel(int x, int y, BYTE colour)
	{
		if (x < 0 || x >= width)
			return;

		if (y < 0 || y >= height)
			return;

		DWORD p = ((y * (width)) + x);

		data[p] = colour;
	}

	void add_pixel(int x, int y, rgba_t colour)
	{
		if (x < 0 || x >= width)
			return;

		if (y < 0 || y >= height)
			return;

		DWORD p = ((y * (width)) + x) * 4;

		data[p++] = colour.r;
		data[p++] = colour.g;
		data[p++] = colour.b;
		data[p++] = colour.a;
	}

	GdkPixbuf* get_pbuf();
	GdkPixbuf* get_pbuf_scale(float scale = 1.0f, GdkInterpType interp = GDK_INTERP_NEAREST);
	GdkPixbuf* get_pbuf_scale_fit(int w, int h, float scaling = 1.0f);
};

void load_textures();
Texture* get_texture(string name, int type = 0);
void init_textures();
void load_flats();
void load_sprites();
