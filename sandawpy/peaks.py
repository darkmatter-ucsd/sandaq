import numpy as np
from . import data_type
from . import config

peak_columns = ('start_time',
                'end_time',
                'coincidence',
                'area',
                'center_time',
                'max_time', 
                'range_50p_area', 
                'range_90p_area',
                'rise_time_height', 
                'rise_time_area',
                'data')

peak_types = (np.int64,
             np.int64,
             np.uint16,
             np.float64,
             np.float64,
             np.float64,
             np.float64,
             np.float64,
             np.float64,
             np.float64,
             np.float64)



class Peaks(data_type.DataType):
    
    def __init__(self, config_file):
        super().__init__()
        self.columns = peak_columns
        self.types = peak_types
        self.config = config.LoadConfig(config_file)
        self.shapes = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, int(self.config['peaks']['MaxSamples']))
        self.SetDataType()