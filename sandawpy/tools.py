"""
Analysis tools
"""

def asymmetry(area_per_channel):
    top_bottom = area_per_channel.reshape(len(area_per_channel), 2,4)
    top_bottom = np.sum(top_bottom, axis = 2)
    s2_asym = -np.diff(top_bottom).flatten()/np.sum(top_bottom, axis = 1).flatten()
    return s2_asym

