import subprocess
import os
import argparse
from multiprocessing import Pool
from multiprocessing.dummy import Pool as ThreadPool
import numpy as np
import time


def RunProcess(config, binary_file, outdir, tag):
    subprocess.run(["./../sandaw/build/process",
     "-c", config,
     "-f", binary_file,
     "-p", outdir,
     "-r", tag])

    return f"Processed {tag}"

ms = 1e-3

if __name__ == "__main__":

    start = time.time()

    parser = argparse.ArgumentParser(description="For processing a single run")
    parser.add_argument('--config',
        help = "Configuration file for processing",
        dest = 'config', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--rawdata_path',
        help = "Path for the raw data, should contain the run-mode folders",
        dest = 'rawdata_path', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--run_mode',
        help = "Run mode",
        dest = 'run_mode', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--run_id',
        help = "Run ID",
        dest = 'run_id', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--outdir',
        help = "Output file directory",
        dest = 'outdir', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--n_proc',
        help = "Number of processes to run at once",
        dest = 'n_proc', action = 'store', required = False, type = int, default = 4)
    
    args = parser.parse_args()

    run_list = os.listdir(f"{args.rawdata_path}{args.run_mode}/{args.run_id}/")
    run_list = [i for i in run_list if i[:len('metadata')]!='metadata']
    # for i, r in enumerate(run_list):
    #     if r[:len(args.run_mode)] == args.run_mode:
    #         print(r)

    output_run_mode_path = f"{args.outdir}{args.run_mode}"
    output_run_id_path = f"{output_run_mode_path}/{args.run_id}/"

    if not os.path.exists(output_run_mode_path):
        os.mkdir(output_run_mode_path, mode = 0o777)

    if not os.path.exists(output_run_id_path):
        os.mkdir(output_run_id_path, mode = 0o777)
    
    
    segments = [r.split('_')[-2] for r in run_list]

    for ch in range(np.ceil(len(run_list)/args.n_proc).astype('int')):
        if ch == np.ceil(len(run_list)/args.n_proc) - 1:
            n_segs = int(len(run_list)- args.n_proc * int(len(run_list)/args.n_proc))
        else:
            n_segs = args.n_proc

        process_args = [(args.config,
            f"{args.rawdata_path}{args.run_mode}/{args.run_id}/{run_list[i]}",
            output_run_id_path,
            f"{args.run_id}_{segments[i]}") for i in range(args.n_proc*ch, args.n_proc*ch + n_segs)]
        
        with Pool() as pool:
            results = pool.starmap(RunProcess, process_args)

            for r in results:
                print(r)

        time.sleep(100*ms)
    
    print(f"\n\nProgram took: {time.time()-start} seconds")