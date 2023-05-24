import numpy as np
from . import data_type
from . import config

#Event peaks are slightly different from regular peaks, they don't have the number of waveform samples copied over

lones1_columns = ('start_time',
                'end_time',
                'coincidence',
                'area',
                'center_time',
                'max_time', 
                'range_50p_area', 
                'range_90p_area',
                'rise_time_height', 
                'rise_time_area',
                'saturated_samples',
                'area_per_channel')

lones1_types = (np.int64,
             np.int64,
             np.uint16,
             np.float32,
             np.float64,
             np.int64,
             np.float32,
             np.float32,
             np.float32,
             np.float32,
             np.uint32,
             np.float32)


class LoneS1(data_type.DataType):
    
    def __init__(self, config_file):
        super().__init__()
        self.columns = lones1_columns
        self.types = lones1_types
        self.config = config.LoadConfig(config_file)
        self.onChannels = np.array(self.config['adc']['OnChannels'].split(' ')).astype('int')
        self.shapes = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, len(self.onChannels))
        self.SetDataType()