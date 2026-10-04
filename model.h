#pragma once

#include <array>
#include <filesystem>
#include <ostream>
#include <vector>

namespace fs = std::filesystem;

struct Vertex {
    double x, y, z;

    Vertex() = default;
    Vertex(double x, double y, double z) : x(x), y(y), z(z) {}
    Vertex(const std::array<double, 3>& arr) : x(arr[0]), y(arr[1]), z(arr[2]) {}
};

using Face = std::array<int, 3>;

class Model {
   private:
    std::vector<Vertex> m_vertices;
    std::vector<Face> m_faces;

   public:
    explicit Model(fs::path path);

    const auto& vertices() const { return m_vertices; }

    const auto& vertice(int fi, int vi) const { return m_vertices[m_faces[fi][vi]]; }

    const auto& faces() const { return m_faces; }

    friend std::ostream& operator<<(std::ostream& os, const Model& m) {
        return os << m.m_faces.size() << " " << m.m_vertices.size() << std::endl;
    }
};
