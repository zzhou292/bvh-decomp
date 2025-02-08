#include "bvh_animator.hpp"
#include <iostream>

BVHAnimator::BVHAnimator(const AABBDataHandler *handler)
    : data_handler(handler), current_frame(0), animation_running(true)
{
    init_gl_window();
}

BVHAnimator::~BVHAnimator()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void BVHAnimator::init_gl_window()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    window = glfwCreateWindow(1280, 720, "BVH Animator", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
}

void BVHAnimator::animate()
{
    while (!glfwWindowShouldClose(window) && animation_running)
    {
        process_input();
        update();
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void BVHAnimator::process_input()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        animation_running = false;
}

void BVHAnimator::update()
{
    if (current_frame >= data_handler->num_frames())
        current_frame = 0;

    auto aabbs = data_handler->get_frame_aabbs(current_frame);
    BVHBuilder builder(aabbs);

    if (!bvh_root)
    {
        bvh_root = builder.build_top_down();
    }
    else
    {
        bvh_root = builder.update_incremental(std::move(bvh_root));
    }

    current_frame++;
}

void BVHAnimator::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up orthogonal projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5, 12, -12, 3, -10, 10); // Adjusted to match Python's view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Move camera up and to the left, point slightly upward
    gluLookAt(
        3, 4, 3, // Eye position (moved up in Y)
        0, 1, 0, // Look at point (moved up to point camera upward)
        0, 1, 0  // Up vector (unchanged)
    );

    // Draw all AABBs
    auto aabbs = data_handler->get_frame_aabbs(current_frame);
    for (const auto &aabb : aabbs)
    {
        draw_aabb(aabb, new float[3]{0.2f, 0.8f, 0.2f});
    }

    // Draw BVH structure
    if (bvh_root)
    {
        draw_tree(bvh_root.get());
    }
}

void BVHAnimator::draw_aabb(const AABB &aabb, const float color[3])
{
    glColor3fv(color);

    // Calculate all 8 vertices of the AABB
    const float &xmin = aabb.min[0];
    const float &ymin = aabb.min[1];
    const float &zmin = aabb.min[2];
    const float &xmax = aabb.max[0];
    const float &ymax = aabb.max[1];
    const float &zmax = aabb.max[2];

    // Define all 12 edges of the cube
    glBegin(GL_LINES);

    // Bottom face
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymin, zmin);

    // Top face
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymin, zmax);

    // Vertical edges
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmax);

    glEnd();
}

void BVHAnimator::draw_tree(const Node *node)
{
    if (!node)
        return;

    float color[] = {0.8f, 0.2f, 0.2f};
    draw_aabb(node->aabb, color);

    draw_tree(node->left.get());
    draw_tree(node->right.get());
}