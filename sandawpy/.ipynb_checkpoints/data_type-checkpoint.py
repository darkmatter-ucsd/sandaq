import numpy as np

class DataType():
    
    def __init__(self):
        self.columns = None
        self.types = None
        self.shapes = None
        self.shape_dict = None
        self.offsets = None
    
    def SetDataType(self):
        self.shape_dict = dict(zip(self.columns, self.shapes))
        offsets = np.pad(np.cumsum([size_per_type[t]*self.shapes[i] for i, t in enumerate(self.types)])[:-1],(1,0))
        self.offsets = dict(zip(self.columns, offsets))
        self.dtypes = []
        for (c, t, s) in zip(self.columns, self.types, self.shapes):
            if s == 1:
                self.dtypes.append((c, t))
            else:
                self.dtypes.append((c, t, s))
                
        self.dtype_dict = dict(zip(self.columns, self.dtypes))
    
    def Load(self, file, cols = None):
        if hasattr(cols, '__iter__'):
            if ~np.all(np.isin(cols, self.columns)):
                raise ValueError('Column list must be a subset of the available columns')
        elif cols == None:
            cols = self.columns
        else:
            raise TypeError('cols must be an iterable')
        
        this_dtype = [self.dtype_dict[c] for c in cols]
        
        try:
            n_entries = np.fromfile(file, dtype = 'uint32', count = 1)
        except:
            return np.array([], dtype = this_dtype)
        
        if len(n_entries)==0:
            return np.array([], dtype = this_dtype)
        
        n_entries = n_entries[0]
        
        data = np.zeros(n_entries, dtype = this_dtype)
        for i, t in enumerate(this_dtype):
            temp_arr = np.fromfile(file, dtype = t[1],
                                   offset = self.offsets[t[0]]*n_entries+4,
                                   count = self.shape_dict[t[0]]*n_entries)
            if self.shape_dict[t[0]]>1:
                temp_arr = temp_arr.reshape(n_entries, self.shape_dict[t[0]])

            data[t[0]] = temp_arr
        return data

size_per_type = {np.uint8 : 1,
                 np.uint16 : 2,
                 np.int16 : 2,
                 np.uint32 : 4,
                 np.int32 : 4,
                 np.float32 : 4,
                 np.int64 : 8,
                 np.uint64 : 8,
                 np.float64 : 8}