#include <gtk/gtk.h>

void end_program(GtkWidget *wid, gpointer ptr)
{
    gtk_main_quit();
}

void console_printf(GtkWidget *wid, gpointer ptr)
{
    printf("button pressed\n");
}

void main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade-file.glade", NULL);

    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "io1_on"));

    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show_all(window);

    gtk_main();
}