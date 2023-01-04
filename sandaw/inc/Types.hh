#ifndef SANDAW_TYPES_HH
#define SANDAW_TYPES_HH

#include <iostream>
#include <vector>

typedef struct {
    std::vector<uint16_t> channels;
    std::vector<int64_t> triggerTimes;
    std::vector<int64_t> startTimes;
    std::vector<int64_t> endTimes;
    std::vector<uint16_t> baselines;
    std::vector<std::vector<int>> waveforms;
} Hits_t;

typedef struct {
    std::vector<int64_t> startTimes;
    std::vector<int64_t> endTimes;
    std::vector<uint16_t> coincidences;
    std::vector<double> areas;
    std::vector<double> centerTimes;
    std::vector<double> maxTimes;
    std::vector<double> range50pArea;
    std::vector<double> range90pArea;
    std::vector<double> riseTimeHeight;
    std::vector<double> riseTimeArea;
    std::vector<std::vector<double>> combinedWaveforms;
} Peaks_t;

#endif