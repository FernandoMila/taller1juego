#include <gtk/gtk.h>
#include <gst/gst.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// Estructura para representar un jugador
struct Player {
    std::string name;
    int score;
};

// Función para cargar el ranking desde un archivo
std::vector<Player> loadRanking(const std::string& filename) {
    std::vector<Player> ranking;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo " << filename << std::endl;
        return ranking;
    }
    Player player;
    while (file >> player.name >> player.score) {
        ranking.push_back(player);
    }
    file.close();
    return ranking;
}

// Función para guardar el ranking en un archivo
void saveRanking(const std::vector<Player>& ranking, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo " << filename << std::endl;
        return;
    }
    for (const auto& player : ranking) {
        file << player.name << " " << player.score << std::endl;
    }
    file.close();
}

// Callback para el botón "Ranking"
void on_ranking_button_clicked(GtkWidget *widget, gpointer data) {
    std::vector<Player> ranking = loadRanking("ranking.txt");
    std::sort(ranking.begin(), ranking.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
    });

    // Limitamos la cantidad de jugadores a mostrar a los primeros 10
    int numPlayersToShow = std::min(10, static_cast<int>(ranking.size()));

    // Creamos un diálogo para mostrar el ranking
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ranking", NULL, GTK_DIALOG_MODAL, "Cerrar", GTK_RESPONSE_NONE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 600);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Creamos una caja vertical para organizar el contenido del diálogo
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    // Agregamos el título "Ranking" en color azul y con un tamaño de letra grande
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='24' color='blue'><b>Ranking</b></span>");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);

    // Construimos el contenido del diálogo con los primeros 10 puntajes
    for (int i = 0; i < numPlayersToShow; ++i) {
        std::string rankText = std::to_string(i + 1) + ". " + ranking[i].name + " - " + std::to_string(ranking[i].score);
        GtkWidget *rank_label = gtk_label_new(NULL);
        gtk_label_set_text(GTK_LABEL(rank_label), rankText.c_str());
        // Ajustamos el tamaño de la fuente
        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_size(font_desc, 18 * PANGO_SCALE);
        gtk_widget_override_font(rank_label, font_desc);
        gtk_widget_set_halign(rank_label, GTK_ALIGN_START);
        GdkRGBA rgba_color;
        gdk_rgba_parse(&rgba_color, "brown"); // Marrón
        gtk_widget_override_color(rank_label, GTK_STATE_FLAG_NORMAL, &rgba_color);
        gtk_box_pack_start(GTK_BOX(vbox), rank_label, FALSE, FALSE, 0);
    }

    gtk_widget_show_all(dialog);

    // Mostramos el diálogo
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Callback para cuando se agrega un pad dinámico en el decodificador
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;
    GstElement *conv = (GstElement *)data;

    sinkpad = gst_element_get_static_pad(conv, "sink");

    gst_pad_link(pad, sinkpad);

    gst_object_unref(sinkpad);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    // Creamos la ventana principal
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Carmen San Diego en Colonia");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(win), 10);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Creamos la caja vertical para organizar los elementos
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    // Etiqueta con el nombre del juego
    GtkWidget *label = gtk_label_new("Carmen San Diego en Colonia");
    gtk_widget_set_name(label, "game-title");
    GdkRGBA rgba_color;
    gdk_rgba_parse(&rgba_color, "blue");
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &rgba_color);
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_label_set_markup(GTK_LABEL(label), "<span font_desc='36'><b>Carmen San Diego en Colonia</b></span>");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    // Creamos una caja para los botones
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_CENTER);
    gtk_box_set_spacing(GTK_BOX(button_box), 10);
    gtk_box_pack_end(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    // Creamos los botones
    GtkWidget *play_button = gtk_button_new_with_label("Jugar");
    GtkWidget *help_button = gtk_button_new_with_label("Ayuda");
    GtkWidget *ranking_button = gtk_button_new_with_label("Ranking");

    // Ajustamos el tamaño de los botones
    int button_width = 100;
    int button_height = 50;
    gtk_widget_set_size_request(play_button, button_width, button_height);
    gtk_widget_set_size_request(help_button, button_width, button_height);
    gtk_widget_set_size_request(ranking_button, button_width, button_height);

    g_signal_connect(G_OBJECT(ranking_button), "clicked", G_CALLBACK(on_ranking_button_clicked), NULL);

    // Agregamos los botones a la caja de botones
    gtk_container_add(GTK_CONTAINER(button_box), play_button);
    gtk_container_add(GTK_CONTAINER(button_box), help_button);
    gtk_container_add(GTK_CONTAINER(button_box), ranking_button);

    // Mostramos todo
    gtk_widget_show_all(win);

    // Reproducimos el sonido al inicio
    GstElement *pipeline = gst_pipeline_new("audio-player");
    GstElement *source = gst_element_factory_make("filesrc", "file-source");
    GstElement *decoder = gst_element_factory_make("decodebin", "decoder");
    GstElement *conv = gst_element_factory_make("audioconvert", "converter");
    GstElement *sink = gst_element_factory_make("autoaudiosink", "audio-output");

    if (!pipeline || !source || !decoder || !conv || !sink) {
        g_printerr("Faltan elementos en el pipeline de GStreamer.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, decoder, conv, sink, NULL);
    gst_element_link(source, decoder);
    gst_element_link(conv, sink);

    g_signal_connect(decoder, "pad-added", G_CALLBACK(on_pad_added), conv);

    g_object_set(G_OBJECT(source), "location", "sonido/backgroundMusic.wav", NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    gtk_main();

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));

    return 0;
}
