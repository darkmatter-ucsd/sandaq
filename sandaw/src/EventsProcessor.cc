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
    PeakBuffer.saturatedSamples[IndexBuf] = (PeakToAdd.saturatedSamples[IndexAdd]);
    for (int i = 0; i<m_pConfig->m_iOnChannels.size(); i++){
        PeakBuffer.areaPerChannel[IndexBuf*m_pConfig->m_iOnChannels.size() + i] = (PeakToAdd.areaPerChannel[IndexAdd*m_pConfig->m_iOnChannels.size() + i]);
    }
}

void SandixEventsProcessor::EmplacePeakInplace(Peaks_t& PeakBuffer, Peaks_t& PeakToAdd, int IndexAdd){
    //NOTE: Does not add dt or wfsamples. that stuff is only ever useful when plotting peaks
    //and if you want to plot peaks then you might as well load the peak data

    PeakBuffer.startTimes.emplace_back(PeakToAdd.startTimes[IndexAdd]);
    PeakBuffer.endTimes.emplace_back(PeakToAdd.endTimes[IndexAdd]);
    PeakBuffer.coincidences.emplace_back(PeakToAdd.coincidences[IndexAdd]);
    PeakBuffer.areas.emplace_back(PeakToAdd.areas[IndexAdd]);
    PeakBuffer.centerTimes.emplace_back(PeakToAdd.centerTimes[IndexAdd]);
    PeakBuffer.maxTimes.emplace_back(PeakToAdd.maxTimes[IndexAdd]);
    PeakBuffer.range50pArea.emplace_back(PeakToAdd.range50pArea[IndexAdd]);
    PeakBuffer.range90pArea.emplace_back(PeakToAdd.range90pArea[IndexAdd]);
    PeakBuffer.riseTimeHeight.emplace_back(PeakToAdd.riseTimeHeight[IndexAdd]);
    PeakBuffer.riseTimeArea.emplace_back(PeakToAdd.riseTimeArea[IndexAdd]);
    PeakBuffer.saturatedSamples.emplace_back(PeakToAdd.saturatedSamples[IndexAdd]);
    for (int i = 0; i<m_pConfig->m_iOnChannels.size(); i++){
        PeakBuffer.areaPerChannel.emplace_back(PeakToAdd.areaPerChannel[IndexAdd*m_pConfig->m_iOnChannels.size() + i]);
    }
}

void SandixEventsProcessor::CallocPeaks(Peaks_t& PeakBuffer, int iNumEvents){
    //Allocate memory for the peaks
    PeakBuffer.startTimes.resize(iNumEvents);
    PeakBuffer.endTimes.resize(iNumEvents);
    PeakBuffer.coincidences.resize(iNumEvents);
    PeakBuffer.areas.resize(iNumEvents);
    PeakBuffer.centerTimes.resize(iNumEvents);
    PeakBuffer.maxTimes.resize(iNumEvents);
    PeakBuffer.range50pArea.resize(iNumEvents);
    PeakBuffer.range90pArea.resize(iNumEvents);
    PeakBuffer.riseTimeHeight.resize(iNumEvents);
    PeakBuffer.riseTimeArea.resize(iNumEvents);
    PeakBuffer.saturatedSamples.resize(iNumEvents);
    PeakBuffer.areaPerChannel.resize(iNumEvents*m_pConfig->m_iOnChannels.size());
}

void SandixEventsProcessor::ReservePeaks(Peaks_t& PeakBuffer, int iNumReserve){
    //Allocate memory for the peaks
    PeakBuffer.startTimes.reserve(iNumReserve);
    PeakBuffer.endTimes.reserve(iNumReserve);
    PeakBuffer.coincidences.reserve(iNumReserve);
    PeakBuffer.areas.reserve(iNumReserve);
    PeakBuffer.centerTimes.reserve(iNumReserve);
    PeakBuffer.maxTimes.reserve(iNumReserve);
    PeakBuffer.range50pArea.reserve(iNumReserve);
    PeakBuffer.range90pArea.reserve(iNumReserve);
    PeakBuffer.riseTimeHeight.reserve(iNumReserve);
    PeakBuffer.riseTimeArea.reserve(iNumReserve);
    PeakBuffer.saturatedSamples.reserve(iNumReserve);
    PeakBuffer.areaPerChannel.reserve(iNumReserve*m_pConfig->m_iOnChannels.size());
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
    outStream.write((char*)&p.saturatedSamples[0], sizeof(uint32_t) * iNumEntries);
    outStream.write((char*)&p.areaPerChannel[0], sizeof(float) * m_pConfig->m_iNChannels * iNumEntries);
}

uint32_t SandixEventsProcessor::FindLoneS1s(std::vector<int64_t>& iWindowStarts, std::vector<int64_t>& iWindowEnds,
                                        Peaks_t& Peaks, uint32_t iNumLeftOverPeaks){
    /*************Finder of lone S1s, to be ran after the rest of the events are processed**************/
    //Lone S1s feature no drift time, so they are the same event type as peaks

    CallocPeaks(LoneS1s, iNumLeftOverPeaks);
    
    //Make the concatenated windows for binning
    std::vector<int64_t> iConcatenatedWindows;
    iConcatenatedWindows.reserve(2*iWindowStarts.size());
    for (int i = 0; i < iWindowStarts.size(); i++){
        iConcatenatedWindows.emplace_back(iWindowStarts[i]);
        iConcatenatedWindows.emplace_back(iWindowEnds[i]);
    }

    int LoneS1Count = 0;
    for (int j = 0; j < Peaks.types.size(); j++){
        if ((Peaks.types[j]==1)&&(Peaks.areas[j]>m_pConfig->m_fLoneS1AreaThresh)){
            auto it = std::upper_bound(iConcatenatedWindows.begin(), iConcatenatedWindows.end(), Peaks.startTimes[j]);
            int index = std::distance(iConcatenatedWindows.begin(), it);
            if (index%2==0){
                AddPeakInplace(LoneS1s, Peaks, LoneS1Count, j);
                LoneS1Count++;
                std::cout << "Reached a lone S1! Lone S1 count is "<< LoneS1Count <<"\n";
                std::cout << "This lone S1 has an area of "<< LoneS1s.areas[LoneS1Count-1]<<"\n";
            }
        }
    }
    
    CallocPeaks(LoneS1s, LoneS1Count);

    std::cout << "Lone S1 size "<<LoneS1s.areas.size()<<"\n";

    return LoneS1Count;
}


void SandixEventsProcessor::ProcessEvents(int iNumS2s, Peaks_t& Peaks, bool bSave, std::string sOutFile, std::string sRunID) {
    //Setup the output file saving
	std::string sOutFileEvents;
    std::string sOutFileLoneS1;

	if (sOutFile == "" || sRunID =="") {
		std::cout << "ERROR: Please provide BOTH a Run ID AND an output file" << std::endl;
		exit(EXIT_FAILURE);
	}
    sOutFileEvents = sOutFile + "events_" + sRunID + ".bin";
    sOutFileLoneS1 = sOutFile + "lone_s1_" + sRunID + ".bin";

	std::ofstream OutFileEvents(sOutFileEvents, std::ios::out | std::ios::binary);
    std::ofstream OutFileLoneS1(sOutFileLoneS1, std::ios::out | std::ios::binary);
    
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
    // std::cout<< "Allocated memory for the events\n";

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
        // std::cout << "processing event "<< w << " of " << iNumEvents << "\n";
        while ((iPeakIndex<Peaks.startTimes.size())&&(Peaks.startTimes[iPeakIndex]<=Events.eventWindowEndTime[w])){
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

    /******************************Dealing with the lone S1s*******************************/
    uint32_t iNumLeftoverPeaks = std::accumulate(Events.numS1s.begin(), Events.numS1s.end(), 0) + std::accumulate(Events.numS2s.begin(), Events.numS2s.end(), 0);
    iNumLeftoverPeaks = Peaks.types.size() - iNumLeftoverPeaks;
    uint32_t iNumLoneS1s = FindLoneS1s(Events.eventWindowStartTime, Events.eventWindowEndTime, Peaks, iNumLeftoverPeaks);

    
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

        OutFileLoneS1.write((char*)&iNumLoneS1s, sizeof(unsigned int));
        WritePeaks(LoneS1s, OutFileLoneS1);
        OutFileLoneS1.close();
	}
}