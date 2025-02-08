#include "bvh_tree_visualizer.hpp"
#include <iostream>
#include <cmath>

BVHTreeVisualizer::BVHTreeVisualizer()
{
    init_gl_window();
}

BVHTreeVisualizer::~BVHTreeVisualizer()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void BVHTreeVisualizer::init_gl_window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    window = glfwCreateWindow(1200, 600, "BVH Tree Structure", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
}

void BVHTreeVisualizer::draw_tree_structure(const Node *root)
{
    if (!root)
        return;

    // Reset positions
    positions.clear();
    current_x = 0;

    // Assign positions
    assign_positions(root);

    // Clear window
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, current_x, -10, 0, -1, 1);

    // Draw connections and nodes
    draw_connections(root);
    draw_nodes(root);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

float BVHTreeVisualizer::assign_positions(const Node *node, int depth)
{
    if (!node)
        return current_x;

    if (node->is_leaf)
    {
        positions[node] = {current_x, -depth};
        current_x += 1;
        return current_x - 1;
    }

    float left_x = assign_positions(node->left.get(), depth + 1);
    float right_x = assign_positions(node->right.get(), depth + 1);
    float node_x = (left_x + right_x) / 2;

    positions[node] = {node_x, -depth};
    return node_x;
}

void BVHTreeVisualizer::draw_connections(const Node *node)
{
    if (!node || node->is_leaf)
        return;

    auto [x_parent, y_parent] = positions[node];
    auto [x_left, y_left] = positions[node->left.get()];
    auto [x_right, y_right] = positions[node->right.get()];

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(x_parent, y_parent);
    glVertex2f(x_left, y_left);
    glVertex2f(x_parent, y_parent);
    glVertex2f(x_right, y_right);
    glEnd();

    draw_connections(node->left.get());
    draw_connections(node->right.get());
}

void BVHTreeVisualizer::draw_nodes(const Node *node)
{
    if (!node)
        return;

    auto [x, y] = positions[node];

    if (node->is_leaf)
    {
        glColor3f(0.0f, 1.0f, 0.0f); // Green for leaf nodes
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }
    else
    {
        glColor3f(1.0f, 0.0f, 0.0f); // Red for internal nodes
        glPointSize(15.0f);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }

    draw_nodes(node->left.get());
    draw_nodes(node->right.get());
}
