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

CVAR(Float, move_speed_3d, 0.3f, CVAR_SAVE)
CVAR(Float, mouse_speed_3d, 1.0f, CVAR_SAVE)

// External Variables --------------------- >>
extern BindList binds;
extern Map map;

extern int xoff, yoff, edit_mode, hilight_item;
extern float zoom;
extern bool line_draw;
extern point2_t mouse;
extern rect_t sel_box;

extern Camera camera;

EXTERN_CVAR(Bool, render_fog)
EXTERN_CVAR(Bool, render_fullbright)

// cycle_edit_mode: Cycles the edit mode
// ----------------------------------- >>
void cycle_edit_mode()
{
	if (++edit_mode > 3)
		edit_mode = 0;
}

// edit_item: When the edit item key is pressed
// ----------------------------------------- >>
void edit_item()
{
	if (edit_mode == 0)
	{
		create_vertex();
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

// keys_edit: Keys for the 2d editor
// ------------------------------ >>
void keys_edit()
{
	if (!map.opened)
		return;

	// Scroll up
	if (binds.pressed("view_up"))
	{
		yoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}
	
	// Scroll down
	if (binds.pressed("view_down"))
	{
		yoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}

	// Scroll left
	if (binds.pressed("view_left"))
	{
		xoff += ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}
	
	// Scroll right
	if (binds.pressed("view_right"))
	{
		xoff -= ((MAJOR_UNIT / (int)zoom)) + 1;
		force_map_redraw(true, true);
	}

	// Zoom in
	if (binds.pressed("view_zoomin"))
	{
		zoom *= 1.2f;

		if (zoom > 1024)
			zoom = 1024;

		force_map_redraw(true, true);
	}
	
	// Zoom out
	if (binds.pressed("view_zoomout"))
	{
		zoom *= 0.8f;

		if (zoom < 1)
			zoom = 1;

		force_map_redraw(true, true);
	}

	// Center view on mouse
	if (binds.pressed("view_mousecenter"))
	{
		xoff = -m_x(mouse.x) / MAJOR_UNIT;
		yoff = -m_y(mouse.y) / MAJOR_UNIT;
		force_map_redraw(true, true);
	}

	// Set offsets to 0, 0
	if (binds.pressed("view_origin"))
	{
		xoff = yoff = 0;
		force_map_redraw(true, true);
	}

	// Vertices mode
	if (binds.pressed("mode_vertices"))
		change_edit_mode(0);
	
	// Linedefs mode
	if (binds.pressed("mode_linedefs"))
		change_edit_mode(1);
	
	// Sectors mode
	if (binds.pressed("mode_sectors"))
		change_edit_mode(2);
	
	// Things mode
	if (binds.pressed("mode_things"))
		change_edit_mode(3);

	// Change mode
	if (binds.pressed("mode_change"))
		cycle_edit_mode();

	// Increase grid size
	if (binds.pressed("view_increasegrid"))
	{
		increase_grid();
		force_map_redraw(false, true);
	}

	// Decrease grid size
	if (binds.pressed("view_decreasegrid"))
	{
		decrease_grid();
		force_map_redraw(false, true);
	}

	// Clear selection
	if (binds.pressed("edit_clearselection"))
	{
		clear_selection();
		force_map_redraw(true);
	}

	// Delete item
	if (binds.pressed("edit_deleteitem"))
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
	if (binds.pressed("edit_createitem"))
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

		binds.clear("edit_createitem");
	}

	// Sector height quick changes (8 units)
	if (binds.pressed("sector_upfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 8);
	}

	if (binds.pressed("sector_downfloor8"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -8);
	}

	if (binds.pressed("sector_upceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 8);
	}

	if (binds.pressed("sector_downceil8"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -8);
	}

	if (binds.pressed("sector_upboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 8);
			sector_changeheight(false, 8);
		}
	}

	if (binds.pressed("sector_downboth8"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -8);
			sector_changeheight(false, -8);
		}
	}

	// Sector height quick changes (1 unit)
	if (binds.pressed("sector_upfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, 1);
	}

	if (binds.pressed("sector_downfloor"))
	{
		if (edit_mode == 2)
			sector_changeheight(true, -1);
	}

	if (binds.pressed("sector_upceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, 1);
	}

	if (binds.pressed("sector_downceil"))
	{
		if (edit_mode == 2)
			sector_changeheight(false, -1);
	}

	if (binds.pressed("sector_upboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, 1);
			sector_changeheight(false, 1);
		}
	}

	if (binds.pressed("sector_downboth"))
	{
		if (edit_mode == 2)
		{
			sector_changeheight(true, -1);
			sector_changeheight(false, -1);
		}
	}

	// Flip line
	if (binds.pressed("line_flip"))
	{
		if (edit_mode == 1)
			line_flip(true, false);

		force_map_redraw(true);
	}

	// Swap line sides
	if (binds.pressed("line_swapsides"))
	{
		if (edit_mode == 1)
			line_flip(false, true);

		force_map_redraw(true);
	}

	// Flip both line direction and sides
	if (binds.pressed("line_flipboth"))
	{
		if (edit_mode == 1)
			line_flip(true, true);

		force_map_redraw(true);
	}

	// Begin line draw
	if (binds.pressed("line_begindraw"))
	{
		if (!line_draw)
			line_draw = true;

		binds.clear("line_begindraw");
	}

	// Begin rectangle draw
	if (binds.pressed("line_begindraw_rect"))
	{
		if (!line_draw)
		{
			line_draw = true;
			sel_box.set(mouse.x, mouse.y, mouse.x, mouse.y);
		}

		binds.clear("line_begindraw_rect");
	}

	// Undo
	if (binds.pressed("edit_undo"))
	{
		undo();
		clear_selection();
		hilight_item = -1;
		force_map_redraw(true, true);
		map_changelevel(3);
		binds.clear("edit_undo");
	}

	// Edit item
	if (binds.pressed("edit_edititem"))
	{
		edit_item();
		binds.clear("edit_edititem");
	}

	// Merge sectors
	if (binds.pressed("sector_merge"))
	{
		sector_merge(false);
		binds.clear("sector_merge");
	}

	// Join sectors
	if (binds.pressed("sector_join"))
	{
		sector_merge(true);
		binds.clear("sector_join");
	}

	if (binds.pressed("view_3dmode"))
	{
		binds.clear("view_3dmode");
		binds.clear("3d_exit");
		start_3d_mode();
	}
}

bool keys_3d()
{
	float speed = 0.2 * move_speed_3d;

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

	return true;
}
