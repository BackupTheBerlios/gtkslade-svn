
#include "main.h"

CVAR(Int, font_size, 0, CVAR_SAVE);

void remove_duplicates_intvector(vector<int> *vec)
{
	vector<int> uniques;

	vector<int>::iterator iter = vec->begin();

	while (iter != vec->end());
	{
		if (vector_exists(uniques, *iter))
			vec->erase(iter);
		else
		{
			uniques.push_back(*iter);
			iter++;
		}
	}
}

string parse_string(char *str, ...)
{
	char text[512] = "";
	va_list ap;

	va_start(ap, str);
	vsprintf(text, str, ap);
	va_end(ap);

	string ret = text;
	return ret;
}

void widget_set_font(GtkWidget *w, string font, int size)
{
	size += font_size;
	string fontstring = parse_string("%s %d", font.c_str(), size);
	gtk_widget_modify_font(w, pango_font_description_from_string(fontstring.c_str()));
}

string bool_yesno(bool val)
{
	if (val)
		return "Yes";
	else
		return "No";
}

void set_cursor(int x, int y)
{
	gdk_display_warp_pointer(gdk_display_get_default(), gdk_screen_get_default(), x, y);
}

guint get_keyval(GdkEventKey *event)
{
	int n_entries = 0;
	guint *vals = 0;

	gdk_keymap_get_entries_for_keycode(gdk_keymap_get_default(),
										event->hardware_keycode,
										NULL,
										&vals,
										&n_entries);

	return vals[0];
}
