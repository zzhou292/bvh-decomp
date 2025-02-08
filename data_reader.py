from typing import List, Tuple
import pandas as pd
import numpy as np

class AABBDataHandler:
    def __init__(self, csv_path: str):
        self.df = pd.read_csv(csv_path, header=None, skiprows=1)
        self.num_frames = len(self.df)
        self.num_objects = 70
        #40 or 70
        
    def get_frame_aabbs(self, frame: int) -> List[Tuple]:
        row = self.df.iloc[frame]
        aabbs = []
        for i in range(self.num_objects):
            idx = 1 + i*6
            min_coords = tuple(row[idx:idx+3].values.astype(float))
            max_coords = tuple(row[idx+3:idx+6].values.astype(float))
            aabbs.append((min_coords, max_coords))
        return aabbs
    
    def get_frame_time(self, frame: int) -> float:
        return self.df.iloc[frame][0]
