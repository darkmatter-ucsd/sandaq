import numpy as np
import numba
import json
import configparser

def LoadConfig(config_file):
    cfg = configparser.ConfigParser()
    cfg.read(config_file)
    return cfg