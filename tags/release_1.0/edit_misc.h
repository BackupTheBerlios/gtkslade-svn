
void sector_create_stairs(int floor_step, int ceil_step);
void sector_create_door(string texture);
void sector_merge(bool remove_lines);

void line_extrude(int amount);
void line_align_x();
void line_correct_references();
void line_auto_align_x(int line, int offset, int side, string tex, int texwidth, vector<int> *processed_lines);
void line_paint_tex(int line, int side, string otex, string ntex, vector<int> *processed_lines);