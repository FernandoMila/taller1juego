#ifndef DETECTIVE_H
#define DETECTIVE_H

#include <string>

class Detective {
public:
    std::string name;
    int score;

    Detective();
    void saveToFile(const std::string& filename);
};

#endif // DETECTIVE_H
