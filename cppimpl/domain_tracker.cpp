#include "domain_tracker.hpp"
#include <algorithm>
#include <Eigen/Dense>
#include <limits>

using namespace Eigen;

void DomainTracker::update(const std::vector<std::vector<int>> &new_groups)
{
    cost_matrix.clear();
    cost_matrix.resize(previous_groups.size(),
                       std::vector<double>(new_groups.size(), 0));

    for (size_t i = 0; i < previous_groups.size(); ++i)
    {
        for (size_t j = 0; j < new_groups.size(); ++j)
        {
            // Use symmetric_difference directly like Python
            auto diff = symmetric_difference(previous_groups[i], new_groups[j]);
            cost_matrix[i][j] = diff.size();
        }
    }
    hungarian_solve();
}
std::vector<std::vector<int>> DomainTracker::match_domains(
    const std::vector<std::vector<int>> &new_groups)
{
    if (previous_groups.empty())
    {
        previous_groups = new_groups;
        return new_groups;
    }

    // Update cost matrix and solve assignment problem
    update(new_groups);

    // Create mapping from old groups to new groups
    std::vector<std::vector<int>> ordered(new_groups.size());

    // Apply Hungarian algorithm results
    for (size_t i = 0; i < assignment.size(); ++i)
    {
        if (assignment[i] != -1 && assignment[i] < (int)new_groups.size())
        {
            ordered[i] = new_groups[assignment[i]];
        }
    }

    // Handle new groups that appeared
    for (size_t j = 0; j < new_groups.size(); ++j)
    {
        if (std::find(assignment.begin(), assignment.end(), j) == assignment.end())
        {
            ordered.push_back(new_groups[j]);
        }
    }

    previous_groups = ordered;
    return ordered;
}

MatrixXf DomainTracker::create_cost_matrix(
    const std::vector<std::vector<int>> &old_domains,
    const std::vector<std::vector<int>> &new_domains)
{
    MatrixXf matrix(old_domains.size(), new_domains.size());

    for (int i = 0; i < matrix.rows(); ++i)
    {
        for (int j = 0; j < matrix.cols(); ++j)
        {
            auto diff = symmetric_difference(old_domains[i], new_domains[j]);
            matrix(i, j) = static_cast<float>(diff.size());
        }
    }
    return matrix;
}

std::vector<int> DomainTracker::symmetric_difference(
    const std::vector<int> &a,
    const std::vector<int> &b)
{
    std::vector<int> result;
    std::vector<int> sorted_a(a.begin(), a.end());
    std::vector<int> sorted_b(b.begin(), b.end());
    std::sort(sorted_a.begin(), sorted_a.end());
    std::sort(sorted_b.begin(), sorted_b.end());

    std::set_symmetric_difference(
        sorted_a.begin(), sorted_a.end(),
        sorted_b.begin(), sorted_b.end(),
        std::back_inserter(result));
    return result;
}

void DomainTracker::hungarian_solve()
{
    int n = previous_groups.size();
    int m = cost_matrix[0].size();

    std::vector<double> u(n + 1, 0);
    std::vector<double> v(m + 1, 0);
    std::vector<int> p(m + 1, 0);
    std::vector<int> way(m + 1, 0);

    for (int i = 1; i <= n; ++i)
    {
        p[0] = i;
        int j0 = 0;
        std::vector<double> minv(m + 1, HUNGARIAN_INFINITY);
        std::vector<bool> used(m + 1, false);

        do
        {
            used[j0] = true;
            int i0 = p[j0];
            double delta = HUNGARIAN_INFINITY;
            int j1 = 0;

            for (int j = 1; j <= m; ++j)
            {
                if (!used[j])
                {
                    double cur = cost_matrix[i0 - 1][j - 1] - u[i0] - v[j];
                    if (cur < minv[j])
                    {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta)
                    {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }

            for (int j = 0; j <= m; ++j)
            {
                if (used[j])
                {
                    u[p[j]] += delta;
                    v[j] -= delta;
                }
                else
                {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do
        {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    assignment.resize(n);
    for (int j = 1; j <= m; ++j)
    {
        if (p[j] != 0)
        {
            assignment[p[j] - 1] = j - 1;
        }
    }
}