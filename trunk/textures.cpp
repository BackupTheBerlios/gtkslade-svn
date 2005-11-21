
#include "main.h"
#include "textures.h"
#include "console.h"
//#include "console_window.h"
#include "misc.h"
#include "splash.h"

vector<Texture*>	textures;
vector<Texture*>	flats;
vector<Texture*>	sprites;
vector<Texture*>	edit_textures;
vector<string>		pnames;
rgba_t				palette[256];

Texture*			no_tex;
Texture*			blank_tex;

bool				allow_tex_load = false;
bool				mix_tex = false;

CVAR(Bool, cache_textures, false, CVAR_SAVE)
CVAR(Bool, allow_np2_tex, false, CVAR_SAVE)
CVAR(Int, tex_filter, 1, CVAR_SAVE)

static const WORD valid_dimensions[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

extern WadList wads;
extern vector<string> spritenames;

Texture::Texture()
{
	this->data = NULL;
	this->width = 0;
	this->height = 0;
	this->rwidth = 0;
	this->rheight = 0;
	this->bpp = 8;
	this->gl_tex_generated = false;
	this->gl_filter = tex_filter;
	this->t_x = 1.0f;
	this->t_y = 1.0f;
	//this->gl_id = 0;
}

Texture::~Texture()
{
	if (data)
		free(data);

	if (gl_tex_generated)
		glDeleteTextures(1, &gl_id);
}

void Texture::setup(string name, BYTE bpp, int width, int height, bool has_alpha, bool sprite)
{
		this->width = width;
		this->height = height;
		this->rwidth = width;
		this->rheight = height;
		this->bpp = bpp;
		this->name = name;
		this->has_alpha = has_alpha;
		this->sprite = sprite;

		/*
		if (data)
		{
			free(data);
			data = NULL;
		}
		*/

		if (bpp == 8)
		{
			data = (BYTE*)realloc(data, width * height);
			memset(data, 247, width * height);
		}

		if (bpp == 32)
		{
			data = (BYTE*)realloc(data, width * height * 4);
			memset(data, 0, width * height * 4);
		}
	}

// is_valid_dimension: Checks if a value is a power of 2
// -------------------------------------------------- >>
bool is_valid_dimension(WORD val)
{
	for (BYTE d = 0; d < 12; d++)
	{
		if (val == valid_dimensions[d])
			return true;
	}

	return false;
}

// up_dimension: Increases a dimension to be a power of two
// ----------------------------------------------------- >>
WORD up_dimension(WORD val)
{
	WORD ret = val;

	while (1)
	{
		ret++;

		for (BYTE d = 0; d < 12; d++)
		{
			if (ret == valid_dimensions[d])
				return ret;
		}
	}

	return 0;
}

void clear_textures(int type)
{
	if (type == 1)
	{
		for (int a = 0; a < textures.size(); a++)
			delete textures[a];

		textures.clear();
	}

	if (type == 2)
	{
		for (int a = 0; a < flats.size(); a++)
			delete flats[a];

		flats.clear();
	}

	if (type == 3)
	{
		for (int a = 0; a < sprites.size(); a++)
			delete sprites[a];

		sprites.clear();
	}
}

void destroy_pb(guchar *pixels, gpointer data)
{
	free(pixels);
}

GLuint Texture::get_gl_id()
{
	// If the opengl texture doesn't exist, create it
	if (!gl_tex_generated && allow_tex_load)
	{
		BYTE* temp = NULL;
		gl_tex_generated = true;

		if (bpp == 8)
		{
			temp = (BYTE*)malloc(width * height * 4);
			memset(temp, 0, width * height * 4);

			DWORD p = 0;
			for (int a = 0; a < width * height; a++)
			{
				temp[p++] = palette[data[a]].r;
				temp[p++] = palette[data[a]].g;
				temp[p++] = palette[data[a]].b;

				if (has_alpha)
					temp[p++] = palette[data[a]].a;
				else
					temp[p++] = 255;
			}
		}
		else if (bpp == 32)
		{
			temp = (BYTE*)malloc(rwidth * rheight * 4);
			memcpy(temp, data, rwidth * rheight * 4);
		}

		// Scale texture if it has invalid dimensions
		int nwidth = rwidth;
		int nheight = rheight;

		if (!allow_np2_tex)
		{
			if (!is_valid_dimension(rwidth))
				nwidth = up_dimension(rwidth);

			if (!is_valid_dimension(rheight))
				nheight = up_dimension(rheight);

			if (nwidth != rwidth || nheight != rheight)
			{
				// Create a pixbuf
				GdkPixbuf *pbuf = gdk_pixbuf_new_from_data(temp, GDK_COLORSPACE_RGB, true,
					8, rwidth, rheight, rwidth * 4,
					destroy_pb, NULL);

				// Create a new, resized pixbuf
				GdkPixbuf *pb2;
				
				if (!sprite)
					pb2 = gdk_pixbuf_scale_simple(pbuf, nwidth, nheight, GDK_INTERP_BILINEAR);
				else
				{
					pb2 = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, nwidth, nheight);
					gdk_pixbuf_fill(pb2, 0);
					gdk_pixbuf_copy_area(pbuf, 0, 0, width, height, pb2, 0, 0);
					t_x = (float)width / (float)nwidth;
					t_y = (float)height / (float)nheight;
				}
				g_object_unref(pbuf);
				temp = NULL;

				// Copy the new data
				temp = (BYTE*)malloc(nwidth * nheight * 4);
				memcpy(temp, gdk_pixbuf_get_pixels(pb2), nwidth * nheight * 4);

				// Delete the resized pixbuf
				g_object_unref(pb2);

				rwidth = nwidth;
				rheight = nheight;
			}
		}

		// Generate gl tex
		if (gl_filter == 1)
		{
			glGenTextures(1, &gl_id);
			glBindTexture(GL_TEXTURE_2D, gl_id);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rwidth, rheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		}

		if (gl_filter == 2)
		{
			glGenTextures(1, &gl_id);
			glBindTexture(GL_TEXTURE_2D, gl_id);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rwidth, rheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		}

		if (gl_filter == 3)
		{
			glGenTextures(1, &gl_id);
			glBindTexture(GL_TEXTURE_2D, gl_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, rwidth, rheight, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		}

		free(temp);
		free(data);
		data = NULL;
		//log_message("Gen ID: %d\n", gl_id);
	}

	return gl_id;
}

bool Texture::load_file(string name, string filename, int filter)
{
	GdkPixbuf *pbuf = gdk_pixbuf_new_from_file(filename.c_str(), NULL);

	if (!pbuf)
	{
		log_message("Error loading \"%s\"\n", filename.c_str());
		return false;
	}

	this->name = name;
	this->gl_filter = filter;
	width = gdk_pixbuf_get_width(pbuf);
	height = gdk_pixbuf_get_height(pbuf);
	rwidth = width;
	rheight = height;
	bpp = 32;

	log_message("%s: %dx%d\n", name.c_str(), width, height);

	free(data);
	data = (BYTE*)malloc(width * height * 4);
	//data = (BYTE *)realloc(data, width * height * 4);

	int size = gdk_pixbuf_get_rowstride(pbuf) * (height - 1);
	size += width * ((gdk_pixbuf_get_n_channels(pbuf) * gdk_pixbuf_get_bits_per_sample(pbuf) + 7) / 8);

	memset(data, 0, width * height * 4);
	memcpy(data, gdk_pixbuf_get_pixels(pbuf), size);
	//memset(data, 180, width * height * 4);

	g_object_unref(pbuf);

	return true;
}

/*
GdkPixbuf* Texture::get_pbuf()
{
	if (!pbuf)
	{
		if (bpp == 8)
		{
			BYTE* temp = (BYTE*)malloc(width * height * 4);
			memset(temp, 0, width * height * 4);

			DWORD p = 0;
			for (int a = 0; a < width * height; a++)
			{
				temp[p++] = palette[data[a]].r;
				temp[p++] = palette[data[a]].g;
				temp[p++] = palette[data[a]].b;

				if (has_alpha)
					temp[p++] = palette[data[a]].a;
				else
					temp[p++] = 255;
			}

			GdkPixbuf *ret = gdk_pixbuf_new_from_data(temp, GDK_COLORSPACE_RGB, true,
				8, width, height, width * 4,
				destroy_pb, NULL);

			//if (cache_textures)
			//	pbuf = gdk_pixbuf_copy(ret);

			return ret;
		}
		else if (bpp == 32)
		{
			BYTE* temp = (BYTE*)malloc(width * height * 4);
			memcpy(temp, data, width * height * 4);

			GdkPixbuf *ret = gdk_pixbuf_new_from_data(temp, GDK_COLORSPACE_RGB, true,
				8, width, height, width * 4,
				destroy_pb, NULL);

			//if (cache_textures)
			//	pbuf = gdk_pixbuf_copy(ret);

			return ret;
		}
	}
	else
		return gdk_pixbuf_copy(pbuf);

	return NULL;
}

GdkPixbuf* Texture::get_pbuf_scale(float scale, GdkInterpType interp)
{
	GdkPixbuf *original = get_pbuf();

	GdkPixbuf *ret = gdk_pixbuf_scale_simple(original, width * scale, height * scale, interp);
	g_object_unref(original);

	return ret;
}

GdkPixbuf* Texture::get_pbuf_scale_fit(int w, int h, float scaling, GdkInterpType interp)
{
	GdkPixbuf *original = get_pbuf_scale(scaling, interp);
	float scale;

	int swidth = width * scaling;
	int sheight = height * scaling;

	int n_width = swidth;
	int n_height = sheight;

	int d = min(w, h);

	if (swidth > sheight)
	{
		if (swidth > d)
		{
			scale = (float)w / (float)swidth;
			n_width = w;
			n_height = sheight * scale;
		}
	}
	else if (swidth < sheight)
	{
		if (sheight > d)
		{
			scale = (float)h / (float)sheight;
			n_height = h;
			n_width = swidth * scale;
		}
	}
	else if (swidth == sheight)
	{
		if (swidth > d)
		{
			n_width = d;
			n_height = d;
		}
	}

	GdkPixbuf *ret = gdk_pixbuf_scale_simple(original, n_width, n_height, interp);
	g_object_unref(original);
	return ret;
}
*/

Texture* search_textures(string name, int type, bool mix)
{
	

	return NULL;
}

Texture* search_flats(string name, int type, bool mix)
{
	

	return NULL;
}

Texture* get_texture(string name, int type, bool force)
{
	if (name == "-")
		return blank_tex;

	bool mix = mix_tex;
	if (force)
		mix = false;

	if (type == TEXTURES_WALLS || type == 0)
	{
		if (type == 0 || type == TEXTURES_WALLS || (type == TEXTURES_FLATS && mix))
		{
			for (int a = 0; a < textures.size(); a++)
			{
				if (textures[a]->name == name)
					return textures[a];
			}
		}

		if (type == 0 || type == TEXTURES_FLATS || (type == TEXTURES_WALLS && mix))
		{
			for (int a = 0; a < flats.size(); a++)
			{
				if (flats[a]->name == name)
					return flats[a];
			}
		}
	}


	if (type == TEXTURES_FLATS)
	{
		if (type == 0 || type == TEXTURES_FLATS || (type == TEXTURES_WALLS && mix))
		{
			for (int a = 0; a < flats.size(); a++)
			{
				if (flats[a]->name == name)
					return flats[a];
			}
		}

		if (type == 0 || type == TEXTURES_WALLS || (type == TEXTURES_FLATS && mix))
		{
			for (int a = 0; a < textures.size(); a++)
			{
				if (textures[a]->name == name)
					return textures[a];
			}
		}
	}

	// Search sprites
	if (type == 0 || type == TEXTURES_SPRITES)
	{
		for (int a = 0; a < sprites.size(); a++)
		{
			if (sprites[a]->name == name)
				return sprites[a];
		}
	}

	// Search editor textures
	for (int a = 0; a < edit_textures.size(); a++)
	{
		if (edit_textures[a]->name == name)
			return edit_textures[a];
	}

	return no_tex;
}

void load_editor_texture(string name, string filename, int width = -1, int height = -1, int filter = tex_filter)
{
	Texture* tex = new Texture();
	tex->load_file(name, filename, filter);
	edit_textures.push_back(tex);
	if (width >= 0)
		tex->width = width;
	if (height >= 0)
		tex->height = height;
}

void init_textures()
{
	no_tex = new Texture();
	no_tex->setup("_notex", 32, 128, 128);

	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			rgba_t col1, col2;
			col1.set(255, 0, 0, 255);
			col2.set(0, 0, 0, 255);

			if ((x / 8) % 2 == 0)
			{
				if ((y / 8) % 2 == 0)
					no_tex->add_pixel(x, y, col1);
				else
					no_tex->add_pixel(x, y, col2);
			}
			else
			{
				if ((y / 8) % 2 == 0)
					no_tex->add_pixel(x, y, col2);
				else
					no_tex->add_pixel(x, y, col1);
			}
		}
	}

	blank_tex = new Texture();
	blank_tex->setup("-", 32, 128, 128);

	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			rgba_t col1, col2;
			col1.set(255, 255, 255, 255);
			col2.set(0, 150, 190, 255);

			if ((x / 8) % 2 == 0)
			{
				if ((y / 8) % 2 == 0)
					blank_tex->add_pixel(x, y, col1);
				else
					blank_tex->add_pixel(x, y, col2);
			}
			else
			{
				if ((y / 8) % 2 == 0)
					blank_tex->add_pixel(x, y, col2);
				else
					blank_tex->add_pixel(x, y, col1);
			}
		}
	}

	load_editor_texture("_font", "res/font.png");
	load_editor_texture("_thing", "res/thing.png", -1, -1, 2);
	load_editor_texture("_unknownsprite", "res/no_thing.png", 32, 32);
	load_editor_texture("_thing_sound", "res/thing_sound.png", 32, 32);
	load_editor_texture("_thing_spot", "res/thing_spot.png", 32, 32);
	load_editor_texture("_thing_light", "res/thing_light.png", 32, 32);
	load_editor_texture("_thing_fountain", "res/thing_fountain.png", 32, 32);
	load_editor_texture("_thing_slope", "res/thing_slope.png", 32, 32);
	load_editor_texture("_xhair", "res/xhair.png", 32, 32);
}

// read_palette: Reads the palette from a wad
// --------------------------------------- >>
void read_palette(Wad* wad)
{
	Lump *pal = wad->get_lump("PLAYPAL", 0);
	FILE *fp = fopen(wad->path.c_str(), "rb");

	fseek(fp, pal->Offset(), SEEK_SET);
	for (DWORD c = 0; c < 256; c++)
	{
		fread(&palette[c].r, 1, 1, fp);
		fread(&palette[c].g, 1, 1, fp);
		fread(&palette[c].b, 1, 1, fp);
		palette[c].a = 255;
	}
	fclose(fp);

	palette[247].a = 0;
}

// load_pnames: Loads the pnames lump from a wad
// ------------------------------------------ >>
void load_pnames(Wad* wad)
{
	DWORD n_pnames = 0;
	FILE *fp = fopen(wad->path.c_str(), "rb");
	Lump *lump = wad->get_lump("PNAMES", 0);

	fseek(fp, lump->Offset(), SEEK_SET);
	fread(&n_pnames, 4, 1, fp);

	for (DWORD p = 0; p < n_pnames; p++)
	{
		char name[9] = "";

		fread(name, 1, 8, fp);
		name[8] = 0;

		pnames.push_back(name);
	}

	fclose(fp);
}

// add_patch_to_tex: Adds patch data to a texture
// ------------------------------------------- >>
void add_patch_to_tex(int x, int y, Wad* wad, Lump *patch, Texture *tex)
{
	patch_header_t	header;
	long			*columns = NULL;
	BYTE			row = 0;
	BYTE			n_pix = 0;
	BYTE			colour = 0;

	FILE *fp = fopen(wad->path.c_str(), "rb");
	fseek(fp, patch->Offset(), SEEK_SET);

	// Get header & offsets
	//fread(header, sizeof(patch_header_t), 1, fp);
	fread(&header.width, 2, 1, fp);
	fread(&header.height, 2, 1, fp);
	fread(&header.left, 2, 1, fp);
	fread(&header.top, 2, 1, fp);
	columns = (long *)calloc(header.width, sizeof(long));
	fread(columns, sizeof(long), header.width, fp);

	// Read data
	for (int c = 0; c < header.width; c++)
	{
		// Go to start of column
		fseek(fp, patch->Offset(), SEEK_SET);
		fseek(fp, columns[c], SEEK_CUR);

		// Read posts
		while (1)
		{
			// Get row offset
			fread(&row, 1, 1, fp);

			if (row == 255) // End of column?
				break;
			
			// Get no. of pixels
			fread(&n_pix, 1, 1, fp);

			// Read pixels
			fread(&colour, 1, 1, fp); // Skip buffer
			for (BYTE p = 0; p < n_pix; p++)
			{
				fread(&colour, 1, 1, fp);
				tex->add_pixel(x + c, y + row + p, colour);
			}
			fread(&colour, 1, 1, fp); // Skip buffer & go to next row offset
		}
	}

	fclose(fp);
}

// load_textures_lump: Loads all textures from a TEXTUREx lump
// -------------------------------------------------------- >>
void load_textures_lump(Wad* wad, Lump *lump)
{
	long n_tex = 0;
	long *offsets = NULL;
	long temp = 0;

	short width = 0;
	short height = 0;
	short patches = 0;

	short xoff = 0;
	short yoff = 0;
	short patch = 0;

	FILE *fp = fopen(wad->path.c_str(), "rb");
	fseek(fp, lump->Offset(), SEEK_SET);

	// Get no. of textures and tex info offsets
	fread(&n_tex, 4, 1, fp);
	offsets = (long *)calloc(n_tex, 4);
	fread(offsets, 4, n_tex, fp);

	//show_progress();

	for (int t = 0; t < n_tex; t++)
	{
		double prog = (double)t / (double)n_tex;

		int inter = (n_tex / 20);
		if (inter == 0) inter = 1;
		if (t % inter == 0)
			splash_progress(prog);
			//update_progress(prog);

		// Go to start of tex definition
		fseek(fp, lump->Offset(), SEEK_SET);
		fseek(fp, offsets[t], SEEK_CUR);

		// Read texture name
		char texname[9] = "";
		fread(texname, 1, 8, fp);
		texname[8] = 0;

		// Skip unused stuff
		fread(&temp, 4, 1, fp);

		// Read width & height
		fread(&width, 2, 1, fp);
		fread(&height, 2, 1, fp);

		// Add texture
		Texture *tex = get_texture(texname, 1, true);
		if (tex->name == "_notex")
		{
			tex = new Texture();
			tex->setup(texname, 8, width, height);
			textures.push_back(tex);
		}

		// Skip more unused stuff
		fread(&temp, 4, 1, fp);

		// Read no. of patches in texture
		fread(&patches, 2, 1, fp);

		// Add patches
		for (int p = 0; p < patches; p++)
		{
			// Read patch info
			fread(&xoff, 2, 1, fp);
			fread(&yoff, 2, 1, fp);
			fread(&patch, 2, 1, fp);

			// Skip unused
			fread(&temp, 4, 1, fp);

			Wad* iwad = wads.get_iwad();
			Lump *patchlump = NULL;

			// Well, patches _should_ be within P(P)_START and P(P)_END, but some popular wads leave these markers out
			// so I have it like this for compatibility reasons (and doom/2.exe doesn't seem to mind missing markers)
			if (wad->patches[START] == -1)
				patchlump = wad->get_lump(pnames[patch], 0);
			else
				patchlump = wad->get_lump(pnames[patch], wad->patches[START]);

			if (patchlump)
				add_patch_to_tex(xoff, yoff, wad, patchlump, tex);
			else
			{
				patchlump = iwad->get_lump(pnames[patch], iwad->patches[START]);

				if (patchlump)
					add_patch_to_tex(xoff, yoff, iwad, patchlump, tex);
				else
				{
					printf("Patch \"%s\" doesn't exist!\n", pnames[patch].c_str());
					break;
				}
			}
		}
	}

	//hide_progress();
}

// load_textures: Loads all textures from the last wad opened containing a TEXTUREx lump
// (or the IWAD if no open wads have TEXTUREx lumps...)
// ---------------------------------------------------------------------------------- >>
void load_textures()
{
	clear_textures(1);

	// Read the palette
	read_palette(wads.get_wad_with_lump("PLAYPAL"));

	// Get the wad with TEXTURE1
	Wad* tex_wad = wads.get_wad_with_lump("TEXTURE1");

	// Console stuff
	console_print("Loading textures...");
	//wait_gtk_events();

	splash("Loading Textures");

	// Load PNAMES
	load_pnames(tex_wad);

	// Load TEXTURE1
	load_textures_lump(tex_wad, tex_wad->get_lump("TEXTURE1", 0));

	// Load TEXTURE2 if it exists
	if (tex_wad->get_lump("TEXTURE2", 0))
		load_textures_lump(tex_wad, tex_wad->get_lump("TEXTURE2", 0));

	console_print("Done");

	/*
	Wad* htexwad = wads.get_wad_with_lump("HIRESTEX");

	if (htexwad)
		load_zgl_hirestex(htexwad, htexwad->get_lump("HIRESTEX", 0));
		*/
}

// load_flats_wad: Loads all flats from a wad
// --------------------------------------- >>
void load_flats_wad(Wad* wad)
{
	int lump = wad->flats[START];
	FILE *fp = fopen(wad->path.c_str(), "rb");

	lump++;

	while (lump < wad->flats[END])
	{
		if (wad->directory[lump]->Size() > 0)
		{
			BYTE p = 0;

			// Create the texture
			Texture *tex = get_texture(wad->directory[lump]->Name(), 2, true);
			if (tex->name == "_notex")
			{
				tex = new Texture();
				tex->setup(wad->directory[lump]->Name(), 8, 64, 64, false);
				flats.push_back(tex);
			}

			// Read flat data
			fseek(fp, wad->directory[lump]->Offset(), SEEK_SET);

			for (BYTE y = 0; y < 64; y++)
			{
				for (BYTE x = 0; x < 64; x++)
				{
					fread(&p, 1, 1, fp);
					tex->add_pixel(x, y, p);
				}
			}
		}

		lump++;
	}
}

// load_flats: Loads flats from all open wads (that have any flats in them)
// --------------------------------------------------------------------- >>
void load_flats()
{
	clear_textures(2);
	console_print("Loading flats...");
	//wait_gtk_events();
	splash("Loading Flats");

	// Load IWAD flats first
	load_flats_wad(wads.get_iwad());

	// Load flats from all open pwads
	if (wads.n_wads == 0)
		return;

	for (DWORD w = 0; w < wads.n_wads; w++)
	{
		if (wads.get_wad(w)->flats[START] != -1)
			load_flats_wad(wads.get_wad(w));
	}

	console_print("Done");
}

// load_sprite: Loads a sprite to a texture
// ------------------------------------- >>
void load_sprite(Wad* wad, string name)
{
	patch_header_t	header;
	long			*columns = NULL;
	BYTE			row = 0;
	BYTE			n_pix = 0;
	BYTE			colour = 0;
	Lump*			patch = NULL;

	patch = wad->get_lump(name, wad->sprites[START]);

	if (!patch)
	{
		printf("Sprite %s not found!\n", name.c_str());
		return;
	}

	FILE *fp = fopen(wad->path.c_str(), "rb");
	fseek(fp, patch->Offset(), SEEK_SET);

	// Get header & offsets
	fread(&header.width, 2, 1, fp);
	fread(&header.height, 2, 1, fp);
	fread(&header.left, 2, 1, fp);
	fread(&header.top, 2, 1, fp);
	columns = (long *)calloc(header.width, sizeof(long));
	fread(columns, sizeof(long), header.width, fp);

	// Create the texture
	Texture *tex = get_texture(patch->Name(), 3);
	if (tex->name == "_notex")
	{
		tex = new Texture();
		tex->setup(patch->Name(), 8, header.width, header.height, true, true);
		sprites.push_back(tex);
	}

	// Read data
	for (int c = 0; c < header.width; c++)
	{
		// Go to start of column
		fseek(fp, patch->Offset(), SEEK_SET);
		fseek(fp, columns[c], SEEK_CUR);

		// Read posts
		while (1)
		{
			// Get row offset
			fread(&row, 1, 1, fp);

			if (row == 255) // End of column?
				break;
			
			// Get no. of pixels
			fread(&n_pix, 1, 1, fp);

			// Read pixels
			fread(&colour, 1, 1, fp); // Skip buffer
			for (BYTE p = 0; p < n_pix; p++)
			{
				fread(&colour, 1, 1, fp);
				tex->add_pixel(c, row + p, colour);
			}
			fread(&colour, 1, 1, fp); // Skip buffer & go to next row offset
		}
	}

	fclose(fp);
}

// load_sprites: Loads sprites for thing previews
// ------------------------------------------- >>
void load_sprites()
{
	clear_textures(3);
	console_print("Loading sprites...");
	splash("Loading Sprites");
	//show_progress();

	for (int s = 0; s < spritenames.size(); s++)
	{
		int inter = (spritenames.size() / 20);
		if (inter == 0) inter = 1;

		if (s % inter == 0)
			splash_progress((float)s / (float)spritenames.size());
			//update_progress((float)s / (float)spritenames.size());

		Wad* wad = wads.get_wad_with_lump(spritenames[s]);

		if (wad)
			load_sprite(wad, spritenames[s]);
	}

	//hide_progress();
	console_print("Done");
}

void load_tx_texture(Wad* wad, long index)
{
	// Get lump
	Lump* lump = wad->directory[index];
	BYTE* data = lump->Data();

	// Check for PNG header
	if (data[0] == 137 && data[1] == 80 &&
		data[2] == 78 && data[3] == 71 &&
		data[4] == 13 && data[5] == 10 &&
		data[6] == 26 && data[7] == 10)
	{
		// Write lump content to file
		FILE *fp = fopen("sladetemp", "wb");
		fwrite(lump->Data(), lump->Size(), 1, fp);
		fclose(fp);

		// If texture doesn't exist, create it
		Texture* tex = get_texture(lump->Name(), 1);
		if (tex->name == "_notex")
		{
			tex = new Texture();
			textures.push_back(tex);
		}

		// Read file to texture
		tex->load_file(lump->Name(), "sladetemp", tex_filter);
		
		remove("sladetemp");
	}
	else // Normal doom format patch
	{
		short *p = (short *)lump->Data();
		short width = *p++;
		short height = *p;

		if (width < 0 || height < 0)
			return;

		Texture* tex = get_texture(lump->Name(), 1);
		if (tex->name == "_notex")
		{
			tex = new Texture();
			textures.push_back(tex);
		}

		tex->setup(lump->Name(), 8, width, height, true);
		add_patch_to_tex(0, 0, wad, lump, tex);
	}
}

void load_tx_textures()
{
	splash("Loading TX_ Textures");

	for (DWORD w = 0; w < wads.n_wads; w++)
	{
		if (wads.get_wad(w)->tx[START] != -1)
		{
			Wad* wad = wads.get_wad(w);
			bool done = false;
			long lump = wad->tx[START] + 1;

			int end = wad->tx[END];
			if (end == -1)
				end = wad->num_lumps;
			int inter = (end - wad->tx[START]) / 20;
			if (inter == 0) inter = 1;

			while (!done)
			{
				if ((lump - wad->tx[START]) % inter == 0)
				{
					double prog = float(lump - wad->tx[START]) / float(end - wad->tx[START]);
					splash_progress(prog);
				}

				load_tx_texture(wad, lump);
				lump++;

				if (lump == wad->tx[END] || lump == wad->num_lumps)
					done = true;
			}
		}
	}
}
