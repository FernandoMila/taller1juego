#ifndef RANKING_H_INCLUDED
#define RANKING_H_INCLUDED

#include <vector>
#include <string>
#include <gtk/gtk.h>

struct Player {
    std::string name;
    int score;
};

std::vector<Player> loadRanking(const std::string& filename);
void saveRanking(const std::vector<Player>& ranking, const std::string& filename);
void on_ranking_button_clicked(GtkWidget *widget, gpointer data);




#endif // RANKING_H_INCLUDED
