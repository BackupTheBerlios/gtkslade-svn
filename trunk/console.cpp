// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << console.cpp - Console functions      >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "console.h"
#include "colours.h"
#include "misc.h"
#include "textures.h"
#include "tex_browser.h"
#include "editor_window.h"

// Variables ----------------------------- >>
GtkTextBuffer	*console_log;
string			cmd_line;
vector<string>	cmd_history;

// External Variables -------------------- >>
extern vector<Texture*>	textures;
extern vector<Texture*>	flats;

// init_console: Initialises the console
// ---------------------------------- >>
void init_console()
{
	console_log = gtk_text_buffer_new(NULL);
	setup_console_window();

	console_print("<< ------------------------------------------------------------------ >>\n");
	console_print("<< S.L.A.D.E. -- \"SlayeR's LeetAss Doom Editor\" (v1.0 beta)           >>\n");
	console_print("<< ------------------------------------------------------------------ >>\n");
	console_print("<< By Simon \"SlayeR\" Judd, 2005                                       >>\n");
	console_print("<< web: \"http://slade.mancubus.net\" email: \"slayer@mancubus.net\"      >>\n");
	console_print("<< ------------------------------------------------------------------ >>\n");
	console_print("<< Type 'cmdlist' for a list of valid commands.                       >>\n");
	console_print("<< ------------------------------------------------------------------ >>\n\n");
}

// console_print: Prints a message to the console
// ------------------------------------------- >>
void console_print(string message)
{
	if (message[message.size() - 1] != '\n')
		message += "\n";

	GtkTextIter end;
	gtk_text_buffer_get_end_iter(console_log, &end);
	gtk_text_buffer_insert(console_log, &end, message.c_str(), -1);
}

// console_parsecommand: Parses the current console command
// ----------------------------------------------------- >>
void console_parsecommand()
{
	if (cmd_line == "")
		return;

	bool parsed = false;
	Tokenizer tz;
	tz.open_string(cmd_line, 0, 0);

	string token = tz.get_token();

	console_print(cmd_line);

	// Check for cvar
	CVar* cvar = get_cvar(token);
	if (cvar)
	{
		bool changed = false;

		// Check if we want to change the value
		if (tz.peek_token() != "!END")
		{
			if (cvar->type == CVAR_INTEGER)
				*((CIntCVar *)cvar) = tz.get_integer();

			if (cvar->type == CVAR_BOOLEAN)
				*((CBoolCVar *)cvar) = !!(tz.get_integer());

			if (cvar->type == CVAR_FLOAT)
				*((CFloatCVar *)cvar) = tz.get_float();

			if (cvar->type == CVAR_STRING)
				*((CStringCVar *)cvar) = tz.get_token();

			// If col_config is changed
			if (cvar->name == "col_config")
			{
				string ccfg = ((CStringCVar *)cvar)->value;
				set_colour_config(ccfg);
				console_print(parse_string("Colour config \"%s\" applied", ccfg.c_str()));
				force_map_redraw(true, true);
			}

			changed = true;
		}

		// Display the value
		string msg = "- \"";
		msg += cvar->name;

		if (changed)
			msg += "\" set to ";
		else
			msg += "\" is ";

		char val[16] = "";

		if (cvar->type == CVAR_INTEGER)
			sprintf(val, "\"%d\"", cvar->GetValue().Int);

		if (cvar->type == CVAR_STRING)
			sprintf(val, "\"%s\"", ((CStringCVar *)cvar)->value.c_str());

		if (cvar->type == CVAR_BOOLEAN)
			sprintf(val, "\"%d\"", cvar->GetValue().Bool);

		if (cvar->type == CVAR_FLOAT)
			sprintf(val, "\"%1.2f\"", cvar->GetValue().Float);

		console_print(msg + val);

		parsed = true;
	}

	// "cvarlist" command
	if (token == "cvarlist")
	{
		console_print("- All CVars:");

		vector<string> l_cvars = get_cvar_list();

		for (DWORD s = 0; s < l_cvars.size(); s++)
		{
			char temp[128] = "";
			sprintf(temp, "\"%s\"", l_cvars[s].c_str());
			console_print(temp);
		}

		parsed = true;
	}

	// "dump_textures" command
	if (token == "dump_textures")
	{
		for (int a = 0; a < textures.size(); a++)
			console_print(textures[a]->name);

		parsed = true;
	}

	// "dump_flats" command
	if (token == "dump_flats")
	{
		for (int a = 0; a < flats.size(); a++)
			console_print(flats[a]->name);

		parsed = true;
	}

	if (token == "tex_browse")
	{
		open_texture_browser(false, false, false);
		parsed = true;
	}

	// Unknown command
	if (!parsed)
	{
		char temp[276] = "";
		sprintf(temp, "- Unknown Command \"%s\"", cmd_line.c_str());
		console_print(temp);
	}

	// Finish up
	cmd_line = "";
}
