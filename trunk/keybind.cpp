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
	add("edit_moveitems", "Mouse3");
	add("edit_selectitem", "Mouse1");
	add("edit_selectbox", "Mouse1", KMOD_SHIFT);
	add("edit_undo", "z", KMOD_CTRL);
	add("edit_gridsnap", "g");

	add("line_flip", "f");
	add("line_swapsides", "f", KMOD_CTRL);
	add("line_flipboth", "f", KMOD_SHIFT|KMOD_CTRL);
	add("line_begindraw", "space");
	add("line_begindraw_rect", "space", KMOD_SHIFT);

	add("sector_upfloor8", "period");
	add("sector_downfloor8", "comma");
	add("sector_upceil8", "period", KMOD_SHIFT);
	add("sector_downceil8", "comma", KMOD_SHIFT);
	add("sector_upfloor", "period", KMOD_CTRL);
	add("sector_downfloor", "comma", KMOD_CTRL);
	add("sector_upceil", "period", KMOD_SHIFT|KMOD_CTRL);
	add("sector_downceil", "comma", KMOD_SHIFT|KMOD_CTRL);
	add("sector_upboth8", "period", KMOD_ALT);
	add("sector_downboth8", "comma", KMOD_ALT);
	add("sector_upboth", "period", KMOD_CTRL|KMOD_ALT);
	add("sector_downboth", "comma", KMOD_CTRL|KMOD_ALT);

	add("sector_uplight", "apostrophe");
	add("sector_downlight", "semicolon");

	add("sector_merge", "j", KMOD_CTRL);
	add("sector_join", "j");

	add("thing_quickangle", "Mouse2");

	add("copy", "c", KMOD_CTRL);
	add("paste", "v", KMOD_CTRL);
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
	add("3d_upceil", "KP_Add", KMOD_CTRL);
	add("3d_downceil", "KP_Subtract", KMOD_CTRL);
	add("3d_upfloor8", "KP_Add", KMOD_SHIFT);
	add("3d_downfloor8", "KP_Subtract", KMOD_SHIFT);
	add("3d_upfloor", "KP_Add", KMOD_SHIFT|KMOD_CTRL);
	add("3d_downfloor", "KP_Subtract", KMOD_SHIFT|KMOD_CTRL);
	add("3d_upboth8", "KP_Add", KMOD_ALT);
	add("3d_downboth8", "KP_Subtract", KMOD_ALT);
	add("3d_upboth", "KP_Add", KMOD_ALT|KMOD_CTRL);
	add("3d_downboth", "KP_Subtract", KMOD_ALT|KMOD_CTRL);

	add("3d_upyoffset", "Up", KMOD_CTRL);
	add("3d_downyoffset", "Down", KMOD_CTRL);
	add("3d_upxoffset", "Left", KMOD_CTRL);
	add("3d_downxoffset", "Right", KMOD_CTRL);
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
	add("3d_upthingz", "Up", KMOD_CTRL);
	add("3d_downthingz", "Down", KMOD_CTRL);

	add("3d_align_tex_x", "a", KMOD_CTRL);
	add("3d_reset_offsets", "r");

	add("3d_change_texture", "Mouse1");
	add("3d_copy_texture", "Mouse2");
	add("3d_paste_texture", "Mouse3");
	add("3d_paste_paint", "Mouse3", KMOD_SHIFT);

	//add("3d_clear_texture", key_id("KEY_DELETE"), 0);

	add("open_console", "grave");
}

// add: Adds a key binding to the bind list
// ------------------------------------- >>
void BindList::add(string name, string key, BYTE mods)
{
	keybind_t newkey;
	newkey.key = key;
	newkey.defaultkey = key;
	newkey.name = name;
	newkey.pressed = false;
	newkey.mods = mods;
	newkey.defaultmods = mods;

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
void BindList::set(string key, GdkModifierType mods, vector<string> *list)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].key == key)
		{
			bool pressed = true;

			if (keys[a].mods & KMOD_SHIFT)
			{
				if (!(mods & GDK_SHIFT_MASK))
					pressed = false;
			}
			else
			{
				if (mods & GDK_SHIFT_MASK)
					pressed = false;
			}

			if (keys[a].mods & KMOD_CTRL)
			{
				if (!(mods & GDK_CONTROL_MASK))
					pressed = false;
			}
			else
			{
				if (mods & GDK_CONTROL_MASK)
					pressed = false;
			}

			if (keys[a].mods & KMOD_ALT)
			{
				if (!(mods & GDK_MOD1_MASK))
					pressed = false;
			}
			else
			{
				if (mods & GDK_MOD1_MASK)
					pressed = false;
			}

			keys[a].pressed = pressed;

			if (pressed)
			{
				if (list)
					list->push_back(keys[a].name);

				//printf("Key \"%s\" pressed\n", keys[a].name.c_str());
			}
		}
	}
}

// unset: 'Releases' a key
// -------------------- >>
void BindList::unset(string key, GdkModifierType mods, vector<string> *list)
{
	//guint nkey = 0;
	//gtk_accelerator_parse(key.c_str(), &nkey, NULL);
	//string kname = gtk_accelerator_name(nkey, (GdkModifierType)0);

	for (int a = 0; a < keys.size(); a++)
	{
		//if (g_str_has_suffix(keys[a].key.c_str(), kname.c_str()))
		if (keys[a].key == key)
		{
			bool prev = keys[a].pressed;
			//printf("Key \"%s\" released\n", keys[a].name.c_str());
			keys[a].pressed = false;

			if (list && prev)
				list->push_back(keys[a].name);
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

/*
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
*/

// change: Changes a key binding
// -------------------------- >>
void BindList::change(string name, string key, BYTE mods)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
		{
			keys[a].key = key;
			keys[a].mods = mods;
		}
	}
}

// change_default: Changes a key binding to it's default setting
// ---------------------------------------------------------- >>
void BindList::change_default(string name)
{
	for (int a = 0; a < keys.size(); a++)
	{
		if (keys[a].name == name)
		{
			keys[a].key = keys[a].defaultkey;
			keys[a].mods = keys[a].defaultmods;
		}
	}
}

void BindList::save(FILE* fp)
{
	fprintf(fp, "key_binds\n{\n");

	for (int a = 0; a < keys.size(); a++)
		fprintf(fp, "\tkey \"%s\" \"%s\" %d\n", keys[a].name.c_str(), keys[a].key.c_str(), keys[a].mods);

	fprintf(fp, "}\n\n");
}

void BindList::load(Tokenizer *tz)
{
	vector<string> loaded_keys;

	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "key")
		{
			string name = tz->get_token();
			string key = tz->get_token();
			BYTE mods = tz->get_integer();

			if (vector_exists(loaded_keys, name))
				add(name, key, mods);
			else
			{
				for (int a = 0; a < keys.size(); a++)
				{
					if (keys[a].name == name)
					{
						keys[a].key = key;
						keys[a].mods = mods;
						loaded_keys.push_back(name);
						break;
					}
				}
			}
		}

		token = tz->get_token();
	}
}

// get_bind: Gets the key of a specific bind
// -------------------------------------- >>
keybind_t* BindList::get_bind(int index)
{
	if (index < keys.size())
		return &keys[index];
	else
		return NULL;
}
