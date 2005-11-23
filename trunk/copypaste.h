
#define	COPY_THINGS			1
#define COPY_ARCHITECTURE	2

/*
struct copy_line_t
{
	rect_t		rect;
	sidedef_t	sides[2];
	linedef_t	props;
};
*/

class Clipboard
{
private:
	BYTE					copy_type;
	vector<thing_t*>		things;
	vector<sidedef_t*>		sides;
	vector<linedef_t*>		lines;
	vector<sector_t*>		sectors;
	vector<vertex_t*>		verts;

public:
	Clipboard();
	~Clipboard();

	void Clear();
	void Copy();
	void Paste();
	void DrawPaste();
};
