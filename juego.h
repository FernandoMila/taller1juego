#ifndef JUEGO_H
#define JUEGO_H

#include <gtk/gtk.h>

void on_play_button_clicked(GtkWidget *widget, gpointer data);
void update_location_info(const char *location, const char *time);
void update_background_image(const char *image_path);
void on_map_button_clicked(GtkWidget *widget, gpointer data);  // Nueva funci�n para el mapa

// Declaraciones de las nuevas funciones
gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

#endif // JUEGO_H
