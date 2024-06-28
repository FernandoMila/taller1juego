#include "juego.h"
#include "detective.h"
#include <cstring>
#include "ciudad.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <iostream> // Para imprimir mensajes de depuraci�n

static const char *typing_text = "Policia al teclado por favor identificate:";
static GtkWidget *typing_label;
static GtkWidget *location_label;
static GtkWidget *background_image;
static GtkWidget *entry;
static GtkWidget *fixed_text_label;
static size_t typing_index = 0;
static Detective detective;

// Coordenadas de los botones
static struct {
    gint x;
    gint y;
} buttons[] = {
    {310, 555}, // Colonia del Sacramento
    {450, 400}, // Tarariras
    {20, 20}, //
    {600, 300},
    {300, 450},
    {100, 300},
    {500, 150},
    {700, 100},
    {150, 450},
    {250, 350},
    {350, 250},
    {450, 150},
    {550, 50},
    {650, 400},
    {750, 200}
};

static const int num_buttons = sizeof(buttons) / sizeof(buttons[0]);

// Nombres de las ciudades correspondientes a los botones
static const char *city_names[] = {
    "Colonia del Sacramento",
    "tarariras",
    "Ciudad 2",
    "Ciudad 3",
    "Ciudad 4",
    "Ciudad 5",
    "Ciudad 6",
    "Ciudad 7",
    "Ciudad 8",
    "Ciudad 9",
    "Ciudad 10",
    "Ciudad 11",
    "Ciudad 12",
    "Ciudad 13",
    "Ciudad 14"
};

// Variables para la animaci�n
static gboolean animate_line = FALSE;
static gboolean line_drawn = FALSE;
static gdouble line_x, line_y;
static gdouble target_x, target_y;
static guint animation_timeout_id;
static int last_button_index = 0;

// Lista de botones recorridos
static std::vector<int> visited_buttons;

// Opacidad de la l�nea para el efecto de desvanecimiento
static gdouble line_opacity = 1.0;
static const gdouble fade_speed = 0.02; // Velocidad de desvanecimiento

// Declaraci�n de funciones
void draw_line_with_opacity(cairo_t *cr, gdouble start_x, gdouble start_y, gdouble end_x, gdouble end_y, gdouble opacity);
static gboolean update_opacity(gpointer user_data);
gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean update_line_position(gpointer user_data);
gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

gboolean typewriter_effect(gpointer data) {
    if (typing_index < strlen(typing_text)) {
        char current_text[typing_index + 2];
        strncpy(current_text, typing_text, typing_index + 1);
        current_text[typing_index + 1] = '\0';
        gtk_label_set_text(GTK_LABEL(typing_label), current_text);
        typing_index++;
        return TRUE; // Continuar llamando a esta funci�n
    }
    return FALSE; // Dejar de llamar a esta funci�n
}

void start_typing_effect() {
    typing_index = 0;
    g_timeout_add(100, typewriter_effect, NULL); // 100 ms intervalo
}

void update_location_info(const char *location, const char *time) {
    char info[256];
    snprintf(info, sizeof(info), "<span font_desc='24' color='white'><b>%s\n%s</b></span>", location, time);
    gtk_label_set_markup(GTK_LABEL(location_label), info);
}

void update_background_image(const char *image_path) {
    gtk_image_set_from_file(GTK_IMAGE(background_image), image_path);
}

void on_submit_button_clicked(GtkWidget *widget, gpointer data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry));
    detective.name = name;
    detective.saveToFile("ranking.txt");
    g_print("Detective: %s\n", detective.name.c_str());

    // Actualizar el mensaje y deshabilitar la entrada de texto
    const char *mission_text = "Tu mision es atrapar al delincuente dentro de 3 dias, comienza a visitar lugares para obtener pistas";
    gtk_label_set_text(GTK_LABEL(typing_label), mission_text);
    gtk_widget_set_sensitive(entry, FALSE);
    gtk_widget_set_sensitive(widget, FALSE);
}

void on_place_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *map_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(map_window), "Mapa de Colonia");
    gtk_window_set_default_size(GTK_WINDOW(map_window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(map_window), 10);

    g_signal_connect(map_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

    GtkWidget *map_image = gtk_image_new_from_file("imagenes/mapa1.png");
    if (!map_image) {
        g_print("Error: No se pudo cargar la imagen del mapa.\n");
    }
    gtk_container_add(GTK_CONTAINER(map_window), map_image);

    gtk_widget_show_all(map_window);
}

void on_play_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *game_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(game_window), "Carmen San Diego - Juego");
    gtk_window_set_default_size(GTK_WINDOW(game_window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(game_window), 10);

    g_signal_connect(game_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(game_window), grid);

    // Caja para el �rea superior (imagen de fondo y texto)
    GtkWidget *top_area = gtk_fixed_new();
    gtk_widget_set_size_request(top_area, 780, 100);
    gtk_grid_attach(GTK_GRID(grid), top_area, 0, 0, 2, 1);

    // Imagen de fondo
    background_image = gtk_image_new_from_file("imagenes/fondo5.png");
    gtk_fixed_put(GTK_FIXED(top_area), background_image, 0, 0);

    // Etiqueta para el texto sobre la imagen
    location_label = gtk_label_new(NULL);
    gtk_label_set_use_markup(GTK_LABEL(location_label), TRUE);
    gtk_fixed_put(GTK_FIXED(top_area), location_label, 10, 10);

    // Inicializar la informaci�n de la ubicaci�n
    update_location_info("Colonia del Sacramento", "Lunes, 4 p.m.");

    // Caja horizontal para las �reas inferiores izquierda y derecha
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_grid_attach(GTK_GRID(grid), hbox, 0, 1, 2, 1);

    // �rea inferior izquierda para la imagen
    GtkWidget *left_area = gtk_image_new_from_file("imagenes/colsacra.png");
    gtk_widget_set_size_request(left_area, 380, 350);
    gtk_box_pack_start(GTK_BOX(hbox), left_area, TRUE, TRUE, 0);

    // �rea inferior derecha para el texto fijo, efecto de m�quina de escribir y campo de entrada
    GtkWidget *right_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(right_area, 380, 350);
    gtk_box_pack_start(GTK_BOX(hbox), right_area, TRUE, TRUE, 0);

    // Contenedor con scroll para el texto fijo sobre Colonia del Sacramento
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 380, 150);
    gtk_box_pack_start(GTK_BOX(right_area), scrolled_window, TRUE, TRUE, 0);

    fixed_text_label = gtk_label_new(NULL);
    gtk_label_set_use_markup(GTK_LABEL(fixed_text_label), TRUE);
    gtk_label_set_markup(GTK_LABEL(fixed_text_label), "<span font_desc='14' color='blue'>Colonia del Sacramento, conocida en el medio local como Colonia, es la capital del departamento de Colonia en el suroeste de Uruguay. Esta ubicada en la ribera norte izquierda del Rio de la Plata, a 177 kilometros de Montevideo y frente a las costas de Buenos Aires, Argentina, de la que dista solo unos 50 kilometros (en linea recta).</span>");
    gtk_label_set_line_wrap(GTK_LABEL(fixed_text_label), TRUE); // Activar el ajuste de l�nea
    gtk_label_set_max_width_chars(GTK_LABEL(fixed_text_label), 50); // Establecer un ancho m�ximo para el ajuste de l�nea
    gtk_container_add(GTK_CONTAINER(scrolled_window), fixed_text_label);

    typing_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(typing_label), TRUE);
    gtk_box_pack_start(GTK_BOX(right_area), typing_label, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(right_area), entry, FALSE, FALSE, 0);

    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(right_area), submit_button, FALSE, FALSE, 0);
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_button_clicked), NULL);

    // Iniciar el efecto de la m�quina de escribir
    start_typing_effect();

    // Caja horizontal para los botones
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 2, 2, 1);

    // Botones en la parte inferior
    GtkWidget *see_button = gtk_button_new_with_label("Visitar");
    GtkWidget *depart_button = gtk_button_new_with_label("Mapa");
    GtkWidget *crime_button = gtk_button_new_with_label("Buscar Criminal");
    GtkWidget *close_button = gtk_button_new_with_label("Salir");

    // Ajustamos el tama�o de los botones
    int button_width = 100;
    int button_height = 50;
    gtk_widget_set_size_request(see_button, button_width, button_height);
    gtk_widget_set_size_request(depart_button, button_width, button_height);
    gtk_widget_set_size_request(crime_button, button_width, button_height);
    gtk_widget_set_size_request(close_button, button_width, button_height);

    // Conectar el bot�n "Mapa" para mostrar el mapa interactivo
    g_signal_connect(depart_button, "clicked", G_CALLBACK(on_map_button_clicked), NULL);

    // Conectar el bot�n "Cerrar" para cerrar la ventana de juego
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), game_window);

    // Agregar botones a la caja de botones
    gtk_box_pack_start(GTK_BOX(button_box), see_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), depart_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), crime_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), close_button, TRUE, TRUE, 0);

    gtk_widget_show_all(game_window);
}

void on_map_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *map_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(map_window), "Mapa de Colonia");
    gtk_window_set_default_size(GTK_WINDOW(map_window), 800, 600);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(map_window), drawing_area);

    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(map_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

    // Conectar la funci�n de clic del bot�n
    g_signal_connect(G_OBJECT(drawing_area), "button-press-event", G_CALLBACK(on_button_press_event), NULL);
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);

    gtk_widget_show_all(map_window);
}

// Funci�n para dibujar la l�nea con opacidad
void draw_line_with_opacity(cairo_t *cr, gdouble start_x, gdouble start_y, gdouble end_x, gdouble end_y, gdouble opacity) {
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, opacity); // Rojo con opacidad
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, end_x, end_y);
    cairo_stroke(cr);
}

// Funci�n para actualizar la opacidad de la l�nea de forma gradual
static gboolean update_opacity(gpointer user_data) {
    GtkWidget *drawing_area = GTK_WIDGET(user_data);

    // Reducir gradualmente la opacidad de la l�nea
    line_opacity -= fade_speed;
    if (line_opacity <= 0.0) {
        line_opacity = 0.0;
        // Detener la animaci�n
        animate_line = FALSE;
        if (animation_timeout_id) {
            g_source_remove(animation_timeout_id);
        }

        // Abrir la nueva ventana despu�s de que la l�nea llegue al bot�n
        open_city_window(last_button_index, city_names[last_button_index]);
    }

    // Forzar la redibujaci�n
    gtk_widget_queue_draw(drawing_area);

    return TRUE; // Continuar la animaci�n hasta que la opacidad llegue a 0
}

// Funci�n de dibujo
gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    // Cargar la imagen de fondo
    cairo_surface_t *image = cairo_image_surface_create_from_png("imagenes/mapa1.png");
    if (!image) {
        g_print("Error: No se pudo cargar la imagen de fondo.\n");
    }

    int img_width = cairo_image_surface_get_width(image);
    int img_height = cairo_image_surface_get_height(image);

    // Escalar la imagen para ajustarla al tama�o del �rea de dibujo
    double scale_x = (double)allocation.width / img_width;
    double scale_y = (double)allocation.height / img_height;

    cairo_save(cr);
    cairo_scale(cr, scale_x, scale_y);
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);

    cairo_surface_destroy(image);

    // Dibujar los botones redondos
    for (int i = 0; i < num_buttons; ++i) {
        // Cambiar el color a rojo si el bot�n ha sido recorrido
        if (std::find(visited_buttons.begin(), visited_buttons.end(), i) != visited_buttons.end()) {
            cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // Rojo
        } else {
            cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); // Amarillo
        }
        cairo_arc(cr, buttons[i].x + 12.5, buttons[i].y + 12.5, 12.5, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    // Dibujar la l�nea si la animaci�n est� en progreso o ha terminado
    if (line_drawn || animate_line) {
        draw_line_with_opacity(cr, buttons[last_button_index].x + 12.5, buttons[last_button_index].y + 12.5, line_x, line_y, line_opacity);
    }

    return FALSE;
}

// Funci�n de actualizaci�n de la animaci�n
static gboolean update_line_position(gpointer user_data) {
    GtkWidget *drawing_area = GTK_WIDGET(user_data);

    // Calcular la nueva posici�n de la l�nea
    gdouble dx = target_x - line_x;
    gdouble dy = target_y - line_y;
    gdouble distance = sqrt(dx * dx + dy * dy);

    if (distance < 5.0) {
        animate_line = FALSE;
        line_drawn = TRUE;
        line_x = target_x;
        line_y = target_y;

        // Agregar el bot�n alcanzado a la lista de botones recorridos
        visited_buttons.push_back(last_button_index);

        // Iniciar el efecto de desvanecimiento
        if (animation_timeout_id) {
            g_source_remove(animation_timeout_id);
        }
        animation_timeout_id = g_timeout_add(30, (GSourceFunc)update_opacity, drawing_area);

        gtk_widget_queue_draw(drawing_area);
        return FALSE; // Detener la animaci�n
    }

    gdouble step = 5.0;
    line_x += step * (dx / distance);
    line_y += step * (dy / distance);

    // Forzar la redibujaci�n
    gtk_widget_queue_draw(drawing_area);

    return TRUE; // Continuar la animaci�n
}

// Funci�n de clic del bot�n
gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GtkWidget *drawing_area = GTK_WIDGET(widget);

    // Verificar en cu�l bot�n se hizo clic
    for (int i = 0; i < num_buttons; ++i) {
        gdouble dx = event->x - (buttons[i].x + 12.5);
        gdouble dy = event->y - (buttons[i].y + 12.5);
        if (dx * dx + dy * dy <= 12.5 * 12.5) {
            line_x = buttons[last_button_index].x + 12.5;
            line_y = buttons[last_button_index].y + 12.5;
            target_x = buttons[i].x + 12.5;
            target_y = buttons[i].y + 12.5;
            last_button_index = i;
            animate_line = TRUE;
            line_drawn = FALSE;
            line_opacity = 1.0;

            // Iniciar la animaci�n
            if (animation_timeout_id) {
                g_source_remove(animation_timeout_id);
            }
            animation_timeout_id = g_timeout_add(30, update_line_position, drawing_area);

            break;
        }
    }

    return TRUE;
}
