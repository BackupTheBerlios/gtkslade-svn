#include "main.h"
#include "map.h"
#include "misc.h"
#include "checks.h"
#include "editor_window.h"

int x_offset = -1;
int y_offset = -1;
string midtex = "";
string uptex = "";
string lotex = "";

extern Map map;
extern GtkWidget *editor_window;
extern vector<int> selected_items;
extern int hilight_item;
