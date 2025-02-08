#include "subdomain_visualizer.hpp"
#include <iostream>

SubdomainVisualizer::SubdomainVisualizer()
{
    init_gl_window();
}

SubdomainVisualizer::~SubdomainVisualizer()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void SubdomainVisualizer::init_gl_window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Create window with OpenGL context
    window = glfwCreateWindow(1200, 800, "Subdomain Visualization", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SubdomainVisualizer::draw_aabb(const AABB &aabb, const float color[3])
{
    // Draw filled box with transparency
    glColor4f(color[0], color[1], color[2], 0.6f); // Match Python's alpha=0.6

    // Front face
    glBegin(GL_QUADS);
    glVertex3f(aabb.min[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.max[1], aabb.min[2]);
    glVertex3f(aabb.min[0], aabb.max[1], aabb.min[2]);

    // Back face
    glVertex3f(aabb.min[0], aabb.min[1], aabb.max[2]);
    glVertex3f(aabb.max[0], aabb.min[1], aabb.max[2]);
    glVertex3f(aabb.max[0], aabb.max[1], aabb.max[2]);
    glVertex3f(aabb.min[0], aabb.max[1], aabb.max[2]);

    // Top face
    glVertex3f(aabb.min[0], aabb.max[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.max[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.max[1], aabb.max[2]);
    glVertex3f(aabb.min[0], aabb.max[1], aabb.max[2]);

    // Bottom face
    glVertex3f(aabb.min[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.min[1], aabb.max[2]);
    glVertex3f(aabb.min[0], aabb.min[1], aabb.max[2]);
    glEnd();

    // Draw wireframe
    glColor4f(color[0] * 0.8f, color[1] * 0.8f, color[2] * 0.8f, 1.0f);
    glBegin(GL_LINE_LOOP);
    // Front face
    glVertex3f(aabb.min[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.min[1], aabb.min[2]);
    glVertex3f(aabb.max[0], aabb.max[1], aabb.min[2]);
    glVertex3f(aabb.min[0], aabb.max[1], aabb.min[2]);
    glEnd();
}

void SubdomainVisualizer::visualize(
    const std::vector<AABB> &aabbs,
    const std::vector<std::vector<int>> &groups)
{
    std::cout << "\nVisualization started with:" << std::endl;
    std::cout << "Total AABBs: " << aabbs.size() << std::endl;
    std::cout << "Number of groups: " << groups.size() << std::endl;

    // Print first AABB's coordinates to verify data
    if (!aabbs.empty())
    {
        std::cout << "First AABB coordinates:" << std::endl;
        std::cout << "Min: (" << aabbs[0].min[0] << ", " << aabbs[0].min[1] << ", " << aabbs[0].min[2] << ")" << std::endl;
        std::cout << "Max: (" << aabbs[0].max[0] << ", " << aabbs[0].max[1] << ", " << aabbs[0].max[2] << ")" << std::endl;
    }

    // Set white background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Setup view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5, 12, -12, 3, -10, 10); // Match Python's axis limits

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Match Python's view_init(elev=15, azim=-45)
    glRotatef(10.0f, 1.0f, 0.0f, 0.0f);   // reduced elevation to 10 degrees
    glRotatef(-135.0f, 0.0f, 1.0f, 0.0f); // changed azimuth to -135 for better orientation

    // Predefined color palette
    const float colors[][3] = {
        {0.12f, 0.47f, 0.71f}, {0.17f, 0.63f, 0.17f}, {0.54f, 0.21f, 0.58f}, {0.89f, 0.47f, 0.76f}, {0.80f, 0.73f, 0.13f}, {0.64f, 0.08f, 0.18f}};

    // Draw AABBs by group
    for (size_t group_idx = 0; group_idx < groups.size(); ++group_idx)
    {
        const auto &group = groups[group_idx];
        const float *color = colors[group_idx % 6];

        for (int idx : group)
        {
            draw_aabb(aabbs[idx], color);
        }
    }

    glfwSwapBuffers(window);
}