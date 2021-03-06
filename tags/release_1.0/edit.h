
#define CHANGE_SAVE		0x01
#define CHANGE_NODES	0x02
#define CHANGE_THINGS	0x04
#define CHANGE_LINES	0x08
#define CHANGE_SECTORS	0x10

double s_x(double x);
double s_y(double y);
double m_x(double x);
double m_y(double y);
point2_t s_p(point2_t point);
point2_t m_p(point2_t point);
int difference(int v1, int v2);
void draw_2d_screen();
void get_hilight_item(int x, int y);
void select_item();
void select_items_box(rect_t box);
void clear_selection();
bool selection();
int snap_to_grid(double pos);
int snap_to_grid_custom(int pos);
point2_t midpoint(point2_t p1, point2_t p2);
int get_side_sector(int line, int side);
void move_items();
void add_move_items();
void clear_move_items();
int check_vertex_split(DWORD vertex);
int check_vertex_split(point2_t p);
void create_vertex();
void create_thing();
void create_lines(bool close);
void create_sector();
void delete_vertex();
void delete_line();
void delete_thing();
void delete_sector();
void init_map();
int determine_sector(double x, double y);
void line_flip(bool verts, bool sides);
void sector_changeheight(bool floor, int amount);
void line_drawpoint();
void end_linedraw();
void line_undrawpoint();
int get_nearest_line_2(double x, double y);
bool determine_line_side(int line, float x, float y);
bool determine_line_side(rect_t r, float x, float y);
bool determine_line_side(float x1, float y1, float x2, float y2, float x, float y);
float determine_line_side_f(rect_t r, float x, float y);
void map_changelevel(int level);
void thing_setquickangle();
void merge_verts();
bool check_gui_mouse();
void increase_grid();
void decrease_grid();
void get_line_colour(WORD l, rgba_t *colour);
bool check_overlapping_lines(vector<int> lines);
void merge_overlapping_lines(vector<int> lines);
void change_edit_mode(int mode);
void init_map();