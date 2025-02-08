#pragma once
#include "bvh_builder.hpp"
#include "domain_tracker.hpp"
#include "aabb_data_handler.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <memory>

class SubdomainVisualizer
{
public:
    SubdomainVisualizer();
    ~SubdomainVisualizer();

    void visualize(const std::vector<AABB> &aabbs,
                   const std::vector<std::vector<int>> &groups);

    GLFWwindow *get_window() const { return window; }

private:
    GLFWwindow *window;
    DomainTracker domain_tracker;

    void init_gl_window();
    void draw_aabb(const AABB &aabb, const float color[3]);
};