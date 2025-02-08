#pragma once
#include <vector>
#include <string>
#include <array>
#include "bvh_builder.hpp"

class AABBDataHandler
{
public:
    explicit AABBDataHandler(const std::string &filename);

    std::vector<AABB> get_frame_aabbs(int frame) const;
    float get_frame_time(int frame) const;
    int num_frames() const;

private:
    std::vector<std::vector<AABB>> all_frame_aabbs;
    std::vector<float> frame_times;

    void load_data_from_csv(const std::string &filename);
};