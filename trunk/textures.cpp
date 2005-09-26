
#include "main.h"
#include "textures.h"

Texture::Texture()
{
	this->data = NULL;
	this->width = 0;
	this->height = 0;
	this->bpp = 8;
}

Texture::~Texture()
{
	if (data)
		free(data);
}
