#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "bvh_builder.hpp"
#include "domain_tracker.hpp"
#include "aabb_data_handler.hpp"
#include <memory>

class BVHAnimator
{
    std::unique_ptr<Node> bvh_root;
    DomainTracker domain_tracker;
    GLFWwindow *window;
    const AABBDataHandler *data_handler;
    int current_frame;
    bool animation_running;

public:
    explicit BVHAnimator(const AABBDataHandler *handler);
    ~BVHAnimator();
    void animate();

private:
    void init_gl_window();
    void process_input();
    void update();
    void render();
    void draw_aabb(const AABB &aabb, const float color[3]);
    void draw_tree(const Node *node);
};