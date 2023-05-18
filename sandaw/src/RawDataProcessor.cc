#include "RawDataProcessor.hh"
#include "Timer.hh"

SandixRawDataProcessor::SandixRawDataProcessor(SandixConfiguration* pConfig) {
    m_pConfig = pConfig;
}

SandixRawDataProcessor::~SandixRawDataProcessor() {
    // free(m_iData);
}

void SandixRawDataProcessor::SetData(std::string& sBinFile) {
    std::ifstream inputData(sBinFile.c_str());
    inputData.seekg(0, std::ios_base::end);
    std::size_t size = inputData.tellg();
    inputData.seekg(0, std::ios_base::beg);

    m_iData.resize(size / sizeof(uint32_t), 0);
    inputData.read((char*)&m_iData[0], size);    
    inputData.close();

    // std::cout << "\nFirst 20 elements: ";
    // for (int i = 0; i<20; i++){
    //     std::cout << m_iData[i] << " ";
    // }
    // std::cout << std::endl;

    m_iNumWords = size / sizeof(uint32_t);

    // std::cout << "Binary file is " << size << " bytes" << std::endl;
}

void SandixRawDataProcessor::SetRootFile() {
    m_tHitsFile = new TFile(m_sOutputFile.c_str(), "RECREATE");
    m_tHitsTree = new TTree("hits", "Raw per-channel hits");

    m_tHitsTree->Branch("window_number", &m_iWindowNumber, "window_number/i");
    m_tHitsTree->Branch("channel", &m_iChannel, "channel/s");
    m_tHitsTree->Branch("trigger_time", &m_iTriggerTime, "trigger_time/l");
    m_tHitsTree->Branch("start_time", &m_iStartTime, "start_time/l");
    m_tHitsTree->Branch("end_time", &m_iEndTime, "end_time/l");
    m_tHitsTree->Branch("baseline", &m_iBaseline, "baseline/s");
    m_tHitsTree->Branch("waveform", "vector<int>", &m_iWaveform);
}



void SandixRawDataProcessor::ProcessRawData(std::string& sBinFile,
    bool bSave = false) {
    
    // std::cout << "Loading the binary file took: ";
    // {
    //     Timer timer;
    SetData(sBinFile);
    // }

    if (bSave)
        SetRootFile();

    // Setting the data within the function itself

    bool bEventHeader = true; //flag for reading the event header
    bool bChannelHeader = false; //flag for reading the channel header
    bool bEventBody = false; //flag for reading the event body

    unsigned int iEventSize; //Used for both V1725 and V1720
    unsigned int iChannelMaskO1, iChannelMaskO2; //Channel mask for octants 1 and 2 (only used in V1725)
    unsigned int iChannelMask; //Channel mask combined (Used for both V1725 and V1720)
    unsigned int iNumOnChannels;

    //********** V1720 only stuff **********
    unsigned int iWindowNumber; // V1720 only
    unsigned int iChannelSampleCounter;
    unsigned int iChannelWordsRead;
    unsigned int iControlFlag;
    unsigned int iControlSampleWords;
    //**************************************

    unsigned int iOnChannels[m_pConfig->m_iNChannels];

    unsigned int iChannelSize;
    unsigned long int iTTSLSBs, iTTSMSBs; //Trigger time stamp (LSB, MSB, and combined)
    long int iEventTriggerTime;

    int iSamp0, iSamp1;
    unsigned int iHitCount = 0;
    unsigned int iWfCounter = 0;
    // std::ofstream outFile(m_sOutputFile.c_str(), std::ios::out | std::ios::binary);

    int iMaxSize = (int) (m_iNumWords/(10 * m_pConfig->m_iRecordLength/2));
    Hits.channels.reserve(iMaxSize); //Set the channel number
    Hits.triggerTimes.reserve(iMaxSize); //Set the trigger time
    Hits.startTimes.reserve(iMaxSize);
    Hits.endTimes.reserve(iMaxSize);
    Hits.baselines.reserve(iMaxSize);
    int iMaxWfSamples;

    if (m_pConfig->m_sBoardType == "V1725"){
        m_iWindowNumber = 0;

        for (int wc = 0; wc< m_iNumWords;) {
            iEventSize = m_iData[wc] & ((1 << 28) - 1);
            iChannelMaskO1 = m_iData[wc + 1] & ((1 << 8) - 1);
            iChannelMaskO2 = (m_iData[wc + 2] & (((1 << 8) - 1)<<24))>>24;
            iChannelMask = (iChannelMaskO2 << 8) + iChannelMaskO1;

            iNumOnChannels = 0;
            for (int ch = 0; ch < m_pConfig->m_iNChannels; ch++) {
                if (iChannelMask & (1 << ch)) {
                    iOnChannels[iNumOnChannels] = ch;
                    iNumOnChannels++;
                }
            }
            
            wc += 4; //Start reading the channel headers

            for (int chn = 0; chn < iNumOnChannels; chn++) {
                m_iChannel = iOnChannels[chn];
                m_iSaturatedSamples = 0;

                iChannelSize = m_iData[wc] & ((1 << 23) - 1);
                iTTSLSBs = m_iData[wc + 1];
                iTTSMSBs = m_iData[wc + 2] & ((1 << 16) - 1);
                m_iTriggerTime = 4 * ((int64_t)((iTTSMSBs << 32) + iTTSLSBs)); //ns
                m_iStartTime = m_iTriggerTime - m_pConfig->m_iPreTrigger * 4;
                m_iBaseline = (uint16_t)((m_iData[wc + 2] & (((1 << 16) - 1) << 16)) >> 16);

                wc += 3;
                iWfCounter = 0;
                m_iWaveform.resize(2 * (iChannelSize - 3), 0);
                for (int samp = 0; samp < iChannelSize - 3; samp++) {
                    iSamp0 = m_iData[wc] & ((1 << 16) - 1);
                    iSamp1 = (m_iData[wc] & (((1 << 16) - 1)<<16))>>16;
                    m_iWaveform[iWfCounter] = m_iBaseline - iSamp0;
                    m_iWaveform[iWfCounter+1] = m_iBaseline - iSamp1;
                    
                    if (iSamp0==0) m_iSaturatedSamples++;
                    if (iSamp1==0) m_iSaturatedSamples++; 

                    iWfCounter+=2;
                    wc += 1;
                }
                // m_iEndTime = m_iStartTime + m_iWaveform.size() * 4;
                m_iEndTime = m_iStartTime + 2 * (iChannelSize - 3) * 4;

                Hits.channels.emplace_back(m_iChannel); //Set the channel number
                Hits.triggerTimes.emplace_back(m_iTriggerTime); //Set the trigger time
                Hits.startTimes.emplace_back(m_iStartTime);
                Hits.endTimes.emplace_back(m_iEndTime);
                Hits.baselines.emplace_back(m_iBaseline);
                Hits.saturatedSamples.emplace_back(m_iSaturatedSamples);
                Hits.waveforms.emplace_back(m_iWaveform);
                Hits.windowNumber.emplace_back(m_iWindowNumber);
                ++iHitCount;
                if (bSave)
                    m_tHitsTree->Fill();
        
                // m_iWaveform.clear();
            }
        }
    }
    else if (m_pConfig->m_sBoardType == "V1720") {
        for (int wc = 0; wc< m_iNumWords;) {
            //Read in the event header
            iEventSize = m_iData[wc]&0xFFFFFFF;
            iChannelMask = m_iData[wc+1]&0xFF;

            iTTSMSBs = (m_iData[wc + 1]&0xFFFF00)>>8;
            m_iWindowNumber = m_iData[wc + 2]&0xFFFFFF;
            iTTSLSBs = m_iData[wc+3];
            iEventTriggerTime = ((int64_t)((iTTSMSBs << 32) + iTTSLSBs)); //8ns increment
            wc +=4;

            //See which channels are on
            iNumOnChannels = 0;
            for (int ch = 0; ch < m_pConfig->m_iNChannels; ch++) {
                if (iChannelMask & (1 << ch)) {
                    iOnChannels[iNumOnChannels] = ch;
                    iNumOnChannels++;
                }
            }

            //Read in the channels
            for (int chn = 0; chn < iNumOnChannels; chn++){
                m_iChannel = iOnChannels[chn];
                iChannelSampleCounter = 0;
                iChannelWordsRead = 0;

                iChannelSize = m_iData[wc]; //iChannelSize is used a bit differently, it's the number of words in the event!
                wc++;
                iChannelWordsRead++;

                //Keep reading until the channel size is all used up
                while (iChannelWordsRead < iChannelSize){
                    iControlFlag = (m_iData[wc]&(1<<31))>>31;
                    iControlSampleWords = m_iData[wc]&((1<<20)-1);
                    wc++;
                    iChannelWordsRead++;

                    if (iControlFlag == 1){
                        iWfCounter = 0;
                        m_iBaseline = m_iData[wc];
                        m_iWaveform.resize(2*iControlSampleWords, 0);
                        for (int samp = 0; samp < iControlSampleWords; samp++) {
                            iSamp0 = m_iData[wc] >> 16;
                            iSamp1 = (m_iData[wc] & 0xFFFF);
                            m_iWaveform[iWfCounter] = m_iBaseline - iSamp0;
                            m_iWaveform[iWfCounter+1] = m_iBaseline - iSamp1;
                            
                            if (iSamp0==0) m_iSaturatedSamples++;
                            if (iSamp1==0) m_iSaturatedSamples++; 

                            iWfCounter+=2;
                            wc += 1;
                        }
                        
                        //NOTE: The m_pConfig->m_iPreTrigger is different for the DAQ and ZLE
                        //For ZLE, this is the number of samples before the trigger in the event window
                        m_iStartTime = iEventTriggerTime*8 - (m_pConfig->m_iPreTrigger)*4 + iChannelSampleCounter*4;
                        m_iTriggerTime = m_iStartTime + (m_pConfig->m_iLBK)*4;
                        m_iEndTime = m_iStartTime + 2*iControlSampleWords*4;

                        Hits.windowNumber.emplace_back(m_iWindowNumber);
                        Hits.channels.emplace_back(m_iChannel); //Set the channel number
                        Hits.triggerTimes.emplace_back(m_iTriggerTime); //Set the trigger time
                        Hits.startTimes.emplace_back(m_iStartTime);
                        Hits.endTimes.emplace_back(m_iEndTime);
                        Hits.baselines.emplace_back(m_iBaseline);
                        Hits.saturatedSamples.emplace_back(m_iSaturatedSamples);
                        Hits.waveforms.emplace_back(m_iWaveform);

                        iChannelWordsRead += iControlSampleWords;
                        ++iHitCount;
                        if (bSave)
                            m_tHitsTree->Fill();
                    }

                    iChannelSampleCounter += 2*iControlSampleWords;
                }
            }
        }
    }
    else {
        std::cout << "The board must be a V1725 or a V1720!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // std::cout << "\nHit count: "<<iHitCount<<"\n";
    // std::cout << "\nAllocated size: "<<iMaxSize<<"\n";

    if (bSave) {
        m_tHitsTree->Write();
        m_tHitsFile->Close();
        // outFile.close();
    }

}