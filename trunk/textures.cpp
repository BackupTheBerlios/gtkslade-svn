
#include "main.h"
#include "textures.h"
#include "console.h"
#include "console_window.h"
#include "misc.h"

vector<Texture*>	textures;
vector<Texture*>	edit_textures;
vector<string>		pnames;
rgba_t				palette[256];

Texture				no_tex;

extern WadList wads;

Texture::Texture()
{
	this->data = NULL;
	this->width = 0;
	this->height = 0;
	this->bpp = 8;
	this->pbuf = NULL;
}

Texture::~Texture()
{
	if (data)
		free(data);

	if (pbuf)
		free(pbuf);
}

void destroy_pb(guchar *pixels, gpointer data)
{
	free(pixels);
}

GdkPixbuf* Texture::get_pbuf()
{
	if (bpp == 8)
	{
		BYTE* temp = (BYTE*)malloc(width * height * 4);

		DWORD p = 0;
		for (int a = 0; a < width * height; a++)
		{
			temp[p++] = palette[data[a]].r;
			temp[p++] = palette[data[a]].g;
			temp[p++] = palette[data[a]].b;
			temp[p++] = palette[data[a]].a;
		}

		return gdk_pixbuf_new_from_data(temp, GDK_COLORSPACE_RGB, true,
			8, width, height, width * 4,
			destroy_pb, NULL);
	}
	else if (bpp == 32)
	{
		BYTE* temp = (BYTE*)malloc(width * height * 4);
		memcpy(temp, data, width * height * 4);

		return gdk_pixbuf_new_from_data(temp, GDK_COLORSPACE_RGB, true,
			8, width, height, width * 4,
			destroy_pb, NULL);
	}

	return NULL;
}

GdkPixbuf* Texture::get_pbuf_scale_fit(int w, int h)
{
	GdkPixbuf *original = get_pbuf();
	float scale;

	int n_width = width;
	int n_height = height;

	int d = min(w, h);

	if (width > height)
	{
		if (width > d)
		{
			scale = (float)w / (float)width;
			n_width = w;
			n_height = height * scale;
		}
	}
	else if (width < height)
	{
		if (height > d)
		{
			scale = (float)h / (float)height;
			n_height = h;
			n_width = width * scale;
		}
	}
	else if (width == height)
	{
		n_width = d;
		n_height = d;
	}

	GdkPixbuf *ret = gdk_pixbuf_scale_simple(original, n_width, n_height, GDK_INTERP_NEAREST);
	g_object_unref(original);
	return ret;
}

Texture* get_texture(string name)
{
	for (int a = 0; a < textures.size(); a++)
	{
		if (textures[a]->name == name)
			return textures[a];
	}

	return &no_tex;
}


void init_textures()
{
	no_tex.setup("_notex", 32, 128, 128);

	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			rgba_t col;
			col.set(255, 0, 0, 255);

			no_tex.add_pixel(x, y, col);
		}
	}
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
	
	for (int t = 0; t < n_tex; t++)
	{
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
		//tex_list.add_tex(texname, width, height, true, false);
		//tex_list.init_tex_data();
		//texture_t *tex = tex_list.get_last();
		//printf("Added texture %s\n", texname);
		Texture *tex = new Texture();
		tex->setup(texname, 8, width, height);
		textures.push_back(tex);
		//console_print(parse_string("Loading texture %s", texname));
		//wait_gtk_events();

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
				//printf("poo\n");
				add_patch_to_tex(xoff, yoff, wad, patchlump, tex);
			else
			{
				patchlump = iwad->get_lump(pnames[patch], iwad->patches[START]);

				if (patchlump)
					//printf("poo\n");
					add_patch_to_tex(xoff, yoff, iwad, patchlump, tex);
				else
				{
					printf("Patch \"%s\" doesn't exist!\n", pnames[patch].c_str());
					break;
				}
			}
		}

		//tex_list.create_tex(gl_filter, 0);
	}
}

// load_textures: Loads all textures from the last wad opened containing a TEXTUREx lump
// (or the IWAD if no open wads have TEXTUREx lumps...)
// ---------------------------------------------------------------------------------- >>
void load_textures()
{
	for (int a = 0; a < textures.size(); a++)
		delete textures[a];

	textures.clear();

	// Read the palette
	read_palette(wads.get_wad_with_lump("PLAYPAL"));

	// Get the wad with TEXTURE1
	Wad* tex_wad = wads.get_wad_with_lump("TEXTURE1");

	//print(true, "Loading textures from %s\n", tex_wad->path.c_str());
	// Console stuff
	popup_console();
	console_print("Loading textures...");
	wait_gtk_events();

	// Load PNAMES
	load_pnames(tex_wad);

	// Load TEXTURE1
	load_textures_lump(tex_wad, tex_wad->get_lump("TEXTURE1", 0));

	// Load TEXTURE2 if it exists
	if (tex_wad->get_lump("TEXTURE2", 0))
		load_textures_lump(tex_wad, tex_wad->get_lump("TEXTURE2", 0));

	console_print("Done");
	hide_console();

	// Test memory usage
	for (int a = 0; a < textures.size(); a++)
		textures[a]->get_pbuf();

	/*
	Wad* htexwad = wads.get_wad_with_lump("HIRESTEX");

	if (htexwad)
		load_zgl_hirestex(htexwad, htexwad->get_lump("HIRESTEX", 0));
		*/
}
