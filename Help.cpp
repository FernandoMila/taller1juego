#include "help.h"
#include <iostream>

int current_image_index = 0;
const char* image_filenames[] = {
    "imagenes/help1.png",
    "imagenes/help2.png",
    "imagenes/help3.png"
};
const int num_images = sizeof(image_filenames) / sizeof(image_filenames[0]);
GtkWidget *image_widget;

void update_help_image() {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(image_filenames[current_image_index], 780, 540, TRUE, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), pixbuf);
    if (pixbuf) {
        g_object_unref(pixbuf);
    }
}

void on_left_button_clicked(GtkWidget *widget, gpointer data) {
    if (current_image_index > 0) {
        current_image_index--;
        update_help_image();
    }
}

void on_right_button_clicked(GtkWidget *widget, gpointer data) {
    if (current_image_index < num_images - 1) {
        current_image_index++;
        update_help_image();
    }
}

void on_help_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ayuda", NULL, GTK_DIALOG_MODAL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 600);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='24' color='blue'><b>Ayuda</b></span>");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);

    image_widget = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(vbox), image_widget, TRUE, TRUE, 0);
    update_help_image();

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *left_button = gtk_button_new_with_label("←");
    GtkWidget *right_button = gtk_button_new_with_label("→");
    GtkWidget *close_button = gtk_button_new_with_label("Cerrar");

    g_signal_connect(left_button, "clicked", G_CALLBACK(on_left_button_clicked), NULL);
    g_signal_connect(right_button, "clicked", G_CALLBACK(on_right_button_clicked), NULL);
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), dialog);

    gtk_box_pack_start(GTK_BOX(hbox), left_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), right_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), close_button, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
}
