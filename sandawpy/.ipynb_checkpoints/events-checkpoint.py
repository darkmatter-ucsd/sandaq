import numpy as np
from . import data_type
from . import config

#Event peaks are slightly different from regular peaks, they don't have the number of waveform samples copied over

event_peak_columns = ('start_time',
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

event_peak_types = (np.int64,
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

modifiers = ("main", "alt")
types = (1, 2)

event_columns = ["window_start_time", "window_end_time"]
event_columns += [f"{m}_s{t}_{pc}" for m in modifiers for t in types for pc in event_peak_columns]
event_columns += ["drift_time", "n_s1", "n_s2"]
event_columns = tuple(event_columns)

event_types = [np.int64, np.int64]
event_types += [t for m in modifiers for t in types for t in event_peak_types]
event_types += [np.float32, np.uint32, np.uint32]
event_types = tuple(event_types)


class Events(data_type.DataType):
    
    def __init__(self, config_file):
        super().__init__()
        self.columns = event_columns
        self.types = event_types
        self.config = config.LoadConfig(config_file)
        self.onChannels = np.array(self.config['adc']['OnChannels'].split(' ')).astype('int')
        self.peakShapes = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, len(self.onChannels))
        self.shapes = [1,1]
        self.shapes += [s for m in modifiers for t in types for s in self.peakShapes]
        self.shapes += [1,1,1]
        self.shapes = tuple(self.shapes)
        self.SetDataType()