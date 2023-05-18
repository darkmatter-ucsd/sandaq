#ifndef SANDAW_TYPES_HH
#define SANDAW_TYPES_HH

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

typedef struct {
    std::vector<uint16_t> channels;
    std::vector<int64_t> triggerTimes;
    std::vector<int64_t> startTimes;
    std::vector<int64_t> endTimes;
    std::vector<uint16_t> baselines;
    std::vector<uint32_t> saturatedSamples;
    std::vector<std::vector<int>> waveforms;
    std::vector<uint32_t> windowNumber; //Only used for ZLE (i.e. triggered acquisition)
} Hits_t;

typedef struct {
    std::vector<int64_t> startTimes;
    std::vector<int64_t> endTimes;
    std::vector<uint16_t> coincidences;
    std::vector<uint8_t> types;
    std::vector<float> areas;
    std::vector<double> centerTimes;
    std::vector<int64_t> maxTimes;
    std::vector<float> range50pArea;
    std::vector<float> range90pArea;
    std::vector<float> riseTimeHeight;
    std::vector<float> riseTimeArea;
    std::vector<uint32_t> wfSamples;
    std::vector<uint32_t> dt;
    std::vector<uint32_t> saturatedSamples;
    std::vector<float> areaPerChannel;
} Peaks_t;


typedef struct {
    std::vector<int64_t> eventWindowStartTime;
    std::vector<int64_t> eventWindowEndTime;
    std::vector<Peaks_t> peakProperties;
    std::vector<float> driftTime;
    std::vector<uint32_t> numS1s;
    std::vector<uint32_t> numS2s; 
} Events_t;

#endif