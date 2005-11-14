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
#include "textures.h"
#include "tex_browser.h"
#include "editor_window.h"
#include "splash.h"

// Variables ----------------------------- >>
GtkTextBuffer	*console_log;
string			cmd_line;
vector<string>	cmd_history;
int				console_hcmd = -1;

string qdb[] = {
	"SLADE: SlayeR's LeetAss Doom Editor",
	"SLADE: Some Lame Ancient Doom Editor",
	"Having fun playing with useless console commands?",
	"Initializing Nickbakery...",
	"Truth is beautiful, without doubt; but so are lies.",
	"Initializing Doom Builder...",
	"...And the lord said, 'lo, there shall only be case or default labels inside a switch statement'",
	"Call me paranoid but finding '/*' inside this comment makes me suspicious.",
	"My word, whats wrong with that mans bottom?",
	"Subliminal (kill) messaging (your) is (parents) awesome!",
	"Yeah it's a map editor, what of it?",
	"Installing Gator...",
	"Formatting C:\...",
	"Remember that when you reach for the stars, they are too far away, so it's hopeless.",
	"http://slade.mancubus.net -- FREE PORN!!",
	"You're trying to say you like DOS better than me right? (Press Y to quit)",
	"I've hidden some pr0n somewhere in the SLADE resources, find it and win a prize!",
	"\"splash hide\" in the console to hide this, by the way.",
};
int p_q = 0;
int n_quotes = sizeof(qdb) / sizeof(string);

// External Variables -------------------- >>
extern vector<Texture*>	textures;
extern vector<Texture*>	flats;

void console_prevcommand()
{
	if (console_hcmd != -1)
		console_hcmd--;

	if (console_hcmd == -1)
	{
		if (cmd_history.size() > 0)
			console_hcmd = cmd_history.size() - 1;
	}

	if (console_hcmd != -1)
		cmd_line = cmd_history[console_hcmd];
}

void console_nextcommand()
{
	if (console_hcmd == -1)
		return;

	if (console_hcmd != -1)
		console_hcmd++;

	if (console_hcmd >= cmd_history.size())
	{
		cmd_line = "";
		console_hcmd = -1;
	}
	else
		cmd_line = cmd_history[console_hcmd];
}

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
	cmd_history.push_back(cmd_line);

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
			}

			// If a map display-related cvar is changed
			if (cvar->name == "col_config" ||
				cvar->name == "grid_dashed" ||
				cvar->name == "thing_sprites")
				force_map_redraw(true, true);

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

	// "tex_browse" command
	if (token == "tex_browse")
	{
		string stex = open_texture_browser(true, true, true);
		console_print(parse_string("Selected \"%s\"", stex.c_str()));
		parsed = true;
	}

	// "splash" command
	// If msg isn't specified, a random quote is shown instead
	if (token == "splash")
	{
		// Get a random number that isn't what was previously shown :P
		int q = p_q;
		while (q == p_q) q = g_random_int_range(0, n_quotes);
		p_q = q;

		string msg = qdb[q];
		float prog = 0.0f;

		if (tz.peek_token() != "!END")
		{
			msg = tz.get_token();

			if (tz.peek_token() != "!END")
				prog = tz.get_float();
		}

		if (msg != "hide")
		{
			splash(msg);
			splash_progress(prog);
		}
		else
			splash_hide();

		parsed = true;
	}

	// "cmdlist" command
	if (token == "cmdlist")
	{
		console_print("Available Commands:");
		console_print("cmdlist");
		console_print("cvarlist");
		console_print("dump_flats");
		console_print("dump_textures");
		console_print("splash");
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
	console_hcmd = -1;
}
