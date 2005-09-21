
#include "main.h"
#include "console.h"

GtkTextBuffer	*console_log;
string			cmd_line;
vector<string>	cmd_history;

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

void console_print(string message)
{
	if (message[message.size() - 1] != '\n')
		message += "\n";

	GtkTextIter end;
	gtk_text_buffer_get_end_iter(console_log, &end);
	gtk_text_buffer_insert(console_log, &end, message.c_str(), -1);
}
