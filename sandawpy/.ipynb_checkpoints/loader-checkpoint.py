from . import peaks

class Loader():
    def __init__(self, config_file):
        self.peaks = peaks.Peaks(config_file)
        self.datatype_dict = {'peaks' : self.peaks}
        
    def GetData(self, file, data_type, **kwargs):
        return self.datatype_dict[data_type].Load(file, **kwargs)