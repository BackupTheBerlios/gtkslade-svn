// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << keybind.cpp - Key bindings            >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "keybind.h"

// Variables ------------------------------ >>
BindList binds;

// set_defaults: Sets the default key bindings
// ---------------------------------------- >>
void BindList::set_defaults()
{
	binds.add("view_up", "Up");
	binds.add("view_down", "Down");
	binds.add("view_left", "Left");
	binds.add("view_right", "Right");
	binds.add("view_zoomin", "equal");
	binds.add("view_zoomout", "minus");
	binds.add("view_origin", "o");
	binds.add("view_increasegrid", "bracketleft");
	binds.add("view_decreasegrid", "bracketright");
	binds.add("view_3dmode", "q");

	binds.add("mode_vertices", "v");
	binds.add("mode_linedefs", "l");
	binds.add("mode_sectors", "s");
	binds.add("mode_things", "t");
	binds.add("mode_change", "Tab");

	binds.add("edit_lockhilight", "h");
	binds.add("edit_clearselection", "c");
	binds.add("edit_edititem", "Return");
	binds.add("edit_createitem", "Insert");
	//binds.add("edit_createitem2", "<Shift>Insert");
	binds.add("edit_deleteitem", "Delete");
	binds.add("edit_undo", "<Control>z");

	binds.add("line_flip", "f");
	binds.add("line_swapsides", "<Control>f");
	binds.add("line_flipboth", "<Shift><Control>f");
	binds.add("line_begindraw", "space");
	binds.add("line_begindraw_rect", "<Shift>space");

	binds.add("sector_upfloor8", "period");
	binds.add("sector_downfloor8", "comma");
	binds.add("sector_upceil8", "<Shift>period");
	binds.add("sector_downceil8", "<Shift>comma");
	binds.add("sector_upfloor", "<Control>period");
	binds.add("sector_downfloor", "<Control>comma");
	binds.add("sector_upceil", "<Shift><Control>period");
	binds.add("sector_downceil", "<Shift><Control>comma");
	binds.add("sector_upboth8", "<Alt>period");
	binds.add("sector_downboth8", "<Alt>comma");
	binds.add("sector_upboth", "<Control><Alt>period");
	binds.add("sector_downboth", "<Control><Alt>comma");
	binds.add("sector_merge", "<Control>j");
	binds.add("sector_join", "j");

	binds.add("3d_exit", "q");

	binds.add("3d_forward", "w");
	binds.add("3d_back", "s");
	binds.add("3d_strafeleft", "a");
	binds.add("3d_straferight", "d");
	binds.add("3d_left", "Left");
	binds.add("3d_right", "Right");
	binds.add("3d_moveup", "Up");
	binds.add("3d_movedown", "Down");
	binds.add("3d_togglegravity", "g");

	/*
	binds.add("3d_2dmode", 'q', 0);
	binds.add("3d_toggle_things", 't', 0);
	binds.add("3d_toggle_fullbright", 'b', 0);

	binds.add("3d_upceil8", key_id("KEYPAD_PLUS"), 0);
	binds.add("3d_downceil8", key_id("KEYPAD_MINUS"), 0);
	binds.add("3d_upceil", key_id("KEYPAD_PLUS"), KEYMOD_CTRL);
	binds.add("3d_downceil", key_id("KEYPAD_MINUS"), KEYMOD_CTRL);
	binds.add("3d_upfloor8", key_id("KEYPAD_PLUS"), KEYMOD_SHIFT);
	binds.add("3d_downfloor8", key_id("KEYPAD_MINUS"), KEYMOD_SHIFT);
	binds.add("3d_upfloor", key_id("KEYPAD_PLUS"), KEYMOD_SHIFT|KEYMOD_CTRL);
	binds.add("3d_downfloor", key_id("KEYPAD_MINUS"), KEYMOD_SHIFT|KEYMOD_CTRL);
	binds.add("3d_upboth8", key_id("KEYPAD_PLUS"), KEYMOD_ALT);
	binds.add("3d_downboth8", key_id("KEYPAD_MINUS"), KEYMOD_ALT);
	binds.add("3d_upboth", key_id("KEYPAD_PLUS"), KEYMOD_ALT|KEYMOD_CTRL);
	binds.add("3d_downboth", key_id("KEYPAD_MINUS"), KEYMOD_ALT|KEYMOD_CTRL);

	binds.add("3d_upbrightness", '=', 0);
	binds.add("3d_downbrightness", '-', 0);

	binds.add("3d_upyoffset", key_id("KEYPAD_UP"), KEYMOD_CTRL);
	binds.add("3d_downyoffset", key_id("KEYPAD_DOWN"), KEYMOD_CTRL);
	binds.add("3d_upxoffset", key_id("KEYPAD_LEFT"), KEYMOD_CTRL);
	binds.add("3d_downxoffset", key_id("KEYPAD_RIGHT"), KEYMOD_CTRL);
	binds.add("3d_upyoffset8", key_id("KEYPAD_UP"), 0);
	binds.add("3d_downyoffset8", key_id("KEYPAD_DOWN"), 0);
	binds.add("3d_upxoffset8", key_id("KEYPAD_LEFT"), 0);
	binds.add("3d_downxoffset8", key_id("KEYPAD_RIGHT"), 0);

	binds.add("3d_upthingangle", key_id("KEYPAD_LEFT"), 0);
	binds.add("3d_downthingangle", key_id("KEYPAD_RIGHT"), 0);

	binds.add("3d_upthingz8", key_id("KEYPAD_UP"), 0);
	binds.add("3d_downthingz8", key_id("KEYPAD_DOWN"), 0);
	binds.add("3d_upthingz", key_id("KEYPAD_UP"), KEYMOD_CTRL);
	binds.add("3d_downthingz", key_id("KEYPAD_DOWN"), KEYMOD_CTRL);

	binds.add("3d_lowerunpegged", 'l', 0);
	binds.add("3d_upperunpegged", 'u', 0);
	binds.add("3d_clear_texture", key_id("KEY_DELETE"), 0);
	binds.add("3d_reset_offsets", 'r', 0);
	binds.add("3d_align_tex_x", 'a', KEYMOD_CTRL);
	*/

	binds.add("open_console", "grave");
}

// add: Adds a key binding to the bind list
// ------------------------------------- >>
void BindList::add(string name, string key)
{
	keybind_t newkey;
	newkey.key = key;
	newkey.name = name;
	newkey.pressed = false;

	keys.push_back(newkey);
}

// pressed: Returns wether a key is pressed
// ------------------------------------- >>
bool BindList::pressed(string name)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
			return keys[a].pressed;
	}

	return false;
}

// set: 'Presses' a key
// ----------------- >>
void BindList::set(string key)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].key == key)
		{
			keys[a].pressed = true;
			//printf("Key \"%s\" pressed\n", keys[a].name.c_str());
			//return;
		}
	}
}

// unset: 'Releases' a key
// -------------------- >>
void BindList::unset(string key)
{
	guint nkey = 0;
	gtk_accelerator_parse(key.c_str(), &nkey, NULL);
	string kname = gtk_accelerator_name(nkey, (GdkModifierType)0);

	for (int a = 0; a < keys.size(); a++)
	{
		if (g_str_has_suffix(keys[a].key.c_str(), kname.c_str()))
		{
			//printf("Key \"%s\" released\n", keys[a].name.c_str());
			keys[a].pressed = false;
		}
	}
}

// clear: Clears a bind (wether the key is pressed or not
// --------------------------------------------------- >>
void BindList::clear(string name)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
		{
			keys[a].pressed = false;
			return;
		}
	}
}

// clear_all: Clears all binds
// ------------------------ >>
void BindList::clear_all()
{
	for (int a = 0; a < keys.size(); a++)
		clear(keys[a].name);
}
