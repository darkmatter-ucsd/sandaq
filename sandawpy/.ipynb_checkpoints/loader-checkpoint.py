from . import peaks
from . import events
from . import config
import pandas as pd
import os
import numpy as np

drive_dir = '/media/daqtest/'
RUN = 'Run29'
types = ['peaks','events','waveforms']
potential_drives = os.listdir(drive_dir)
sandaw_drives = []

for i, d in enumerate(potential_drives):
    if np.isin('SanDAWEnabled.txt', os.listdir(f"{drive_dir}{d}")):
        sandaw_drives.append(d)

def UpdateRunList():
    run_modes_df = []
    run_ids_df = []
    drive_df = []
    n_type_seg_df = {t:[] for t in types}
    
    for sd in sandaw_drives:
        drive = f"{drive_dir}{sd}/{RUN}/processed/"
        rawdata_dir = f"{drive_dir}{sd}/{RUN}/rawdata/"

        run_modes = os.listdir(rawdata_dir)

        for rm in run_modes:
            runs = os.listdir(f"{rawdata_dir}{rm}")
            for r in runs:
                try:
                    allfiles = os.listdir(f"{drive}{rm}/{r}")
                    for t in types:
                        n_type_seg_df[t].append(len([f for f in allfiles if f.startswith(t)]))
                except:
                    for t in types:
                        n_type_seg_df[t].append(0)
                run_modes_df.append(rm)
                run_ids_df.append(r)
                drive_df.append(sd)

    master_runs_df = pd.DataFrame({'run_mode': run_modes_df,
        'run_id': run_ids_df,
        'n_peak_segments': n_type_seg_df['peaks'],
        'drive': drive_df,
        'n_waveform_segments': n_type_seg_df['waveforms'],
        'n_event_segments': n_type_seg_df['events']})
    
    return master_runs_df
    
class Loader():
    def __init__(self, config_file, run_list = None):
        self.peaks = peaks.Peaks(config_file)
        self.events = events.Events(config_file)
        if run_list == None:
            self.run_list = UpdateRunList()
        elif type(run_list) == str:
            self.run_list = pd.read_hdf(run_list, 'runs')
        else:
            self.run_list = run_list
        self.datatype_dict = {'peaks' : self.peaks, 'events' : self.events}
        
    def GetData(self, file, data_type, **kwargs):
        return self.datatype_dict[data_type].Load(file, **kwargs)
    
    def LoadRuns(self, run_ids, data_type, max_segments = None, **kwargs):
        if type(run_ids) == str:
            runs = self.run_list[self.run_list['run_id'] == run_ids]
        elif hasattr(run_ids, '__iter__'):
            runs = self.run_list[np.isin(self.run_list['run_id'], run_ids)]
        else:
            raise ValueError('Please either input a string for the run_id or a list of run_ids')
        
        drives = list(runs['drive'])
        run_modes = list(runs['run_mode'])
        rids = list(runs['run_id'])
        run_segments = list(runs['n_event_segments'])
        
        
        data = []
        for i, rid in enumerate(rids):
            if max_segments == None:
                d = [self.GetData(f"{drive_dir}"
                                 f"{drives[i]}/"
                                 f"{RUN}/"
                                 f"processed/"
                                 f"{run_modes[i]}/"
                                 f"{rid}/"
                                 f"{data_type}_{rid}_seg{s}.bin", data_type, **kwargs) for s in range(run_segments[i])]
            else:
                d = [self.GetData(f"{drive_dir}"
                                 f"{drives[i]}/"
                                 f"{RUN}/"
                                 f"processed/"
                                 f"{run_modes[i]}/"
                                 f"{rid}/"
                                 f"{data_type}_{rid}_seg{s}.bin", data_type, **kwargs) for s in range(min(run_segments[i],max_segments))]
            d = np.concatenate(d)
            
            run_metadata_path = [i for i in os.listdir(f"{drive_dir}{drives[i]}/{RUN}/rawdata/{run_modes[i]}/{rid}/") if i.startswith("metadata")][0]
            run_metadata = config.LoadConfig(f"{drive_dir}{drives[i]}/{RUN}/rawdata/{run_modes[i]}/{rid}/{run_metadata_path}")
            run_unix_time = np.int64(run_metadata['metadata']['UnixTime'])
            
            for t in d.dtype.names:
                if (t.endswith('time')&(t!='drift_time')):
                    d[t] += run_unix_time
            
            data.append(d)
        return np.concatenate(data)