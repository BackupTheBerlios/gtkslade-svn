
#include "main.h"
#include "action_special.h"
#include "misc.h"

int current_special;

extern GtkWidget *editor_window;

void special_number_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	/*
	guint  year_born;
	guint  year_now = 2003;
	gchar  buf[64];

	gtk_tree_model_get(model, iter, COL_YEAR_BORN, &year_born, -1);

	if (year_born <= year_now && year_born > 0)
	{
		guint age = year_now - year_born;

		g_snprintf(buf, sizeof(buf), "%u years old", age);

		g_object_set(renderer, "foreground-set", FALSE, NULL);
	}
	else
	{
		g_snprintf(buf, sizeof(buf), "age unknown");

		g_object_set(renderer, "foreground", "Red", "foreground-set", TRUE, NULL);
	}

	g_object_set(renderer, "text", buf, NULL);
	*/

	int special_index;
	gtk_tree_model_get(model, iter, 1, &special_index, -1);

	if (special_index == -1)
		g_object_set(renderer, "text", "", NULL);
	else
		g_object_set(renderer, "text", parse_string("%d", special_index).c_str(), NULL);
}

void special_tree_view_changed(GtkTreeView *view, gpointer data)
{
	GtkTreeIter			iter;
	GtkTreeSelection	*selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	GtkTreeModel		*model = gtk_tree_view_get_model(view);

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		int special_index;
		gtk_tree_model_get(model, &iter, 1, &special_index, -1);

		if (special_index >= 0)
			current_special = special_index;
	}
}

GtkWidget* setup_special_select(int spec)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;

	// Setup tree model
	GtkTreeStore *tree_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	populate_tree_store_specials(tree_store);

	// Setup scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	// Setup tree view
	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Special", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "#");
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(col, renderer, special_number_func, NULL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	//gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), false);
	g_signal_connect(G_OBJECT(tree_view), "cursor-changed", G_CALLBACK(special_tree_view_changed), NULL);
	gtk_container_add(GTK_CONTAINER(s_window), tree_view);

	g_object_unref(tree_store);

	return s_window;
}

int open_special_select_dialog(int spec)
{
	current_special = spec;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Action Special",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_special_select(spec));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_set_size_request(dialog, 320, 360);
	gtk_widget_show_all(dialog);

	int ret = spec;
	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
		ret = current_special;

	gtk_widget_destroy(dialog);

	return ret;
}
