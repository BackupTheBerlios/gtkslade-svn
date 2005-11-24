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
	keys.clear();

	add("view_up", "Up");
	add("view_down", "Down");
	add("view_left", "Left");
	add("view_right", "Right");
	add("view_zoomin", "equal");
	add("view_zoomout", "minus");
	add("view_origin", "o");
	add("view_increasegrid", "bracketleft");
	add("view_decreasegrid", "bracketright");
	add("view_3dmode", "q");

	add("mode_vertices", "v");
	add("mode_linedefs", "l");
	add("mode_sectors", "s");
	add("mode_things", "t");
	add("mode_change", "Tab");

	add("edit_lockhilight", "h");
	add("edit_clearselection", "c");
	add("edit_edititem", "Return");
	add("edit_createitem", "Insert");
	//add("edit_createitem2", "<Shift>Insert");
	add("edit_deleteitem", "Delete");
	add("edit_undo", "<Control>z");

	add("line_flip", "f");
	add("line_swapsides", "<Control>f");
	add("line_flipboth", "<Shift><Control>f");
	add("line_begindraw", "space");
	add("line_begindraw_rect", "<Shift>space");

	add("sector_upfloor8", "period");
	add("sector_downfloor8", "comma");
	add("sector_upceil8", "<Shift>period");
	add("sector_downceil8", "<Shift>comma");
	add("sector_upfloor", "<Control>period");
	add("sector_downfloor", "<Control>comma");
	add("sector_upceil", "<Shift><Control>period");
	add("sector_downceil", "<Shift><Control>comma");
	add("sector_upboth8", "<Alt>period");
	add("sector_downboth8", "<Alt>comma");
	add("sector_upboth", "<Control><Alt>period");
	add("sector_downboth", "<Control><Alt>comma");
	add("sector_merge", "<Control>j");
	add("sector_join", "j");

	add("copy", "<Control>c");
	add("paste", "<Control>v");
	add("cancel_paste", "Escape");

	add("3d_exit", "q");

	add("3d_forward", "w");
	add("3d_back", "s");
	add("3d_strafeleft", "a");
	add("3d_straferight", "d");
	add("3d_left", ",");
	add("3d_right", ".");
	add("3d_moveup", "Home");
	add("3d_movedown", "End");

	add("3d_toggle_gravity", "g");
	add("3d_toggle_fullbright", "b");
	add("3d_toggle_fog", "f");
	add("3d_toggle_hilight", "h");
	add("3d_toggle_things", "t");

	add("3d_upceil8", "KP_Add");
	add("3d_downceil8", "KP_Subtract");
	add("3d_upceil", "<Control>KP_Add");
	add("3d_downceil", "<Control>KP_Subtract");
	add("3d_upfloor8", "<Shift>KP_Add");
	add("3d_downfloor8", "<Shift>KP_Subtract");
	add("3d_upfloor", "<Shift><Control>KP_Add");
	add("3d_downfloor", "<Shift><Control>KP_Subtract");
	add("3d_upboth8", "<Alt>KP_Add");
	add("3d_downboth8", "<Alt>KP_Subtract");
	add("3d_upboth", "<Control><Alt>KP_Add");
	add("3d_downboth", "<Control><Alt>KP_Subtract");

	add("3d_upyoffset", "<Control>Up");
	add("3d_downyoffset", "<Control>Down");
	add("3d_upxoffset", "<Control>Left");
	add("3d_downxoffset", "<Control>Right");
	add("3d_upyoffset8", "Up");
	add("3d_downyoffset8", "Down");
	add("3d_upxoffset8", "Left");
	add("3d_downxoffset8", "Right");

	add("3d_lowerunpegged", "l");
	add("3d_upperunpegged", "u");

	add("3d_uplightlevel", "equal");
	add("3d_downlightlevel", "minus");

	add("3d_upthingangle", "Left");
	add("3d_downthingangle", "Right");

	add("3d_upthingz8", "Up");
	add("3d_downthingz8", "Down");
	add("3d_upthingz", "<Control>Up");
	add("3d_downthingz", "<Control>Down");

	add("3d_align_tex_x", "<Control>a");
	add("3d_paste_paint", "<Shift>p");
	add("3d_reset_offsets", "r");

	/*
	add("3d_clear_texture", key_id("KEY_DELETE"), 0);
	*/

	add("open_console", "grave");
}

// add: Adds a key binding to the bind list
// ------------------------------------- >>
void BindList::add(string name, string key)
{
	keybind_t newkey;
	newkey.key = key;
	newkey.defaultkey = key;
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

// press: 'Presses' a bind (wether the key is pressed or not)
// ------------------------------------------------------- >>
void BindList::press(string name)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
		{
			keys[a].pressed = true;
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

// get_name: Gets the name of a specific bind
// --------------------------------------- >>
string BindList::get_name(int index)
{
	if (index < keys.size())
		return keys[index].name;
	else
		return "";
}

// get_bind: Gets the key of a specific bind
// -------------------------------------- >>
string BindList::get_bind(int index)
{
	if (index < keys.size())
		return keys[index].key;
	else
		return "";
}

// change: Changes a key binding
// -------------------------- >>
void BindList::change(string name, string key)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
			keys[a].key = key;
	}
}

// change_default: Changes a key binding to it's default setting
// ---------------------------------------------------------- >>
void BindList::change_default(string name)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
			keys[a].key = keys[a].defaultkey;
	}
}

void BindList::save(FILE* fp)
{
	fprintf(fp, "binds\n{\n");

	for (int a = 0; a < keys.size(); a++)
		fprintf(fp, "\tkey \"%s\" \"%s\"\n", keys[a].name.c_str(), keys[a].key.c_str());

	fprintf(fp, "}\n\n");
}

void BindList::load(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "key")
		{
			string name = tz->get_token();
			string key = tz->get_token();

			for (int a = 0; a < keys.size(); a++)
			{
				if (keys[a].name == name)
				{
					keys[a].key = key;
					break;
				}
			}
		}

		token = tz->get_token();
	}
}
