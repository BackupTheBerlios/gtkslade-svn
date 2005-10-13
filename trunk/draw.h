
void draw_map();
void update_map();
void update_grid();
void draw_texture_scale(rect_t rect, string texname = "_notex", int textype = 0, rgba_t col = rgba_t(255, 255, 255, 255), float scalef = 1.0f);
void draw_texture(int x, int y, int width = -1, int height = -1, string texname = "_notex", int textype = 0, rgba_t col = rgba_t(255, 255, 255, 255));
