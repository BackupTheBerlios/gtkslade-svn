// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << input.cpp - Input stuff               >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "keybind.h"
#include "editor_window.h"
#include "edit.h"
#include "edit_misc.h"
#include "map.h"
#include "undoredo.h"
#include "line_edit.h"
#include "thing_edit.h"
#include "sector_edit.h"
#include "struct_3d.h"
#include "mathstuff.h"
#include "camera.h"
#include "3dmode.h"
#include "console.h"
#include "console_window.h"
#include "copypaste.h"

CVAR(Float, move_speed_3d, 0.3f, CVAR_SAVE)
CVAR(Float, mouse_speed_3d, 1.0f, CVAR_SAVE)
CVAR(Int, key_delay_3d, 7, CVAR_SAVE)

#define PRESSED(name) vector_exists(pressed_keys, name)
#define RELEASED(name) vector_exists(released_keys, name)

// External Variables --------------------- >>
extern BindList binds;
extern Map map;

extern int xoff, yoff, edit_mode, hilight_item;
extern float zoom;
extern bool line_draw, paste_mode, items_moving, thing_quickangle;
extern point2_t mouse, down_pos;
extern rect_t sel_box;
extern Clipboard clipboard;
extern Camera camera;

extern vector<string> pressed_keys;
extern vector<string> released_keys;

EXTERN_CVAR(Bool, render_fog)
EXTERN_CVAR(Bool, render_fullbright)
EXTERN_CVAR(Bool, render_hilight)
EXTERN_CVAR(Int, render_things)
EXTERN_CVAR(Bool, edit_snap_grid)

// cycle_edit_mode: Cycles the edit mode
// ----------------------------------- >>
void cycle_edit_mode()
{
	if (++edit_mode > 3)
		edit_mode = 0;

	force_map_redraw(true, false);
}

// edit_item: When the edit item key is pressed
// ----------------------------------------- >>
void edit_item()
{
	if (edit_mode == 0)
	{
		create_vertex();
		get_hilight_item(mouse.x, mouse.y);
		force_map_redraw(true, false);
		return;
	}

	if (hilight_item == -1 && !selection())
		return;

	if (edit_mode == 1)
		open_line_edit();

	if (edit_mode == 2)
		open_sector_edit();

	if (edit_mode == 3)
		open_thing_edit();
}

void view_zoom(bool in)
{
	if (in)
	{
		zoom *= 1.2f;

		if (zoom > 1024)
			zoom = 1024;
	}
	else
	{
		zoom *= 0.8f;

		if (zoom < 1)
			zoom = 1;
	}

	force_map_redraw(true, true);
}

// keys_edit: Keys for the 2d editor
// ------------------------------ >>
void keys_edit()
{
	if (!map.opened || line_draw)
		return;

	// **** Dragging stuff ****


	// Selection box
	if (sel_box.x1() != -1 && RELEASED("edit_selectbox"))
	{
		select_items_box(sel_box);
		force_map_redraw(true, false);
		sel_box.set(-1, -1, -1, -1);
		return;
	}

	// Moving items
	if (items_moving && RELEASED("edit_moveitems"))
	{
		clear_move_items();
		force_map_redraw(true, false);
		items_moving = false;
		return;
	}

	// Quick thing angle
	if (thing_quickangle && RELEASED("thing_quickangle"))
	{
		thing_quickangle = false;
		force_map_redraw(true, false);
		return;
	}


	if (items_moving || sel_box.x1() != -1)
		return;

	// **** Other controls ****

	// Scroll up
	if (PRESSED("view_up"))
	{
		yoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}
	
	// Scroll down
	if (PRESSED("view_down"))
	{
		yoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}

	// Scroll left
	if (PRESSED("view_left"))
	{
		xoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}
	
	// Scroll right
	if (PRESSED("view_right"))
	{
		xoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}

	// Zoom in
	if (PRESSED("view_zoomin"))
		view_zoom(true);

	// Zoom out
	if (PRESSED("view_zoomout"))
		view_zoom(false);

	// Center view on mouse
	if (RELEASED("view_mousecenter"))
	{
		xoff = -m_x(mouse.x) / MAJOR_UNIT;
		yoff = -m_y(mouse.y) / MAJOR_UNIT;
		force_map_redraw(true, true);
	}

	// Set offsets to 0, 0
	if (RELEASED("view_origin"))
	{
		xoff = yoff = 0;
		force_map_redraw(true, true);
	}

	// Vertices mode
	if (RELEASED("mode_vertices"))
		change_edit_mode(0);
	
	// Linedefs mode
	if (RELEASED("mode_linedefs"))
		change_edit_mode(1);
	
	// Sectors mode
	if (RELEASED("mode_sectors"))
		change_edit_mode(2);
	
	// Things mode
	if (RELEASED("mode_things"))
		change_edit_mode(3);

	// Change mode
	if (RELEASED("mode_change"))
		cycle_edit_mode();

	// Increase grid size
	if (RELEASED("view_increasegrid"))
	{
		increase_grid();
		force_map_redraw(false, true);
	}

	// Decrease grid size
	if (RELEASED("view_decreasegrid"))
	{
		decrease_grid();
		force_map_redraw(false, true);
	}

	// Clear selection
	if (RELEASED("edit_clearselection"))
	{
		clear_selection();
		force_map_redraw(true);
	}

	// Delete item
	if (RELEASED("edit_deleteitem"))
	{
		if (edit_mode == 0)
			delete_vertex();

		if (edit_mode == 1)
			delete_line();

		if (edit_mode == 2)
			delete_sector();

		if (edit_mode == 3)
			delete_thing();

		force_map_redraw(true);
	}

	// Create item
	if (RELEASED("edit_createitem"))
	{
		if (edit_mode == 0)
		{
			if (!selection())
				create_vertex();
			else
				create_lines(false);

			force_map_redraw(true);
			return;
		}

		if (edit_mode == 1)
		{
			if (selection())
				create_sector();

			force_map_redraw(true);
			return;
		}

		if (edit_mode == 3)
		{
			create_thing();
			force_map_redraw(true);
			return;
		}

		get_hilight_item(mouse.x, mouse.y);
		binds.clear("edit_createitem");
	}

	// Select item
	if (RELEASED("edit_selectitem"))
	{
		select_item();
		force_map_redraw(true);
	}

	// Sector height quick changes (8 units)
	if (PRESSED("sector_upfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 8);
	}

	if (PRESSED("sector_downfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -8);
	}

	if (PRESSED("sector_upceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 8);
	}

	if (PRESSED("sector_downceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -8);
	}

	if (PRESSED("sector_upboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 8);
			sector_changeheight(false, 8);
		}
	}

	if (PRESSED("sector_downboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -8);
			sector_changeheight(false, -8);
		}
	}

	// Sector height quick changes (1 unit)
	if (PRESSED("sector_upfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 1);
	}

	if (PRESSED("sector_downfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -1);
	}

	if (PRESSED("sector_upceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 1);
	}

	if (PRESSED("sector_downceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -1);
	}

	if (PRESSED("sector_upboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 1);
			sector_changeheight(false, 1);
		}
	}

	if (PRESSED("sector_downboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -1);
			sector_changeheight(false, -1);
		}
	}

	// Sector light change
	if (RELEASED("sector_uplight"))
	{
		//binds.clear("sector_uplight");
		sector_changelight(16);
	}

	if (RELEASED("sector_downlight"))
	{
		//binds.clear("sector_downlight");
		sector_changelight(-16);
	}

	// Flip line
	if (RELEASED("line_flip"))
	{
		if (edit_mode == 1)
			line_flip(true, false);

		force_map_redraw(true);
	}

	// Swap line sides
	if (RELEASED("line_swapsides"))
	{
		if (edit_mode == 1)
			line_flip(false, true);

		force_map_redraw(true);
	}

	// Flip both line direction and sides
	if (RELEASED("line_flipboth"))
	{
		if (edit_mode == 1)
			line_flip(true, true);

		force_map_redraw(true);
	}

	// Begin line draw
	if (PRESSED("line_begindraw"))
	{
		if (!line_draw)
			line_draw = true;

		binds.clear("line_begindraw");
	}

	// Begin rectangle draw
	if (PRESSED("line_begindraw_rect"))
	{
		if (!line_draw)
		{
			line_draw = true;
			sel_box.set(mouse.x, mouse.y, mouse.x, mouse.y);
		}

		binds.clear("line_begindraw_rect");
	}

	// Undo
	if (RELEASED("edit_undo"))
	{
		undo();
		clear_selection();
		hilight_item = -1;
		force_map_redraw(true, true);
		//map_changelevel(3);
		map.change_level(MC_NODE_REBUILD);
		binds.clear("edit_undo");
	}

	// Edit item
	if (RELEASED("edit_edititem"))
	{
		edit_item();
		binds.clear("edit_edititem");
	}

	// Merge sectors
	if (RELEASED("sector_merge"))
	{
		sector_merge(false);
		binds.clear("sector_merge");
	}

	// Join sectors
	if (RELEASED("sector_join"))
	{
		sector_merge(true);
		binds.clear("sector_join");
	}

	if (RELEASED("view_3dmode"))
	{
		binds.clear("view_3dmode");
		binds.clear("3d_exit");
		start_3d_mode();
	}

	if (RELEASED("open_console"))
	{
		binds.clear("open_console");
		popup_console();
	}

	if (RELEASED("copy"))
	{
		binds.clear("copy");
		clipboard.Copy();
	}

	if (RELEASED("paste"))
	{
		binds.clear("paste");
		paste_mode = true;
		clear_selection();
	}

	if (RELEASED("cancel_paste"))
	{
		binds.clear("cancel_paste");
		paste_mode = false;
		force_map_redraw(true, false);
	}

	// Toggle grid snap
	if (RELEASED("edit_gridsnap"))
	{
		binds.clear("edit_gridsnap");
		edit_snap_grid = !edit_snap_grid;
		force_map_redraw(false, false);
	}
}

//#define KEY_3D_DELAY 7
int key_3d_rep = 0;

bool keys_3d()
{
	bool key_3d_allow = false;
	float speed = 0.2 * move_speed_3d;

	if (key_3d_rep == 0)
		key_3d_allow = true;

	if (binds.pressed("3d_exit"))
	{
		binds.clear("view_3dmode");
		binds.clear("3d_exit");
		return false;
	}

	if (binds.pressed("3d_forward"))
		camera.move_camera(speed);

	if (binds.pressed("3d_back"))
		camera.move_camera(-speed);

	if (binds.pressed("3d_left"))
		camera.rotate_view(speed, 0.0f, 0.0f, 1.0f);

	if (binds.pressed("3d_right"))
		camera.rotate_view(-speed, 0.0f, 0.0f, 1.0f);

	if (binds.pressed("3d_strafeleft"))
		camera.strafe_camera(-speed);

	if (binds.pressed("3d_straferight"))
		camera.strafe_camera(speed);

	if (binds.pressed("3d_moveup"))
	{
		camera.position.z += 0.1f;
		camera.view.z += 0.1f;
	}

	if (binds.pressed("3d_movedown"))
	{
		camera.position.z -= 0.1f;
		camera.view.z -= 0.1f;
	}

	if (binds.pressed("3d_toggle_fullbright"))
	{
		binds.clear("3d_toggle_fullbright");
		render_fullbright = !render_fullbright;
	}

	if (binds.pressed("3d_toggle_fog"))
	{
		binds.clear("3d_toggle_fog");
		render_fog = !render_fog;
	}
	
	if (binds.pressed("3d_toggle_things"))
	{
		binds.clear("3d_toggle_things");
		render_things = render_things + 1;

		if (render_things > 3)
			render_things = 0;
	}

	if (binds.pressed("3d_toggle_gravity"))
	{
		binds.clear("3d_toggle_gravity");
		camera.gravity = !camera.gravity;

		if (camera.gravity)
			add_3d_message("Gravity ON");
		else
			add_3d_message("Gravity OFF");
	}

	// Sector height quick changes (8 units)
	if (binds.pressed("3d_upfloor8") && key_3d_allow)
	{
		change_sector_height_3d(8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downfloor8") && key_3d_allow)
	{
		change_sector_height_3d(-8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upceil8") && key_3d_allow)
	{
		change_sector_height_3d(8, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downceil8") && key_3d_allow)
	{
		change_sector_height_3d(-8, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upboth8") && key_3d_allow)
	{
		change_sector_height_3d(8);
		change_sector_height_3d(8, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downboth8") && key_3d_allow)
	{
		change_sector_height_3d(-8);
		change_sector_height_3d(-8, false);
		key_3d_rep = key_delay_3d;
	}

	// Sector height quick changes (1 unit)
	if (binds.pressed("3d_upfloor") && key_3d_allow)
	{
		change_sector_height_3d(1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downfloor") && key_3d_allow)
	{
		change_sector_height_3d(-1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upceil") && key_3d_allow)
	{
		change_sector_height_3d(1, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downceil") && key_3d_allow)
	{
		change_sector_height_3d(-1, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upboth") && key_3d_allow)
	{
		change_sector_height_3d(1);
		change_sector_height_3d(1, false);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downboth") && key_3d_allow)
	{
		change_sector_height_3d(-1);
		change_sector_height_3d(-1, false);
		key_3d_rep = key_delay_3d;
	}

	// Texture offset
	if (binds.pressed("3d_upyoffset") && key_3d_allow)
	{
		change_offsets_3d(0, 1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downyoffset") && key_3d_allow)
	{
		change_offsets_3d(0, -1);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upxoffset") && key_3d_allow)
	{
		change_offsets_3d(1, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downxoffset") && key_3d_allow)
	{
		change_offsets_3d(-1, 0);
		key_3d_rep = key_delay_3d;
	}

	// Texture offset x8
	if (binds.pressed("3d_upyoffset8") && key_3d_allow)
	{
		change_offsets_3d(0, 8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downyoffset8") && key_3d_allow)
	{
		change_offsets_3d(0, -8);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_upxoffset8") && key_3d_allow)
	{
		change_offsets_3d(8, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downxoffset8") && key_3d_allow)
	{
		change_offsets_3d(-8, 0);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_toggle_hilight"))
	{
		binds.clear("3d_toggle_hilight");
		render_hilight = !render_hilight;
	}

	// Upper/Lower unpegged toggle
	if (binds.pressed("3d_upperunpegged"))
	{
		binds.clear("3d_upperunpegged");
		toggle_texture_peg_3d(true);
	}

	if (binds.pressed("3d_lowerunpegged"))
	{
		binds.clear("3d_lowerunpegged");
		toggle_texture_peg_3d(false);
	}

	// Change light level
	if (binds.pressed("3d_uplightlevel"))
	{
		binds.clear("3d_uplightlevel");
		change_light_3d(16);
	}

	if (binds.pressed("3d_downlightlevel"))
	{
		binds.clear("3d_downlightlevel");
		change_light_3d(-16);
	}

	// Change thing angle
	if (binds.pressed("3d_upthingangle") && key_3d_allow)
	{
		change_thing_angle_3d(45);
		key_3d_rep = key_delay_3d;
	}

	if (binds.pressed("3d_downthingangle") && key_3d_allow)
	{
		change_thing_angle_3d(-45);
		key_3d_rep = key_delay_3d;
	}

	// Change thing z height
	if (map.hexen)
	{
		if (binds.pressed("3d_upthingz8") && key_3d_allow)
		{
			change_thing_z_3d(8);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_downthingz8") && key_3d_allow)
		{
			change_thing_z_3d(-8);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_upthingz") && key_3d_allow)
		{
			change_thing_z_3d(1);
			key_3d_rep = key_delay_3d;
		}

		if (binds.pressed("3d_downthingz") && key_3d_allow)
		{
			change_thing_z_3d(-1);
			key_3d_rep = key_delay_3d;
		}
	}

	if (binds.pressed("3d_align_tex_x"))
	{
		binds.clear("3d_align_tex_x");
		auto_align_x_3d();
	}

	if (binds.pressed("3d_reset_offsets"))
	{
		binds.clear("3d_reset_offsets");
		reset_offsets_3d();
	}

	// Change (browse) texture
	if (binds.pressed("3d_change_texture"))
	{
		binds.clear("3d_change_texture");
		change_texture_3d(false);
	}

	// Copy/Paste texture
	if (binds.pressed("3d_copy_texture"))
	{
		binds.clear("3d_copy_texture");
		copy_texture_3d();
	}

	if (binds.pressed("3d_paste_texture"))
	{
		binds.clear("3d_paste_texture");
		paste_texture_3d(false);
	}

	if (binds.pressed("3d_paste_paint"))
	{
		binds.clear("3d_paste_paint");
		paste_texture_3d(true);
	}

	if (binds.pressed("3d_copy_side"))
	{
		binds.clear("3d_copy_side");
		copy_side_3d();
	}

	if (binds.pressed("3d_paste_side"))
	{
		binds.clear("3d_paste_side");
		paste_side_3d();
	}

	key_3d_rep--;
	if (key_3d_rep < 0)
		key_3d_rep = 0;

	return true;
}
