from typing import List, Tuple
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from mpl_toolkits.mplot3d import art3d
from bvh import BVHBuilder, Node
from data_reader import AABBDataHandler
import numpy as np

class BVHAnimator:
    def __init__(self, data_handler: AABBDataHandler):
        self.data_handler = data_handler
        self.fig = plt.figure(figsize=(12, 10))
        self.ax = self.fig.add_subplot(111, projection='3d')
        self.tree_fig, self.tree_ax = plt.subplots(figsize=(16, 10))
        self._setup_axes()
        
    def _setup_axes(self):
        self.ax.set_proj_type('ortho')         
        self.ax.view_init(elev=15, azim=-45,   
                        vertical_axis='y')   
        self.ax.set_xlim(-3, 3)
        self.ax.set_ylim(-3, 3)
        self.ax.set_zlim(-3, 3)
        self.ax.set_xlabel('X')
        self.ax.set_ylabel('Y')
        self.ax.set_zlabel('Z')
        
    def _draw_original_boxes(self, aabbs: List[Tuple]):
        for aabb in aabbs:
            min_coords, max_coords = aabb
            vertices = np.array([
                [min_coords[0], min_coords[1], min_coords[2]],
                [max_coords[0], min_coords[1], min_coords[2]],
                [max_coords[0], max_coords[1], min_coords[2]],
                [min_coords[0], max_coords[1], min_coords[2]],
                [min_coords[0], min_coords[1], max_coords[2]],
                [max_coords[0], min_coords[1], max_coords[2]],
                [max_coords[0], max_coords[1], max_coords[2]],
                [min_coords[0], max_coords[1], max_coords[2]]
            ])
            edges = [
                [vertices[0], vertices[1], vertices[2], vertices[3], vertices[0]],
                [vertices[4], vertices[5], vertices[6], vertices[7], vertices[4]],
                [vertices[0], vertices[4]], [vertices[1], vertices[5]],
                [vertices[2], vertices[6]], [vertices[3], vertices[7]]
            ]
            box = art3d.Line3DCollection(edges, colors='blue', alpha=0.3)
            self.ax.add_collection3d(box)
            
    def _draw_bvh(self, root: Node):
        def recursive_draw(node: Node):
            if node is None:
                return
            if node.is_leaf:
                BVHBuilder.draw_aabb(self.ax, node.aabb, color='green', alpha=0.5)
            else:
                BVHBuilder.draw_aabb(self.ax, node.aabb, color='red', alpha=0.2)
            recursive_draw(node.left)
            recursive_draw(node.right)
        recursive_draw(root)
        
    def update(self, frame: int):
        self.ax.clear()
        self._setup_axes()
        
        # Get data and build BVH
        aabbs = self.data_handler.get_frame_aabbs(frame)
        bvh_root = BVHBuilder(aabbs).build_top_down()
        
        # Draw elements
        self._draw_original_boxes(aabbs)
        self._draw_bvh(bvh_root)
        
        self.ax.set_title(f'Time: {self.data_handler.get_frame_time(frame):.3f} seconds')
        return []

    def animate(self):
        subdomain_vis = SubdomainVisualizer()
        
        def update_combined(frame):
            # Clear both axes
            self.ax.clear()
            self.tree_ax.clear()
            
            # Update 3D visualization
            self._setup_axes()
            aabbs = self.data_handler.get_frame_aabbs(frame)
            bvh_root = BVHBuilder(aabbs).build_top_down()
            self._draw_original_boxes(aabbs)
            self._draw_bvh(bvh_root)
            self.ax.set_title(f'Time: {self.data_handler.get_frame_time(frame):.3f} seconds')
            
            # Update tree visualization
            BVHBuilder.draw_tree_structure(self.tree_ax, bvh_root)
            self.tree_ax.set_title(f"BVH Tree Structure at Time: {self.data_handler.get_frame_time(frame):.3f} seconds")
            
            # New subdomain visualization
            num_subdomains = 4  # Can be 2, 4, 8, etc
            groups = BVHBuilder(aabbs).get_subdomains(bvh_root, num_subdomains)
            subdomain_vis.visualize(aabbs, groups)
            
            # Force both figures to update
            self.fig.canvas.draw_idle()
            self.tree_fig.canvas.draw_idle()
            return []

        # Create a single animation that updates both figures
        ani = animation.FuncAnimation(
            self.fig,
            update_combined,
            frames=self.data_handler.num_frames,
            interval=5,
            blit=False
        )
        plt.show()

class SubdomainVisualizer:
    def __init__(self):
        self.fig = plt.figure(figsize=(12, 10))
        self.ax = self.fig.add_subplot(111, projection='3d')
        self._setup_axes()
        self.colors = plt.cm.tab20.colors  # 20 distinct colors
        
    def _setup_axes(self):
        self.ax.set_proj_type('ortho')         
        self.ax.view_init(elev=15, azim=-45,   
                        vertical_axis='y')   
        self.ax.set_xlim(-3, 3)
        self.ax.set_ylim(-3, 3)
        self.ax.set_zlim(-3, 3)
        self.ax.set_xlabel('X')
        self.ax.set_ylabel('Y')
        self.ax.set_zlabel('Z')
        
    def visualize(self, aabbs: List[Tuple], groups: List[List[int]]):
        self.ax.clear()
        self._setup_axes()
        
        for group_idx, obj_indices in enumerate(groups):
            color = self.colors[group_idx % len(self.colors)]
            for idx in obj_indices:
                aabb = aabbs[idx]
                BVHBuilder.draw_aabb(self.ax, aabb, color=color, alpha=0.6)
                
        plt.title(f"Subdomain Groups ({len(groups)} partitions)")
        plt.show(block=False)

# Usage
if __name__ == "__main__":
    data_handler = AABBDataHandler("processed_output.csv")
    animator = BVHAnimator(data_handler)
    animator.animate()