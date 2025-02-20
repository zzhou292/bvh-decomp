#pragma once
#include <algorithm>
#include <array>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

struct AABB {
  std::array<float, 3> min;
  std::array<float, 3> max;

  bool operator!=(const AABB &other) const {
    return min != other.min || max != other.max;
  }
};

class Node {
public:
  AABB aabb;
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;
  bool is_leaf;
  int obj_index;
  int leaf_count;

  Node(AABB box, bool leaf = false, int idx = -1)
      : aabb(box), is_leaf(leaf), obj_index(idx), leaf_count(1) {}
};

class BVHBuilder {
public:
  explicit BVHBuilder(const std::vector<AABB> &aabbs) : aabbs(aabbs) {}

  std::unique_ptr<Node> build_top_down();
  std::unique_ptr<Node> update_incremental(std::unique_ptr<Node> root,
                                           float threshold = 0.3f);
  std::vector<std::vector<int>> get_subdomains_greedy(const Node *root,
                                                      int num_groups);
  std::vector<const Node *>
  get_group_nodes(const Node *root,
                  const std::vector<std::vector<int>> &groups);

  static float aabb_volume(const AABB &aabb);
  static AABB merge_aabbs(const AABB &a, const AABB &b);
  static float calc_overlap(const AABB &a, const AABB &b);

private:
  const std::vector<AABB> &aabbs;

  struct BuildNode {
    AABB aabb;
    int start;
    int end;
  };

  struct HeapNode {
    int negative_leaf_count;
    int counter;
    const Node *node;

    bool operator>(const HeapNode &other) const {
      if (negative_leaf_count != other.negative_leaf_count)
        return negative_leaf_count > other.negative_leaf_count;
      return counter > other.counter;
    }
  };

  std::unique_ptr<Node> recursive_build(BuildNode bnode);
  void refit(Node *node, std::vector<bool> &updated);
  void print_tree_structure(const Node *node, std::string prefix = "",
                            bool isLeft = false) const;
};