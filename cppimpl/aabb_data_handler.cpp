#include "aabb_data_handler.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

AABBDataHandler::AABBDataHandler(const std::string &filename)
{
    load_data_from_csv(filename);
}

void AABBDataHandler::load_data_from_csv(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    std::getline(file, line); // Skip header

    // Determine number of objects based on filename
    int num_objects = (filename.find("sce_40") != std::string::npos) ? 40 : 70;
    std::cout << "Detected " << num_objects << " objects from filename" << std::endl;

    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string token;
        std::vector<float> values;

        // Read all values from the line
        while (std::getline(ss, token, ','))
        {
            values.push_back(std::stof(token));
        }

        float time = values[0];
        std::vector<AABB> frame_aabbs;

        // Process each object's AABB in the row
        for (int i = 0; i < num_objects; ++i)
        {
            int idx = 1 + i * 6; // Skip time value and offset for each object
            AABB aabb;
            aabb.min = {values[idx], values[idx + 1], values[idx + 2]};
            aabb.max = {values[idx + 3], values[idx + 4], values[idx + 5]};
            frame_aabbs.push_back(aabb);
        }

        all_frame_aabbs.push_back(frame_aabbs);
        frame_times.push_back(time);
    }

    std::cout << "\nFile loading summary:" << std::endl;
    std::cout << "Total frames: " << all_frame_aabbs.size() << std::endl;
    for (size_t i = 0; i < all_frame_aabbs.size(); ++i)
    {
        std::cout << "Frame " << i << " has " << all_frame_aabbs[i].size()
                  << " AABBs at time " << frame_times[i] << std::endl;
    }
}

std::vector<AABB> AABBDataHandler::get_frame_aabbs(int frame) const
{
    if (frame < 0 || frame >= static_cast<int>(all_frame_aabbs.size()))
    {
        throw std::out_of_range("Frame index out of range");
    }
    return all_frame_aabbs[frame];
}

float AABBDataHandler::get_frame_time(int frame) const
{
    if (frame < 0 || frame >= static_cast<int>(frame_times.size()))
    {
        throw std::out_of_range("Frame index out of range");
    }
    return frame_times[frame];
}

int AABBDataHandler::num_frames() const
{
    return all_frame_aabbs.size();
}