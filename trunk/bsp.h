struct gl_seg_t
{
	long	vertex1;
	long	vertex2;
	short	line;
	short	flags;
	long	partner;
};

struct gl_ssect_t
{
	long	n_segs;
	long	startseg;
};

struct gl_vertex_t
{
	int	x;
	int	y;
};

struct gl_node_t
{
   short x; // partition line
   short y;
   short dx;
   short dy;
   short right_bbox[4];
   short left_bbox[4];
   unsigned short right_child;
   unsigned short left_child;
};


void walk_bsp_tree(unsigned short node);
void open_view();
void set_visbuffer(int blocked);
void clear_visibility();
void build_gl_nodes();

#define CHILD_IS_SUBSEC (1 << 15)
#define SEG_MINISEG		0xFFFF
#define	SEG_FLAGS_SIDE	0x0001
#define SEG_GLVERTEX	0x40000000