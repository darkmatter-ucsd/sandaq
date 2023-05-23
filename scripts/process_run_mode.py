import subprocess
import os
import argparse
from multiprocessing import Pool
from multiprocessing.dummy import Pool as ThreadPool
import numpy as np
import time


def RunProcess(config, binary_file, outdir, tag, metadata):
    subprocess.run(["./../sandaw/build/process",
     "-c", config,
     "-f", binary_file,
     "-p", outdir,
     "-r", tag,
     "-m", metadata])

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
    parser.add_argument('--outdir',
        help = "Output file directory",
        dest = 'outdir', action = 'store', required = True, type = str, default = None)
    parser.add_argument('--n_proc',
        help = "Number of processes to run at once",
        dest = 'n_proc', action = 'store', required = False, type = int, default = 4)
    
    args = parser.parse_args()

    run_ids = os.listdir(f"{args.rawdata_path}{args.run_mode}/")
    output_run_mode_path = f"{args.outdir}{args.run_mode}"

    if not os.path.exists(output_run_mode_path):
        os.mkdir(output_run_mode_path, mode = 0o777)


    for r_id in run_ids:
        run_list = os.listdir(f"{args.rawdata_path}{args.run_mode}/{r_id}/")
        metadata = [i for i in run_list if i.startswith('metadata')][0]
        run_list = [i for i in run_list if ((i[:len('metadata')]!='metadata') and (i[:len('deadtimes')]!='deadtimes'))]

        output_run_id_path = f"{output_run_mode_path}/{r_id}/"

        if not os.path.exists(output_run_id_path):
            os.mkdir(output_run_id_path, mode = 0o777)
        
        
        segments = np.array([r.split('_')[-2] for r in run_list])
        segnums = np.array([s[3:] for s in segments])
        resort_segs = np.argsort(segnums.astype('int'))
        segments = segments[resort_segs]

        run_list = (np.array(run_list)[resort_segs])

        for ch in range(np.ceil(len(run_list)/args.n_proc).astype('int')):
            if ch == np.ceil(len(run_list)/args.n_proc) - 1:
                n_segs = int(len(run_list)- args.n_proc * int(len(run_list)/args.n_proc))
            else:
                n_segs = args.n_proc

            process_args = [(args.config,
                f"{args.rawdata_path}{args.run_mode}/{r_id}/{run_list[i]}",
                output_run_id_path,
                f"{r_id}_{segments[i]}",
                f"{args.rawdata_path}{args.run_mode}/{r_id}/{metadata}") for i in range(args.n_proc*ch, args.n_proc*ch + n_segs)]
            
            with Pool() as pool:
                results = pool.starmap(RunProcess, process_args)

                pool.close()
                pool.terminate()
                pool.join()

                for r in results:
                    print(r)

            time.sleep(100*ms)

        print(f"\n\n\nRun ID {r_id} processed!\n\n\n")

    print(f"\n\nProgram took: {time.time()-start} seconds")
