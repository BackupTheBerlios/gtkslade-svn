void start_3d_mode();
void apply_gravity();
void determine_hilight();

// Editing
void change_sector_height_3d(int amount, bool floor = true);
void change_texture_3d(bool paint);
void change_offsets_3d(int x, int y);
void toggle_texture_peg_3d(bool upper);
void copy_texture_3d();
void paste_texture_3d(bool paint);
void change_light_3d(int amount);
void change_thing_angle_3d(int amount);
void change_thing_z_3d(int amount);
void auto_align_x_3d();
void reset_offsets_3d();
void add_3d_message(string message);
