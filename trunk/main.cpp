// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << main.cpp - Main stuff                >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "map.h"
#include "main_window.h"
#include "editor_window.h"
#include "console.h"
#include "colours.h"
#include "textures.h"

// Variables ----------------------------- >>
CVAR(String, col_config, "Default", CVAR_SAVE)

// External Variables -------------------- >>
extern WadList wads;
extern Map map;
extern GtkWidget *editor_window;
extern GLuint font_list;

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

// file_browser: Opens up a file browser dialog and returns the path chosen
// --------------------------------------------------------------------- >>
string file_browser(string extension)
{
	string ret = "";
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, extension.c_str());
	gtk_file_filter_set_name(filter, extension.c_str());

	GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
													NULL,
													GTK_FILE_CHOOSER_ACTION_OPEN,
													GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
													NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		ret = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	gtk_widget_destroy(dialog);

	return ret;
}

void entry_box_act(GtkWidget *w, gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_ACCEPT);
}

string entry_box(string prompt)
{
	string ret = "";

	GtkWidget *dialog = gtk_dialog_new_with_buttons(prompt.c_str(),
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	GtkWidget *label = gtk_label_new(prompt.c_str());
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, false, false, 0);
	GtkWidget *entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(entry_box_act), dialog);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry, false, false, 0);
	gtk_widget_show_all(dialog);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_ACCEPT)
		ret = gtk_entry_get_text(GTK_ENTRY(entry));

	gtk_widget_destroy(dialog);
	return ret;
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

	// Init logfile
	FILE* f = fopen("slade.log", "wt");
	fclose(f);

	init_console();
	setup_icons();
	init_textures();

	load_main_config();

	load_colour_configs();
	string ccfg = col_config;
	set_colour_config(ccfg);

	setup_editor_window();
	setup_main_window();
	open_main_window();

	// Setup gl font
	font_list = glGenLists(256);
	PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 10");
	gdk_gl_font_use_pango_font(font_desc, 0, 255, font_list);

	gtk_main();
	
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
