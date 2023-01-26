#ifndef SANDAW_EVENTS_HH
#define SANDAW_EVENTS_HH

//Internal
#include "Types.hh"
#include "Configuration.hh"

//Standard C++ Library
#include <numeric>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>

class SandixEventsProcessor {
    
    public:
        SandixEventsProcessor(SandixConfiguration* pConfig);
	    ~SandixEventsProcessor();

        void ProcessEvents(int iNumS2s, Peaks_t& Peaks, Events_t& Events, bool bSave, std::string sOutFile, std::string sRunID);
        void AddPeakInplace(Peaks_t& PeakBuffer, Peaks_t& PeakToAdd, int IndexBuf, int IndexAdd);
        void CallocPeaks(Peaks_t& PeakBuffer, int iNumEvents);
        void WritePeaks(Peaks_t& p, std::ofstream& outStream);
    
    private:
        SandixConfiguration* m_pConfig;

        float m_fDriftTime;

};

#endif