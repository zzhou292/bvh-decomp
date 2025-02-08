#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "bvh_builder.hpp"
#include <unordered_map>

class BVHTreeVisualizer
{
public:
    BVHTreeVisualizer();
    ~BVHTreeVisualizer();

    void draw_tree_structure(const Node *root);
    GLFWwindow *get_window() const { return window; }

private:
    GLFWwindow *window;
    std::unordered_map<const Node *, std::pair<float, float>> positions;
    float current_x = 0;

    void init_gl_window();
    float assign_positions(const Node *node, int depth = 0);
    void draw_connections(const Node *node);
    void draw_nodes(const Node *node);
};