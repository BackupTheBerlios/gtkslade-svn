
class Texture
{
private:
	WORD	width;
	WORD	height;
	BYTE	bpp;
	BYTE	*data;

public:
	Texture();
	~Texture();
};
