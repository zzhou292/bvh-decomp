#include "aabb_data_handler.hpp"
#include "bvh_animator.hpp"
#include "subdomain_visualizer.hpp"
#include "domain_tracker.hpp"
#include "bvh_builder.hpp"
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main(int argc, char *argv[])
{
    try
    {
        // Default input file if not provided
        std::string input_file = argc > 1 ? argv[1] : "sce_40.csv";

        // Create data handler
        AABBDataHandler data_handler(input_file);

        // Only create subdomain visualizer
        SubdomainVisualizer subdomain_vis;
        DomainTracker domain_tracker;
        std::unique_ptr<Node> bvh_root = nullptr;

        // Main animation/processing loop
        for (int frame = 0; frame < data_handler.num_frames(); ++frame)
        {
            // Get current frame's AABBs
            auto aabbs = data_handler.get_frame_aabbs(frame);
            std::cout << "Frame " << frame << ": Loaded " << aabbs.size() << " AABBs" << std::endl;
            float current_time = data_handler.get_frame_time(frame);

            // Print current simulation time
            std::cout << "Current simulation time: "
                      << current_time << " seconds" << std::endl;

            // Build or update BVH
            BVHBuilder builder(aabbs);
            if (!bvh_root)
            {
                bvh_root = builder.build_top_down();
                std::cout << "Built initial BVH tree" << std::endl;
            }
            else
            {
                bvh_root = builder.update_incremental(std::move(bvh_root));
            }

            // Get subdomains
            int num_subdomains = 5;
            auto groups = builder.get_subdomains_greedy(bvh_root.get(), num_subdomains);

            // Match domains
            auto ordered_groups = domain_tracker.match_domains(groups);

            // Visualize (these will open separate windows)
            subdomain_vis.visualize(aabbs, ordered_groups);

            // Optional: add a small delay or wait for user input
            glfwPollEvents();

            std::cout << "Created " << groups.size() << " subdomain groups:" << std::endl;
            for (size_t i = 0; i < groups.size(); ++i)
            {
                std::cout << "Group " << i << " has " << groups[i].size() << " objects" << std::endl;
            }
        }

        // Keep windows open until closed
        while (!glfwWindowShouldClose(subdomain_vis.get_window()))
        {
            glfwPollEvents();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}