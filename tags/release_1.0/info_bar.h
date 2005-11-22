
GtkWidget *get_info_bar();
void change_infobar_page();

GtkWidget *get_line_info_bar();
GtkWidget *get_vertex_info_bar();
GtkWidget *get_thing_info_bar();
GtkWidget *get_sector_info_bar();

void update_line_info_bar(int line);
void update_vertex_info_bar(int vertex);
void update_thing_info_bar(int thing);
void update_sector_info_bar(int sector);

void setup_label(GtkWidget **label, char* text);
