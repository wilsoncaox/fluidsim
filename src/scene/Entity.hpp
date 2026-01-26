#pragma once

#include "../resource/Vertex.hpp"

#include <vector>

struct Entity {
    virtual void load_mesh() = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
