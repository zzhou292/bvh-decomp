from dataclasses import dataclass
from typing import List, Tuple, Deque, Optional
from collections import deque
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.pyplot as plt
import numpy as np
import heapq

@dataclass(frozen=True, eq=True)
class Node:
    aabb: Tuple[Tuple[float, float, float], Tuple[float, float, float]]
    left: Optional['Node'] = None
    right: Optional['Node'] = None
    is_leaf: bool = False
    obj_index: int = -1
    leaf_count: int = 1

class BVHBuilder:
    def __init__(self, aabbs: List[Tuple]):
        self.aabbs = aabbs
        
    @staticmethod
    def aabb_volume(aabb: Tuple) -> float:
        min_point, max_point = aabb
        return (max_point[0] - min_point[0]) * \
               (max_point[1] - min_point[1]) * \
               (max_point[2] - min_point[2])

    @staticmethod
    def merge_aabbs(aabb1: Tuple, aabb2: Tuple) -> Tuple:
        return (
            (min(aabb1[0][0], aabb2[0][0]), 
             min(aabb1[0][1], aabb2[0][1]),
             min(aabb1[0][2], aabb2[0][2])),
            (max(aabb1[1][0], aabb2[1][0]),
             max(aabb1[1][1], aabb2[1][1]),
             max(aabb1[1][2], aabb2[1][2]))
        )

    def build_top_down(self) -> Optional[Node]:
        indexed_aabbs = [(aabb, i) for i, aabb in enumerate(self.aabbs)]
        if not indexed_aabbs:
            return None

        queue = deque([(indexed_aabbs, None, None)])
        root = None

        while queue:
            items, parent, is_left = queue.popleft()

            if len(items) == 1:
                aabb, idx = items[0]
                node = Node(aabb=aabb, is_leaf=True, obj_index=idx, leaf_count=1)
            else:
                combined_aabb = items[0][0]
                
                for aabb, _ in items[1:]:
                    combined_aabb = self.merge_aabbs(combined_aabb, aabb)

                min_pt, max_pt = combined_aabb
                axis_lengths = [max_pt[i] - min_pt[i] for i in range(3)]
                split_axis = axis_lengths.index(max(axis_lengths))
                sorted_items = sorted(items, 
                    key=lambda x: round((x[0][0][split_axis] + x[0][1][split_axis])/2 / 0.1) * 0.1)
                
                split_idx = len(sorted_items) // 2  # Split at midpoint
                split_idx = max(1, min(split_idx, len(sorted_items)-1))
                node = Node(aabb=combined_aabb)
                queue.append((sorted_items[:split_idx], node, True))
                queue.append((sorted_items[split_idx:], node, False))

            if parent:
                if is_left:
                    object.__setattr__(parent, 'left', node)
                else:
                    object.__setattr__(parent, 'right', node)
                new_parent = Node(
                    aabb=parent.aabb,
                    left=parent.left,
                    right=parent.right,
                    leaf_count=(node.leaf_count + (parent.left.leaf_count if parent.left else 0))
                )
                object.__setattr__(parent, 'leaf_count', new_parent.leaf_count)
            else:
                root = node
        return root

    @staticmethod
    def draw_aabb(ax, aabb, color='b', alpha=0.5):
        min_point, max_point = aabb
        vertices = [
            [min_point[0], min_point[1], min_point[2]],
            [min_point[0], min_point[1], max_point[2]],
            [min_point[0], max_point[1], min_point[2]],
            [min_point[0], max_point[1], max_point[2]],
            [max_point[0], min_point[1], min_point[2]],
            [max_point[0], min_point[1], max_point[2]],
            [max_point[0], max_point[1], min_point[2]],
            [max_point[0], max_point[1], max_point[2]]
        ]
        faces = [
            [vertices[0], vertices[1], vertices[3], vertices[2]],  # Left face
            [vertices[4], vertices[5], vertices[7], vertices[6]],  # Right face
            [vertices[0], vertices[1], vertices[5], vertices[4]],  # Bottom face
            [vertices[2], vertices[3], vertices[7], vertices[6]],  # Top face
            [vertices[0], vertices[2], vertices[6], vertices[4]],  # Front face
            [vertices[1], vertices[3], vertices[7], vertices[5]]   # Back face
        ]
        for face in faces:
            poly = Poly3DCollection([face], color=color, alpha=alpha)
            ax.add_collection3d(poly)

    @staticmethod
    def draw_tree_structure(ax, root: Node):
        """Draws the BVH tree hierarchy on the given axis."""
        positions = {}
        current_x = 0
        fig = ax.figure
        fig.set_size_inches(20, 10)  # Increase figure width

        def assign_positions(node, depth=0):
            nonlocal current_x
            if node.is_leaf:
                x = current_x
                positions[node] = (x, depth)
                current_x += 1
                return x
            else:
                left_x = assign_positions(node.left, depth + 1)
                right_x = assign_positions(node.right, depth + 1)
                node_x = (left_x + right_x) / 2
                positions[node] = (node_x, depth)
                return node_x

        assign_positions(root)
        ax.set_axis_off()

        # Draw connections
        for node in positions:
            if not node.is_leaf:
                x_parent, y_parent = positions[node]
                x_left, y_left = positions[node.left]
                x_right, y_right = positions[node.right]
                ax.plot([x_parent, x_left], [-y_parent, -y_left], 'k-', alpha=0.4)
                ax.plot([x_parent, x_right], [-y_parent, -y_right], 'k-', alpha=0.4)

        # Draw nodes
        for node, (x, y) in positions.items():
            if node.is_leaf:
                min_coords = ",".join(f"{c:.1f}" for c in node.aabb[0])
                max_coords = ",".join(f"{c:.1f}" for c in node.aabb[1])
                ax.plot(x, -y, 'o', markersize=12, color='green', alpha=0.7)
                ax.text(x, -y + 0.25, 
                       f'Leaf {node.obj_index}\nMin: {min_coords}\nMax: {max_coords}',
                       ha='center', va='bottom', fontsize=7,
                       bbox=dict(facecolor='white', alpha=0.8, boxstyle='round,pad=0.2'))
            else:
                vol = BVHBuilder.aabb_volume(node.aabb)
                ax.plot(x, -y, 'o', markersize=22, color='red', alpha=0.5)
                ax.text(x, -y, f'Vol: {vol:.1f}',
                       ha='center', va='center', fontsize=9, color='black')
        plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05)  # Manual margins

    @staticmethod
    def visualize_tree_structure(root: Node):
        """Visualizes the BVH tree hierarchy as a node-link diagram."""
        fig, ax = plt.subplots(figsize=(16, 10))
        positions = {}
        current_x = 0

        def assign_positions(node, depth=0):
            nonlocal current_x
            if node.is_leaf:
                x = current_x
                positions[node] = (x, depth)
                current_x += 1
                return x
            else:
                left_x = assign_positions(node.left, depth + 1)
                right_x = assign_positions(node.right, depth + 1)
                node_x = (left_x + right_x) / 2
                positions[node] = (node_x, depth)
                return node_x

        assign_positions(root)

        # Draw connections
        for node in positions:
            if not node.is_leaf:
                x_parent, y_parent = positions[node]
                x_left, y_left = positions[node.left]
                x_right, y_right = positions[node.right]
                ax.plot([x_parent, x_left], [-y_parent, -y_left], 'k-', alpha=0.4)
                ax.plot([x_parent, x_right], [-y_parent, -y_right], 'k-', alpha=0.4)

        # Draw nodes
        for node, (x, y) in positions.items():
            if node.is_leaf:
                min_coords = ",".join(f"{c:.1f}" for c in node.aabb[0])
                max_coords = ",".join(f"{c:.1f}" for c in node.aabb[1])
                ax.plot(x, -y, 'o', markersize=14, color='green', alpha=0.7)
                ax.text(x, -y + 0.25, 
                       f'Leaf {node.obj_index}\nMin: {min_coords}\nMax: {max_coords}',
                       ha='center', va='bottom', fontsize=8,
                       bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
            else:
                vol = BVHBuilder.aabb_volume(node.aabb)
                ax.plot(x, -y, 'o', markersize=22, color='red', alpha=0.5)
                ax.text(x, -y, f'Vol: {vol:.1f}',
                       ha='center', va='center', fontsize=9, color='black')

        ax.set_axis_off()
        ax.set_title("BVH Tree Hierarchy with AABB Details", pad=20)
        plt.tight_layout()
        plt.show()

    def get_subdomains(self, root: Node, num_subdomains: int) -> List[List[int]]:
        """Collect leaf indices grouped by subdomains at depth log2(num_subdomains)"""
        target_depth = int(np.log2(num_subdomains))
        groups = []
        
        def traverse(node: Node, current_depth: int):
            if not node:
                return
            
            if current_depth == target_depth:
                group = []
                stack = [node]
                while stack:
                    n = stack.pop()
                    if n.is_leaf:
                        group.append(n.obj_index)
                    else:
                        stack.extend([n.right, n.left])
                groups.append(group)
            else:
                traverse(node.left, current_depth + 1)
                traverse(node.right, current_depth + 1)
        
        traverse(root, 0)
        return groups

    def get_subdomains_greedy(self, root: Node, num_subdomains: int) -> List[List[int]]:
        """Greedy decomposition using max-heap of leaf counts"""
        # Use unique counter to avoid comparing nodes directly
        counter = 0
        heap = [(-root.leaf_count, counter, root)]
        heapq.heapify(heap)
        counter += 1
        
        # Split largest nodes until we reach target count
        while len(heap) < num_subdomains and heap:
            current_count, _, current_node = heapq.heappop(heap)
            
            if current_node.is_leaf:
                heapq.heappush(heap, (current_count, counter, current_node))
                counter += 1
                break
                
            # Push children with their leaf counts
            for child in [current_node.left, current_node.right]:
                if child:
                    heapq.heappush(heap, (-child.leaf_count, counter, child))
                    counter += 1
        
        # Collect all leaves under each node in the heap
        groups = []
        for _, _, node in heap:
            group = []
            stack = [node]
            while stack:
                n = stack.pop()
                if n.is_leaf:
                    group.append(n.obj_index)
                else:
                    stack.extend([n.right, n.left])
            groups.append(group)
            
        return groups

    def update_incremental(self, root: Node, threshold=0.3):
        """Update existing tree structure with new AABBs"""
        changed_nodes = []
        
        # 1. Update leaf AABBs and mark changed nodes
        def update_leaves(node):
            if node.is_leaf:
                new_aabb = self.aabbs[node.obj_index]
                if node.aabb != new_aabb:
                    object.__setattr__(node, 'aabb', new_aabb)
                    changed_nodes.append(node)
        
        # 2. Bottom-up refit of changed hierarchy
        def refit(node):
            if node.is_leaf:
                return node.aabb
            
            new_left_aabb = refit(node.left)
            new_right_aabb = refit(node.right)
            merged = self.merge_aabbs(new_left_aabb, new_right_aabb)
            
            if merged != node.aabb or node in changed_nodes:
                object.__setattr__(node, 'aabb', merged)
                changed_nodes.append(node)
            
            return merged
        
        # 3. Partial rebuild for high-overlap nodes
        def check_overlap(node):
            if node.is_leaf:
                return True
            
            overlap_ratio = self.calc_overlap(node.left.aabb, node.right.aabb)
            if overlap_ratio > threshold:
                return False
            return check_overlap(node.left) and check_overlap(node.right)
        
        update_leaves(root)
        refit(root)
        
        if not check_overlap(root):
            return self.build_top_down()  # Full rebuild if overlap too high
        
        return root

    @staticmethod
    def calc_overlap(aabb1, aabb2):
        """Calculate overlap ratio between two AABBs (0-1)"""
        min_inter = [max(aabb1[0][i], aabb2[0][i]) for i in range(3)]
        max_inter = [min(aabb1[1][i], aabb2[1][i]) for i in range(3)]
        
        intersection = max(0, (max_inter[0]-min_inter[0]) * 
                           (max_inter[1]-min_inter[1]) * 
                           (max_inter[2]-min_inter[2]))
        
        vol1 = BVHBuilder.aabb_volume(aabb1)
        vol2 = BVHBuilder.aabb_volume(aabb2)
        return intersection / min(vol1, vol2)