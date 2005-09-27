
#include "main.h"

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

void widget_set_font(GtkWidget *w, string font)
{
	gtk_widget_modify_font(w, pango_font_description_from_string(font.c_str()));
}

string bool_yesno(bool val)
{
	if (val)
		return "Yes";
	else
		return "No";
}
