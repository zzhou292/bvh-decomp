#pragma once
#include "bvh_builder.hpp"
#include <Eigen/Dense>
#include <vector>

class DomainTracker {
  std::vector<std::vector<int>> previous_groups;
  std::vector<int> assignment;
  std::vector<std::vector<double>> cost_matrix;
  std::vector<const Node *> previous_nodes;

public:
  std::vector<std::vector<int>>
  match_domains(const std::vector<std::vector<int>> &new_groups,
                const std::vector<const Node *> &new_nodes);

private:
  static constexpr double HUNGARIAN_INFINITY = 1e10;

  void update(const std::vector<std::vector<int>> &new_groups,
              const std::vector<const Node *> &new_nodes);
  void hungarian_solve();

  AABB compute_domain_aabb(const std::vector<int> &domain_indices,
                           const std::vector<AABB> &aabbs) const;
};