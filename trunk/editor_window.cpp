// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << editor_window.cpp - Editor window     >>
// << functions                             >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "map.h"
#include "draw.h"
#include "edit.h"
#include "editor_window.h"
#include "console_window.h"
#include "main_window.h"
#include "keybind.h"
#include "input.h"
#include "linedraw.h"
#include "info_bar.h"
#include "prefs_dialog.h"
#include "script_editor.h"
#include "3dmode.h"
#include "edit_misc.h"
#include "tex_browser.h"
#include "checks.h"

// Variables ------------------------------ >>
GtkWidget	*editor_window = NULL;
GtkWidget	*map_area = NULL;
GdkPixmap	*pixmap = NULL;

Wad*		edit_wad = NULL;

rect_t	sel_box(-1, -1, 0, 0, 0);
point2_t mouse;

GdkGLConfig *glconfig = NULL;
GdkGLContext *glcontext = NULL;

int vid_width;
int vid_height;

bool thing_quickangle = false;
bool items_moving = false;

// External Variables --------------------- >>
extern Map map;
extern int hilight_item, edit_mode;
extern BindList binds;
extern bool line_draw;

// init_opengl: Initialises OpenGL
// ---------------------------- >>
bool init_opengl()
{
	glViewport(0, 0, vid_width, vid_height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, vid_width, vid_height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

// realize_main: Called when map_area is realized
// ------------------------------------------- >>
static void realize_main(GtkWidget *widget, gpointer data)
{
	//GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();

	gdk_gl_drawable_gl_end (gldrawable);
}

/*
// realize_sub: Called when other context-sharing gl widgets are realized
// ------------------------------------------------------------------- >>
static void realize_sub(GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();

	gdk_gl_drawable_gl_end(gldrawable);
}
*/

void render_map()
{
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(map_area);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	draw_map();

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);

	/*
	PangoLayout *pl = gtk_widget_create_pango_layout(map_area, "Testing!");
	gdk_draw_layout(map_area->window, map_area->style->text_aa_gc[GTK_STATE_NORMAL],
					10, 10, pl);
					*/
}

// expose_event: Called when a part of the map_area needs to be redrawn
// ----------------------------------------------------------------- >>
gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	render_map();

	return false;
}

// force_map_redraw: Forces the map area to be redrawn
// ------------------------------------------------ >>
void force_map_redraw(bool map, bool grid)
{
	if (grid)
		update_grid();

	if (map)
		update_map();

	render_map();
}

// configure_event: Called when the map area is resized or initialised
// ---------------------------------------------------------------- >>
gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return FALSE;

	glViewport(0, 0, widget->allocation.width, widget->allocation.height);
	vid_width = widget->allocation.width;
	vid_height = widget->allocation.height;
	init_opengl();
	update_map();
	update_grid();
	render_map();

	gdk_gl_drawable_gl_end (gldrawable);

	return true;
}

// motion_notify_event: When the mouse pointer is moved over the map area
// ------------------------------------------------------------------- >>
static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	bool redraw_map = false;
	bool update_map = false;
	GdkModifierType state;
	
	if (event->is_hint)
	{
		gdk_window_get_pointer(event->window, &x, &y, &state);
	}
	else
	{
		x = event->x;
		y = event->y;
		state = (GdkModifierType)(event->state);
	}

	mouse.set(x, y);

	if (state & GDK_BUTTON3_MASK)
	{
		if (items_moving)
		{
			move_items();
			redraw_map = true;
			update_map = true;
		}
		else
		{
			add_move_items();
			items_moving = true;
			redraw_map = update_map = true;
		}
	}

	if (state & GDK_BUTTON2_MASK)
	{
		// Quick thing angle
		thing_quickangle = true;
		thing_setquickangle();
		redraw_map = true;
		update_map = true;
	}

	if (sel_box.x1() != -1)
	{
		sel_box.br.set(x, y);

		if (line_draw)
			line_drawbox();

		redraw_map = true;
	}
	else
	{
		if (line_draw)
			redraw_map = true;

		if (!thing_quickangle)
		{
			int old_hilight = hilight_item;
			get_hilight_item(x, y);

			if (hilight_item != old_hilight)
				redraw_map = true;
		}
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// button_press_event: When a mouse button is pressed in the map area
// --------------------------------------------------------------- >>
static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	bool redraw_map = false;
	bool update_map = false;

	// Single click
	if (event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == 1)
		{
			if (event->state & GDK_SHIFT_MASK)
				sel_box.set(event->x, event->y, event->x, event->y);
			else
			{
				if (line_draw)
				{
					line_drawpoint();
					redraw_map = true;
				}
				else
				{
					select_item();
					redraw_map = true;
					update_map = true;
				}
			}
		}

		if (event->button == 3)
		{
			if (line_draw)
			{
				line_undrawpoint();
				redraw_map = true;
			}
			/*
			else
			{
				add_move_items();
				redraw_map = update_map = true;
			}
			*/
		}
	}

	// Double click
	if (event->type == GDK_2BUTTON_PRESS)
	{
		if (event->button == 3)
		{
			edit_item();
			clear_move_items();
			redraw_map = update_map = true;
		}
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// button_release_event: When a mouse button is released in the map area
// ------------------------------------------------------------------ >>
static gboolean button_release_event(GtkWidget *widget, GdkEventButton *event)
{
	bool redraw_map = false;
	bool update_map = false;

	if (event->button == 1)
	{
		if (sel_box.x1() != -1)
		{
			select_items_box(sel_box);
			sel_box.set(-1, -1, -1, -1);
			redraw_map = update_map = true;
		}
	}

	if (event->button == 3)
	{
		if (!line_draw)
		{
			clear_move_items();
			items_moving = false;
			redraw_map = update_map = true;
		}
	}

	if (event->button == 2)
	{
		thing_quickangle = false;
		redraw_map = update_map = true;
	}

	if (redraw_map)
		force_map_redraw(update_map);

	return TRUE;
}

// key_press_event: When a keyboard key is pressed
// -------------------------------------------- >>
gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	//printf("\"%s\"\n", key.c_str());
	binds.set(key);

	keys_edit();

	return false;
}

// key_release_event: When a keyboard key is released
// ----------------------------------------------- >>
gboolean key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	string key = gtk_accelerator_name(vals[0], (GdkModifierType)event->state);
	//printf("\"%s\"\n", key.c_str());
	binds.unset(key);

	return false;
}

// destroy: Called when the editor window is closed
// --------------------------------------------- >>
static gboolean destroy(GtkWidget *widget, gpointer data)
{
	gtk_widget_show_all(editor_window);

	if (map.changed & MC_SAVE_NEEDED)
	{
		if (!yesno_box("There are unsaved changes, are you sure you want to exit?"))
			return true;
	}

    gtk_main_quit();

	return false;
}

// MENU STUFF
void file_save()
{
	map.add_to_wad(edit_wad);
	edit_wad->save(true);

	map.changed = (map.changed & ~MC_SAVE_NEEDED);
	string title = gtk_window_get_title(GTK_WINDOW(editor_window));
	if (g_str_has_suffix(title.c_str(), "*"))
		title.erase(title.size() - 1, 1);
	gtk_window_set_title(GTK_WINDOW(editor_window), title.c_str());
}

void file_saveas()
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Wad",
													NULL,
													GTK_FILE_CHOOSER_ACTION_SAVE,
													GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
													NULL);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		string filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		
		if (!g_str_has_suffix(filename.c_str(), ".wad"))
			filename += ".wad";

		edit_wad = new Wad();
		edit_wad->path = filename;
		map.add_to_wad(edit_wad);
		edit_wad->save(true);
		gtk_window_set_title(GTK_WINDOW(editor_window), parse_string("SLADE (%s, %s)", edit_wad->path.c_str(), map.name.c_str()).c_str());
		add_recent_wad(filename);

		map.changed = (map.changed & ~MC_SAVE_NEEDED);
		string title = gtk_window_get_title(GTK_WINDOW(editor_window));
		if (g_str_has_suffix(title.c_str(), "*"))
			title.erase(title.size() - 1, 1);
		gtk_window_set_title(GTK_WINDOW(editor_window), title.c_str());
	}

	gtk_widget_destroy(dialog);
}

void file_close()
{
	if (map.changed & MC_SAVE_NEEDED)
	{
		if (!yesno_box("There are unsaved changes, are you sure you want to close the map?"))
			return;
	}

	map.close();
	force_map_redraw(true, true);
	gtk_window_set_title(GTK_WINDOW(editor_window), "SLADE");
}

void edit_create_stairs()
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Create Stairs",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	// Floor step entry
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, false, false, 0);
	GtkWidget *entry_floor = gtk_entry_new();
	gtk_widget_set_size_request(entry_floor, 32, -1);
	gtk_entry_set_text(GTK_ENTRY(entry_floor), "8");
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Floor step height: "), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(""), true, true, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry_floor, false, false, 0);

	// Ceiling step entry
	hbox = gtk_hbox_new(false, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, false, false, 0);
	GtkWidget *entry_ceil = gtk_entry_new();
	gtk_widget_set_size_request(entry_ceil, 32, -1);
	gtk_entry_set_text(GTK_ENTRY(entry_ceil), "0");
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Ceiling step height: "), false, false, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(""), true, true, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry_ceil, false, false, 0);

	gtk_widget_show_all(dialog);
	int result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_ACCEPT)
	{
		int f_step = atoi(gtk_entry_get_text(GTK_ENTRY(entry_floor)));
		int c_step = atoi(gtk_entry_get_text(GTK_ENTRY(entry_ceil)));
		sector_create_stairs(f_step, c_step);
	}

	gtk_widget_destroy(dialog);
}

void check_map_stats()
{
	string msg = parse_string("Vertices: %d\nLines: %d\nSides: %d\nSectors: %d\nThings: %d",
								map.n_verts, map.n_lines, map.n_sides, map.n_sectors, map.n_things);

	message_box(msg, GTK_MESSAGE_INFO);
}

// menu_action: Called when a menu/toolbar item is selected
// ----------------------------------------------------- >>
static void menu_action(GtkAction *action)
{
	string act = gtk_action_get_name(action);

	if (act == "WadManager")
		open_main_window();
	else if (act == "Exit")
		gtk_main_quit();
	else if (act == "Close")
		file_close();
	else if (act == "ModeVerts")
		change_edit_mode(0);
	else if (act == "ModeLines")
		change_edit_mode(1);
	else if (act == "ModeSectors")
		change_edit_mode(2);
	else if (act == "ModeThings")
		change_edit_mode(3);
	else if (act == "Mode3d")
		start_3d_mode();
	else if (act == "ShowConsole")
		popup_console();
	else if (act == "ShowScriptEditor")
		open_script_edit();
	else if (act == "Preferences")
		open_prefs_dialog();
	else if (act == "About")
	{
		gtk_show_about_dialog(GTK_WINDOW(editor_window),
								"name", "SLADE",
								"comments", "by Simon 'SlayeR' Judd, 2005",
								"version", "1.0 beta",
								"website", "http://slade.mancubus.net",
								NULL);
	}
	else if (act == "Save")
	{
		if (edit_wad)
			file_save();
		else
			file_saveas();
	}
	else if (act == "SaveAs")
		file_saveas();
	else if (act == "MergeSectors")
		sector_merge(false);
	else if (act == "JoinSectors")
		sector_merge(true);
	else if (act == "CreateDoor")
		sector_create_door(open_texture_browser(true, false, false, "-"));
	else if (act == "CreateStairs")
		edit_create_stairs();
	else if (act == "CheckMapStats")
		check_map_stats();
	else if (act == "CheckTags")
		message_box(parse_string("%d Tags Cleaned", clean_tags()), GTK_MESSAGE_INFO);
	else if (act == "CheckVerts")
		message_box(parse_string("%d Vertices Removed", remove_free_verts()), GTK_MESSAGE_INFO);
	else if (act == "CheckLines")
		message_box(parse_string("%d 0-Length Lines Removed", remove_zerolength_lines()), GTK_MESSAGE_INFO);
	else if (act == "CheckSectors")
		message_box(parse_string("%d Sectors Removed", remove_unused_sectors()), GTK_MESSAGE_INFO);
	else if (act == "CheckTextures")
		check_textures();
	else if (act == "AlignX")
		line_align_x();
	else if (act == "CorrectRefs")
		line_correct_references();
	else
		message_box("Menu action not implemented", GTK_MESSAGE_INFO);

	force_map_redraw(true, true);
}

/*
static void menu_mode_select(GtkAction *action, GtkRadioAction *current)
{
	//g_message ("Radio action \"%s\" selected", gtk_action_get_name(GTK_ACTION(current)));
}
*/

static GtkActionEntry entries[] = {
	{ "FileMenu", NULL, "_File" },
	{ "EditMenu", NULL, "_Edit" },
	{ "EditLineMenu", NULL, "_Lines" },
	{ "EditSectorMenu", NULL, "_Sectors" },
	{ "ModeMenu", NULL, "_Mode"  },
	{ "ViewMenu", NULL, "_View"  },
	{ "CheckMenu", NULL, "_Check" },
	{ "HelpMenu", NULL, "_Help" },

	// File menu
	{ "WadManager", GTK_STOCK_EXECUTE, "Wad _Manager", "<control>W", "Open the wad manager", G_CALLBACK(menu_action) },
	{ "Save", GTK_STOCK_SAVE, "_Save", "<control>S", "Save map", G_CALLBACK(menu_action) },
	{ "SaveAs", GTK_STOCK_SAVE_AS, "Save _As...", NULL, "Save map to a new file", G_CALLBACK(menu_action) },
	{ "Close", GTK_STOCK_CLOSE, "_Close", "", "Close the map", G_CALLBACK(menu_action) },
	{ "Exit", GTK_STOCK_QUIT, "E_xit SLADE", NULL, "Close the map", G_CALLBACK(menu_action) },

	// Line edit menu
	{ "AlignX", NULL, "Align Textures _X", NULL, "Align selected wall textures along the x axis", G_CALLBACK(menu_action) },
	{ "AlignY", NULL, "Align Textures _Y", NULL, "Align selected wall textures along the y axis", G_CALLBACK(menu_action) },
	{ "CorrectRefs", NULL, "_Correct Sector References", NULL,
	  "Attempts to set the correct sector references for the line", G_CALLBACK(menu_action) },

	// Sector edit menu
	{ "MergeSectors", NULL, "_Merge Sectors", NULL, "Merge selected sectors", G_CALLBACK(menu_action) },
	{ "JoinSectors", NULL, "_Join Sectors", NULL, "Join selected sectors", G_CALLBACK(menu_action) },
	{ "CreateDoor", NULL, "Create _Door", NULL, "Create door(s) from selected sector(s)", G_CALLBACK(menu_action) },
	{ "CreateStairs", NULL, "Create _Stairs", NULL, "Create stairs from selected sectors", G_CALLBACK(menu_action) },

	// Edit menu
	{ "Preferences", NULL, "_Preferences...", NULL, "Change SLADE Preferences", G_CALLBACK(menu_action) },

	// Mode menu
	{ "ModeVerts", "slade-mode-verts", "_Vertices", NULL, "Vertices Mode", G_CALLBACK(menu_action) },
	{ "ModeLines", "slade-mode-lines", "_Lines", NULL, "Lines Mode", G_CALLBACK(menu_action) },
	{ "ModeSectors", "slade-mode-sectors", "_Sectors", NULL, "Sector Mode", G_CALLBACK(menu_action) },
	{ "ModeThings", "slade-mode-things", "_Things", NULL, "Things Mode", G_CALLBACK(menu_action) },
	{ "Mode3d", "slade-mode-3d", "_3d Mode", NULL, "3d Mode", G_CALLBACK(menu_action) },

	// View menu
	{ "ShowConsole", NULL, "Show _Console", NULL, "Shows the SLADE console window", G_CALLBACK(menu_action) },
	{ "ShowScriptEditor", NULL, "Open _Script Editor", NULL, "Shows the SLADE scripts editor", G_CALLBACK(menu_action) },

	// Checks menu
	{ "CheckMapStats", NULL, "_Map Statistics", NULL, "View map statistics", G_CALLBACK(menu_action) },
	{ "CheckTags", NULL, "Clean _Tags", NULL, "Cleans unused/unmatched tags", G_CALLBACK(menu_action) },
	{ "CheckVerts", NULL, "Remove Unused _Vertices", NULL, "Deletes any unattached vertices", G_CALLBACK(menu_action) },
	{ "CheckLines", NULL, "Remove 0-Length _Lines", NULL, "Deletes any lines that are of length 0", G_CALLBACK(menu_action) },
	{ "CheckSectors", NULL, "Remove Unused _Sectors", NULL, "Deletes any unused sectors", G_CALLBACK(menu_action) },
	{ "CheckTextures", NULL, "Check Valid _Textures", NULL, "Checks for any invalid textures", G_CALLBACK(menu_action) },
	
	// Help menu
	{ "About", GTK_STOCK_ABOUT, "_About SLADE", NULL, "", G_CALLBACK(menu_action) },
	{ "Help", GTK_STOCK_HELP, "_Help", NULL, "", G_CALLBACK(menu_action) },
};
static guint n_entries = G_N_ELEMENTS(entries);

/*
static GtkRadioActionEntry mode_entries[] = {
	
};
static guint n_mode_entries = G_N_ELEMENTS(mode_entries);
*/

static const gchar *ui_info = 
"<ui>"
"  <menubar name='MenuBar'>"
"   <menu action='FileMenu'>"
"     <menuitem action='WadManager'/>"
"     <separator/>"
"     <menuitem action='Save'/>"
"     <menuitem action='SaveAs'/>"
"     <menuitem action='Close'/>"
"     <separator/>"
"     <menuitem action='Exit'/>"
"   </menu>"
"   <menu action='EditMenu'>"
"    <menu action='EditLineMenu'>"
"     <menuitem action='AlignX'/>"
"     <menuitem action='CorrectRefs'/>"
//"     <menuitem action='AlignY'/>"
"    </menu>"
"    <menu action='EditSectorMenu'>"
"     <menuitem action='MergeSectors'/>"
"     <menuitem action='JoinSectors'/>"
"     <separator/>"
"     <menuitem action='CreateDoor'/>"
"     <menuitem action='CreateStairs'/>"
"    </menu>"
"    <separator/>"
"    <menuitem action='Preferences'/>"
"   </menu>"
"   <menu action='ModeMenu'>"
"    <menuitem action='ModeVerts'/>"
"    <menuitem action='ModeLines'/>"
"    <menuitem action='ModeSectors'/>"
"    <menuitem action='ModeThings'/>"
"    <menuitem action='Mode3d'/>"
"   </menu>"
"   <menu action='ViewMenu'>"
"    <menuitem action='ShowConsole'/>"
"    <menuitem action='ShowScriptEditor'/>"
"   </menu>"
"   <menu action='CheckMenu'>"
"    <menuitem action='CheckMapStats'/>"
"    <menuitem action='CheckTags'/>"
"    <menuitem action='CheckVerts'/>"
"    <menuitem action='CheckLines'/>"
"    <menuitem action='CheckSectors'/>"
"    <menuitem action='CheckTextures'/>"
"   </menu>"
"   <menu action='HelpMenu' position='bot'>"
"    <menuitem action='About'/>"
"    <menuitem action='Help'/>"
"   </menu>"
"  </menubar>"
"  <toolbar  name='ToolBar'>"
"    <placeholder/>"
"    <toolitem action='WadManager'/>"
"    <separator action='Sep1'/>"
"    <toolitem action='Save'/>"
"    <toolitem action='SaveAs'/>"
"    <toolitem action='Close'/>"
"    <separator action='Sep2'/>"
"    <toolitem action='ModeVerts'/>"
"    <toolitem action='ModeLines'/>"
"    <toolitem action='ModeSectors'/>"
"    <toolitem action='ModeThings'/>"
"    <toolitem action='Mode3d'/>"
"  </toolbar>"
"</ui>";

// tbar_keypress: Custom handler for keypresses on the toolbar (prevents them)
// if I don't prevent them, pressing the arrow keys to move around the map
// will select different toolbar items, and we don't want that :P
// ------------------------------------------------------------------------ >>
gboolean tbar_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	return true;
}

// setup_editor_window: Sets up the main editor window
// ------------------------------------------------ >>
void setup_editor_window()
{
	// Setup OpenGL
	glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB|GDK_GL_MODE_DEPTH|GDK_GL_MODE_DOUBLE));

	if (glconfig == NULL)
	{
		glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_RGB|GDK_GL_MODE_DEPTH));

		if (glconfig == NULL)
			exit(1);
	}

	editor_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(editor_window), "SLADE");
	gtk_widget_set_size_request(editor_window, 800, 600);
	gtk_widget_set_double_buffered(editor_window, true);

	GtkWidget *main_vbox = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(editor_window), main_vbox);

	// Menu and Toolbar
	GtkUIManager *ui = gtk_ui_manager_new();
	GtkActionGroup *actions = gtk_action_group_new ("Actions");
	GError *error = NULL;

	gtk_action_group_add_actions(actions, entries, n_entries, NULL);
	//gtk_action_group_add_radio_actions(actions, mode_entries, n_mode_entries, 1, G_CALLBACK(menu_mode_select), NULL);

	gtk_ui_manager_insert_action_group(ui, actions, 0);
	gtk_window_add_accel_group(GTK_WINDOW(editor_window), gtk_ui_manager_get_accel_group(ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		printf("Building menus failed: %s", error->message);
		g_error_free(error);
	}

	gtk_box_pack_start (GTK_BOX(main_vbox), gtk_ui_manager_get_widget(ui, "/MenuBar"), false, false, 0);

	GtkWidget *toolbar = gtk_ui_manager_get_widget(ui, "/ToolBar");
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_MENU);
	gtk_container_unset_focus_chain(GTK_CONTAINER(toolbar));
	GTK_WIDGET_UNSET_FLAGS(toolbar, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(toolbar, GTK_HAS_FOCUS);
	g_signal_connect(G_OBJECT(toolbar), "key-press-event", G_CALLBACK(tbar_keypress), NULL);
	gtk_box_pack_start (GTK_BOX(main_vbox), toolbar, false, false, 0);

	// Map area
	map_area = gtk_drawing_area_new();

	gtk_widget_set_gl_capability(map_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

	gtk_widget_set_events(map_area, GDK_EXPOSURE_MASK
									| GDK_LEAVE_NOTIFY_MASK
									| GDK_BUTTON_PRESS_MASK
									| GDK_BUTTON_RELEASE_MASK
									| GDK_POINTER_MOTION_MASK
									| GDK_POINTER_MOTION_HINT_MASK);

	g_signal_connect(G_OBJECT(map_area), "expose-event", G_CALLBACK(expose_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "configure-event", G_CALLBACK(configure_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "button_press_event", G_CALLBACK(button_press_event), NULL);
	g_signal_connect(G_OBJECT(map_area), "button_release_event", G_CALLBACK(button_release_event), NULL);
	g_signal_connect(G_OBJECT(editor_window), "key-press-event", G_CALLBACK(key_press_event), NULL);
	g_signal_connect(G_OBJECT(editor_window), "key-release-event", G_CALLBACK(key_release_event), NULL);
	g_signal_connect_after(G_OBJECT(map_area), "realize", G_CALLBACK (realize_main), NULL);
	g_signal_connect(G_OBJECT(editor_window), "delete_event", G_CALLBACK(destroy), NULL);
	gtk_box_pack_start(GTK_BOX(main_vbox), map_area, true, true, 0);

	// Setup widgets that need to share the context
	gtk_widget_realize(map_area);
	glcontext = gtk_widget_get_gl_context(map_area);

	// Info area
	GtkWidget *infobar = get_info_bar();
	gtk_widget_set_size_request(infobar, -1, 144);
	gtk_box_pack_start(GTK_BOX(main_vbox), infobar, false, false, 0);
	change_infobar_page();

	gtk_window_maximize(GTK_WINDOW(editor_window));
	gtk_widget_show_all(editor_window);
}

// open_map: Opens a map to edit, if wad is NULL then it's a new/standalone map
// ------------------------------------------------------------------------- >>
bool open_map(Wad* wad, string mapname)
{
	if (wad)
	{
		if (!map.open(wad, mapname))
			return false;

		gtk_window_set_title(GTK_WINDOW(editor_window), parse_string("SLADE (%s, %s)", wad->path.c_str(), mapname.c_str()).c_str());
	}
	else
	{
		map.create(mapname);
		gtk_window_set_title(GTK_WINDOW(editor_window), parse_string("SLADE (unsaved, %s)", mapname.c_str()).c_str());
	}

	force_map_redraw(true, true);
	edit_wad = wad;
	return true;
}
