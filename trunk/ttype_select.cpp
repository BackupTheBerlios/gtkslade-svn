
#include "main.h"
#include "thing_type.h"

int current_type;

extern GtkWidget *editor_window;

void ttype_number_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	int index;
	gtk_tree_model_get(model, iter, 1, &index, -1);

	if (index == -1)
		g_object_set(renderer, "text", "", NULL);
	else
		g_object_set(renderer, "text", parse_string("%d", index).c_str(), NULL);
}

void ttype_tree_view_changed(GtkTreeView *view, gpointer data)
{
	GtkTreeIter			iter;
	GtkTreeSelection	*selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	GtkTreeModel		*model = gtk_tree_view_get_model(view);

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		int index;
		gtk_tree_model_get(model, &iter, 1, &index, -1);

		if (index >= 0)
			current_type = index;
	}
}

GtkWidget* setup_ttype_select(int type)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;

	// Setup tree model
	GtkTreeStore *tree_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	populate_tree_store_ttypes(tree_store);

	// Setup scrolled window
	GtkWidget *s_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	// Setup tree view
	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "#");
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(col, renderer, ttype_number_func, NULL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	//gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), false);
	g_signal_connect(G_OBJECT(tree_view), "cursor-changed", G_CALLBACK(ttype_tree_view_changed), NULL);
	gtk_container_add(GTK_CONTAINER(s_window), tree_view);

	g_object_unref(tree_store);

	return s_window;
}

int open_ttype_select_dialog(int type)
{
	current_type = type;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Thing Type",
													GTK_WINDOW(editor_window),
													GTK_DIALOG_MODAL,
													GTK_STOCK_OK,
													GTK_RESPONSE_ACCEPT,
													GTK_STOCK_CANCEL,
													GTK_RESPONSE_REJECT,
													NULL);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), setup_ttype_select(type));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_set_size_request(dialog, 320, 360);
	gtk_widget_show_all(dialog);

	int ret = type;
	int response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
		ret = current_type;

	gtk_widget_destroy(dialog);

	return ret;
}
