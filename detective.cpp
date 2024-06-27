#include "detective.h"
#include <fstream>

Detective::Detective() : name(""), score(0) {}

void Detective::saveToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << name << " " << score << std::endl;
        file.close();
    }
}
