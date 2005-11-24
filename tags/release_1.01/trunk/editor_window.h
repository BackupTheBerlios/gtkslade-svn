
void setup_editor_window();
//void redraw_map(bool update_map, int x = 0, int y = 0, int width = -1, int height = -1);
void force_map_redraw(bool map = false, bool grid = false);
bool open_map(Wad* wad, string mapname);
void save_window_properties(FILE* fp);
void load_window_properties(Tokenizer *tz);
