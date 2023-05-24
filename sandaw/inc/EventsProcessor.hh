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

        Events_t Events;
        Peaks_t LoneS1s;

        void ProcessEvents(int iNumS2s, Peaks_t& Peaks, bool bSave, std::string sOutFile, std::string sRunID);
        void AddPeakInplace(Peaks_t& PeakBuffer, Peaks_t& PeakToAdd, int IndexBuf, int IndexAdd);
        void EmplacePeakInplace(Peaks_t& PeakBuffer, Peaks_t& PeakToAdd, int IndexAdd);
        void CallocPeaks(Peaks_t& PeakBuffer, int iNumEvents);
        void ReservePeaks(Peaks_t& PeakBuffer, int iNumReserve);
        void WritePeaks(Peaks_t& p, std::ofstream& outStream);

        //Lone S1 processing
        uint32_t FindLoneS1s(std::vector<int64_t>& iWindowStarts, std::vector<int64_t>& iWindowEnds, Peaks_t& Peaks, uint32_t iNumLeftOverPeaks);
    
    private:
        SandixConfiguration* m_pConfig;

        float m_fDriftTime;

};

#endif