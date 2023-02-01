import numpy as np
from . import data_type
from . import config

peak_columns = ('start_time',
                'end_time',
                'coincidence',
                'type',
                'area',
                'center_time',
                'max_time', 
                'range_50p_area', 
                'range_90p_area',
                'rise_time_height', 
                'rise_time_area',
                'n_samples',
                'dt',
                'area_per_channel')

peak_types = (np.int64,
             np.int64,
             np.uint16,
             np.uint8,
             np.float32,
             np.float64,
             np.int64,
             np.float32,
             np.float32,
             np.float32,
             np.float32,
             np.uint32,
             np.uint32,
             np.float32)

class Peaks(data_type.DataType):
    
    def __init__(self, config_file):
        super().__init__()
        self.columns = peak_columns
        self.types = peak_types
        self.config = config.LoadConfig(config_file)
        self.onChannels = np.array(self.config['adc']['OnChannels'].split(' ')).astype('int')
        self.shapes = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, len(self.onChannels))
        self.SetDataType()
