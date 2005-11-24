
void set_gl_colour(rgba_t col);
void draw_map();
void update_map();
void update_grid();
void draw_texture_scale(rect_t rect, string texname = "_notex", int textype = 0, rgba_t col = rgba_t(255, 255, 255, 255), float scalef = 1.0f);
void draw_texture(int x, int y, int width = -1, int height = -1, string texname = "_notex", int textype = 0, rgba_t col = rgba_t(255, 255, 255, 255));
void draw_rect(rect_t rect, rgba_t col, bool fill);
void draw_text(int x, int y, rgba_t colour, BYTE alignment, const char* text, ...);
bool draw_thing(thing_t *thing, bool moving = false, bool selected = false);
void draw_line(rect_t rect, rgba_t col, bool aa, bool side_indicator = false);