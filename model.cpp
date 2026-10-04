#include "model.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

Model::Model(fs::path path) {
    std::ifstream in(path);

    if (!in) return;

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            std::array<double, 3> p;
            ss >> p[0] >> p[1] >> p[2];
            this->m_vertices.push_back(p);
        } else if (tag == "f") {
            Face f;
            for (int& idx : f) {
                std::string tok;
                ss >> tok;
                idx = std::stoi(tok) - 1;  // OBJ is 1-based; stoi stops at the first '/'
            }
            this->m_faces.push_back(f);
        }
    }
}
