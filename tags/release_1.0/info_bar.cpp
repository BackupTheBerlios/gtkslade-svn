
#include "main.h"
#include "info_bar.h"
#include "misc.h"

GtkWidget *infobar;

extern int edit_mode;

GtkWidget *get_info_bar()
{
	GtkWidget *temp = NULL;

	infobar = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(infobar), false);

	// Lines
	temp = gtk_label_new("Line info");
	gtk_notebook_append_page(GTK_NOTEBOOK(infobar), get_line_info_bar(), NULL);

	// Vertices
	temp = gtk_label_new("Vertex info");
	gtk_notebook_append_page(GTK_NOTEBOOK(infobar), get_vertex_info_bar(), NULL);

	// Sectors
	temp = gtk_label_new("Sector info");
	gtk_notebook_append_page(GTK_NOTEBOOK(infobar), get_sector_info_bar(), NULL);

	// Things
	temp = gtk_label_new("Thing info");
	gtk_notebook_append_page(GTK_NOTEBOOK(infobar), get_thing_info_bar(), NULL);

	return infobar;
}

void change_infobar_page()
{
	int mode;

	// Swap line and vertex mode numbers (so that the infobar page can default to lines)
	if (edit_mode == 0)
		mode = 1;
	else if (edit_mode == 1)
		mode = 0;
	else
		mode = edit_mode;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(infobar), mode);
}

void setup_label(GtkWidget **label, char* text)
{
	*label = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(*label), 0.0, 0.5);
	widget_set_font(*label, "Sans", 8);
}
