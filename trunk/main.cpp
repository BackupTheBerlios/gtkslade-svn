
#include "main.h"
#include "map.h"
#include "main_window.h"
#include "editor_window.h"
#include "console.h"
#include "colours.h"

CVAR(String, col_config, "Default", CVAR_SAVE)

extern WadList wads;
extern Map map;

// str_upper: Returns a string in uppercase
// ------------------------------------- >>
string str_upper(string str)
{
	DWORD len = 0;
	char* ustr = NULL;

	for (DWORD c = 0; c < str.length(); c++)
		len++;

	ustr = (char *)malloc(len + 1);

	for (DWORD c = 0; c < str.length(); c++)
		ustr[c] = str[c];

	ustr[len] = 0;

	string ret = g_ascii_strup(ustr, -1);
	free(ustr);
	return ret;
}

// log_message: Prints a message to the log file
// ------------------------------------------ >>
void log_message(char *message, ...)
{
	char string[512] = "";
	va_list ap;

	va_start(ap, message);
	vsprintf(string, message, ap);
	va_end(ap);

	FILE *fp = fopen("slade.log", "a");
	fprintf(fp, string);
	fclose(fp);
}

// print: Same as printf, but has the option to print to the logfile too
// ------------------------------------------------------------------ >>
void print(bool debug, char *message, ...)
{
	char string[512] = "";
	va_list ap;

	va_start(ap, message);
	vsprintf(string, message, ap);
	va_end(ap);

	if (!debug)
		printf(string);

	log_message(string);
}

// message_box: Pops up a message box
// ------------------------------- >>
void message_box(string message, GtkMessageType type)
{
	GtkWidget *msgbox = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, type, GTK_BUTTONS_CLOSE, message.c_str());
	gtk_dialog_run (GTK_DIALOG(msgbox));
	gtk_widget_destroy (msgbox);
}

// yesno_box: Pops up a yes/no box, returns true on yes
// ------------------------------------------------- >>
bool yesno_box(string message)
{
	GtkWidget *msgbox = gtk_message_dialog_new(NULL,
												GTK_DIALOG_MODAL,
												GTK_MESSAGE_QUESTION,
												GTK_BUTTONS_YES_NO,
												message.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(msgbox));
	gtk_widget_destroy (msgbox);

	if (response == GTK_RESPONSE_YES)
		return true;
	else
		return false;
}

// load_main_config: Loads the main SLADE configuration file
// ------------------------------------------------------ >>
void load_main_config()
{
	Tokenizer mr;

	if (!mr.open_file("slade.cfg", 0, 0))
		return;

	string token = mr.get_token();

	while (token != "!END")
	{
		if (token == "cvars")
			load_cvars(&mr);

		token = mr.get_token();
	}
}

// save_main_config: Saves the main SLADE configuration file
// ------------------------------------------------------ >>
void save_main_config()
{
	FILE *fp = fopen("slade.cfg", "wt");
	save_cvars(fp);
	fclose(fp);
}

// wait_gtk_events: Waits for all pending gtk events
// to be handled before continuing execution
// ---------------------------------------------- >>
void wait_gtk_events()
{
	while(gtk_events_pending())
		gtk_main_iteration();
}

void setup_icons()
{
	GtkIconFactory *icons = gtk_icon_factory_new();

	gtk_icon_factory_add(icons, "slade-mode-verts",
		gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_file("res/tb_verts16.png", NULL)));
	gtk_icon_factory_add(icons, "slade-mode-lines",
		gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_file("res/tb_lines16.png", NULL)));
	gtk_icon_factory_add(icons, "slade-mode-sectors",
		gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_file("res/tb_sectors16.png", NULL)));
	gtk_icon_factory_add(icons, "slade-mode-things",
		gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_file("res/tb_things16.png", NULL)));
	gtk_icon_factory_add(icons, "slade-mode-3d",
		gtk_icon_set_new_from_pixbuf(gdk_pixbuf_new_from_file("res/tb_3d16.png", NULL)));

	gtk_icon_factory_add_default(icons);
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);
	//gtk_gl_init(&argc, &argv);

	init_console();
	setup_icons();

	load_main_config();

	load_colour_configs();
	string ccfg = col_config;
	set_colour_config(ccfg);

	//wads.open_iwad("D:/Games/Doom/doom2.wad");
	wads.open_iwad("/media/hdb1/Games/Doom/doom2.wad");
	map.open(wads.get_iwad(), "MAP01");

	GtkWidget *window = open_main_window();

	if (window)
	{
		setup_editor_window();
		gtk_main();
	}
	else
		log_message("Some error occurred... Exiting SLADE\n");

	save_main_config();

	return 0;
}

#ifndef DEBUG
#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(nCmdShow, &lpCmdLine);
}
#endif
#endif
