#include "EventsProcessor.hh"
#include "Tools.hh"

SandixEventsProcessor::SandixEventsProcessor(SandixConfiguration* pConfig) {
    m_pConfig = pConfig;
}

SandixEventsProcessor::~SandixEventsProcessor() {}

void SandixEventsProcessor::AddPeakInplace(Peaks_t& PeakBuffer, Peaks_t& PeakToAdd, int IndexBuf, int IndexAdd){
    //NOTE: Does not add dt or wfsamples. that stuff is only ever useful when plotting peaks
    //and if you want to plot peaks then you might as well load the peak data

    PeakBuffer.startTimes[IndexBuf] = (PeakToAdd.startTimes[IndexAdd]);
    PeakBuffer.endTimes[IndexBuf] = (PeakToAdd.endTimes[IndexAdd]);
    PeakBuffer.coincidences[IndexBuf] = (PeakToAdd.coincidences[IndexAdd]);
    PeakBuffer.areas[IndexBuf] = (PeakToAdd.areas[IndexAdd]);
    PeakBuffer.centerTimes[IndexBuf] = (PeakToAdd.centerTimes[IndexAdd]);
    PeakBuffer.maxTimes[IndexBuf] = (PeakToAdd.maxTimes[IndexAdd]);
    PeakBuffer.range50pArea[IndexBuf] = (PeakToAdd.range50pArea[IndexAdd]);
    PeakBuffer.range90pArea[IndexBuf] = (PeakToAdd.range90pArea[IndexAdd]);
    PeakBuffer.riseTimeHeight[IndexBuf] = (PeakToAdd.riseTimeHeight[IndexAdd]);
    PeakBuffer.riseTimeArea[IndexBuf] = (PeakToAdd.riseTimeArea[IndexAdd]);
    for (int i = 0; i<m_pConfig->m_iOnChannels.size(); i++){
        PeakBuffer.areaPerChannel[IndexBuf*m_pConfig->m_iOnChannels.size() + i] = (PeakToAdd.areaPerChannel[IndexAdd*m_pConfig->m_iOnChannels.size() + i]);
    }
}

void SandixEventsProcessor::CallocPeaks(Peaks_t& PeakBuffer, int iNumEvents){
    //Allocate memory for the peaks
    PeakBuffer.startTimes.resize(iNumEvents, 0);
    PeakBuffer.endTimes.resize(iNumEvents, 0);
    PeakBuffer.coincidences.resize(iNumEvents, 0);
    PeakBuffer.areas.resize(iNumEvents, 0.);
    PeakBuffer.centerTimes.resize(iNumEvents, 0.);
    PeakBuffer.maxTimes.resize(iNumEvents, 0);
    PeakBuffer.range50pArea.resize(iNumEvents, 0.);
    PeakBuffer.range90pArea.resize(iNumEvents, 0.);
    PeakBuffer.riseTimeHeight.resize(iNumEvents, 0.);
    PeakBuffer.riseTimeArea.resize(iNumEvents, 0.);
    PeakBuffer.areaPerChannel.resize(iNumEvents*m_pConfig->m_iOnChannels.size(), 0.);
}

void SandixEventsProcessor::WritePeaks(Peaks_t& p, std::ofstream& outStream){
    unsigned int iNumEntries = p.startTimes.size();
    outStream.write((char*)&p.startTimes[0], sizeof(int64_t) * iNumEntries);
    outStream.write((char*)&p.endTimes[0], sizeof(int64_t) * iNumEntries);
    outStream.write((char*)&p.coincidences[0], sizeof(uint16_t) * iNumEntries);
    outStream.write((char*)&p.areas[0], sizeof(float) * iNumEntries);
    outStream.write((char*)&p.centerTimes[0], sizeof(double) * iNumEntries);
    outStream.write((char*)&p.maxTimes[0], sizeof(int64_t) * iNumEntries);
    outStream.write((char*)&p.range50pArea[0], sizeof(float) * iNumEntries);
    outStream.write((char*)&p.range90pArea[0], sizeof(float) * iNumEntries);
    outStream.write((char*)&p.riseTimeHeight[0], sizeof(float) * iNumEntries);
    outStream.write((char*)&p.riseTimeArea[0], sizeof(float) * iNumEntries);
    outStream.write((char*)&p.areaPerChannel[0], sizeof(float) * m_pConfig->m_iNChannels * iNumEntries);
}

void SandixEventsProcessor::ProcessEvents(int iNumS2s, Peaks_t& Peaks, Events_t& Events, bool bSave, std::string sOutFile, std::string sRunID) {
    	//Setup the output file saving
	std::string sOutFileEvents;

	if (sOutFile == "" || sRunID =="") {
		std::cout << "ERROR: Please provide BOTH a Run ID AND an output file" << std::endl;
		exit(EXIT_FAILURE);
	}
    sOutFileEvents = sOutFile + "events_" + sRunID + ".bin";
	std::ofstream OutFileEvents(sOutFileEvents, std::ios::out | std::ios::binary);
    
    Peaks_t PeakPropertyPlaceholder;

    std::vector<int64_t> iS2Starts, iS2Lookback;
    iS2Starts.reserve(iNumS2s);
    iS2Lookback.reserve(iNumS2s);

    for (int i = 0; i<Peaks.types.size(); i++){
        if (Peaks.types[i]==2){
            iS2Starts.push_back(Peaks.startTimes[i]);
            iS2Lookback.push_back(Peaks.startTimes[i]-m_pConfig->m_iMaxDriftTime);
        }
    }

    MergeWindows(iS2Lookback, iS2Starts, Events.eventWindowStartTime, Events.eventWindowEndTime);
    unsigned int iNumEvents = Events.eventWindowStartTime.size();
    
    //Set up the buffer for the entire dataset
    //Event peak properties are: 0 = Main S1, 1 = Main S2, 2 = Alt S1, 3 = Alt S2
    for (int i = 0; i<4; i++){
        Events.peakProperties.push_back(PeakPropertyPlaceholder);
        CallocPeaks(Events.peakProperties[i], iNumEvents);
    }
    Events.driftTime.resize(iNumEvents, 0.);
    Events.numS1s.resize(iNumEvents, 0);
    Events.numS2s.resize(iNumEvents, 0);
    std::cout<< "Allocated memory for the events\n";

    //Set up the buffer for a single event's peaks
    uint8_t iCurrentType;
    int iNTypesInEvent[2] = {0, 0};
    int iAreaSortedIndices[2];
    std::vector<Peaks_t> EventPeaksBuffer(2);
    for (int i = 0; i<2; i++){
        CallocPeaks(EventPeaksBuffer[i], m_pConfig->m_iMaxPeaksInEvent);
    }

    //Primary event processing loop
    int iPeakIndex = 0, iPeakCount = 0;
    for (int w = 0; w < iNumEvents; w++) {
        
        while (Peaks.startTimes[iPeakIndex]<=Events.eventWindowEndTime[w]){
            iCurrentType = Peaks.types[iPeakIndex];
            //If the peak is an S1 or an S2 then increment the peak count, and copy the data over
            //If we've reached more than the total allowable peaks in an event, however, skip
            if ((iCurrentType!=0) && (Peaks.startTimes[iPeakIndex]>=Events.eventWindowStartTime[w]) && (iPeakCount<m_pConfig->m_iMaxPeaksInEvent)){
                AddPeakInplace(EventPeaksBuffer[iCurrentType-1], Peaks, iNTypesInEvent[iCurrentType-1], iPeakIndex);
                iNTypesInEvent[iCurrentType-1]+=1;
                iPeakCount++;
            }
            //Add the peak index
            iPeakIndex++;
        }
        Events.numS1s[w] = iNTypesInEvent[0];
        Events.numS2s[w] = iNTypesInEvent[1];

        for (int t = 0; t<2; t++){
            switch (iNTypesInEvent[t]){
                case 0:
                    break;
                case 1:
                    AddPeakInplace(Events.peakProperties[t], EventPeaksBuffer[t], w, 0);
                    break;
                default:
                    LargestTwoArgs(EventPeaksBuffer[t].areas, iNTypesInEvent[t], iAreaSortedIndices);
                    //Add the main S1 or S2
                    AddPeakInplace(Events.peakProperties[t], EventPeaksBuffer[t], w, iAreaSortedIndices[0]);
                    //Add the alternate S1 or S2
                    AddPeakInplace(Events.peakProperties[t+2], EventPeaksBuffer[t], w, iAreaSortedIndices[1]);
            }
        }

        if ((iNTypesInEvent[0]>0)&&(iNTypesInEvent[1]>0)){
            Events.driftTime[w] = (float) (Events.peakProperties[1].centerTimes[w] - Events.peakProperties[0].centerTimes[w]);
        }

        iPeakCount = 0;
        iNTypesInEvent[0] = 0;
        iNTypesInEvent[1] = 0;
    }

    
    if (bSave){
		OutFileEvents.write((char*)&iNumEvents, sizeof(unsigned int));
        OutFileEvents.write((char*)&Events.eventWindowStartTime[0], sizeof(int64_t) * iNumEvents);
        OutFileEvents.write((char*)&Events.eventWindowEndTime[0], sizeof(int64_t) * iNumEvents);
        for (int i = 0; i < 4; i++){
            WritePeaks(Events.peakProperties[i], OutFileEvents);
        }
        OutFileEvents.write((char*)&Events.driftTime[0], sizeof(float) * iNumEvents);
        OutFileEvents.write((char*)&Events.numS1s[0], sizeof(uint32_t) * iNumEvents);
        OutFileEvents.write((char*)&Events.numS2s[0], sizeof(uint32_t) * iNumEvents);

		OutFileEvents.close();
	}
}