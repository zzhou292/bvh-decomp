#pragma once
#include <vector>
#include <Eigen/Dense>

class DomainTracker
{
    std::vector<std::vector<int>> prev_domains;
    std::vector<std::vector<int>> previous_groups;
    std::vector<int> assignment;
    std::vector<std::vector<double>> cost_matrix;

public:
    std::vector<std::vector<int>> match_domains(
        const std::vector<std::vector<int>> &new_domains);

private:
    static constexpr double HUNGARIAN_INFINITY = 1e10;
    Eigen::MatrixXf create_cost_matrix(
        const std::vector<std::vector<int>> &old_domains,
        const std::vector<std::vector<int>> &new_domains);

    std::vector<int> symmetric_difference(
        const std::vector<int> &a,
        const std::vector<int> &b);

    void hungarian_method(
        const std::vector<std::vector<float>> &cost_matrix,
        std::vector<int> &assignment);

    void update(const std::vector<std::vector<int>> &new_groups);
    void hungarian_solve();
};