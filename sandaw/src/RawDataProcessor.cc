#include "RawDataProcessor.hh"

SandixRawDataProcessor::SandixRawDataProcessor(SandixConfiguration* pConfig) {
    m_pConfig = pConfig;
}

SandixRawDataProcessor::~SandixRawDataProcessor() {
}

void SandixRawDataProcessor::SetData(std::string& sBinFile) {
    std::ifstream inputData(sBinFile.c_str());
    inputData.seekg(0, std::ios_base::end);
    std::size_t size = inputData.tellg();
    inputData.seekg(0, std::ios_base::beg);

    m_iData.resize(size / sizeof(uint32_t));
    inputData.read((char*)&m_iData[0], size);
    inputData.close();

    iNumWords = size / sizeof(uint32_t);

    std::cout << "Binary file is " << size << " bytes" << std::endl;
}

void SandixRawDataProcessor::SetRootFile() {
    m_tHitsFile = new TFile(m_sOutputFile.c_str(), "RECREATE");
    m_tHitsTree = new TTree("hits", "Raw per-channel hits");

    m_tHitsTree->Branch("channel", &m_iChannel, "channel/s");
    m_tHitsTree->Branch("trigger_time", &m_iTriggerTime, "trigger_time/l");
    m_tHitsTree->Branch("start_time", &m_iStartTime, "start_time/l");
    m_tHitsTree->Branch("end_time", &m_iEndTime, "end_time/l");
    m_tHitsTree->Branch("baseline", &m_iBaseline, "baseline/s");
    m_tHitsTree->Branch("waveform", "vector<int>", &m_iWaveform);
}

void SandixRawDataProcessor::ProcessRawData(std::string& sBinFile,
    bool bSave = false) {

    SetData(sBinFile);

    if (bSave)
        SetRootFile();

    int wc = 0; //Word counter
    bool bEventHeader = true; //flag for reading the event header
    bool bChannelHeader = false; //flag for reading the channel header
    bool bEventBody = false; //flag for reading the event body

    unsigned int iEventSize;
    unsigned int iChannelMaskO1, iChannelMaskO2; //Channel mask for octants 1 and 2
    unsigned int iChannelMask; //Channel mask combined
    unsigned int iNumOnChannels;

    unsigned int iOnChannels[m_pConfig->m_iNChannels];

    unsigned int iChannelSize;
    unsigned long int iTTSLSBs, iTTSMSBs; //Trigger time stamp (LSB, MSB, and combined)

    int iSamp0, iSamp1;
    unsigned int iHitCount = 0;
    std::ofstream outFile(m_sOutputFile.c_str(), std::ios::out | std::ios::binary);

    while (wc< iNumWords) {
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

            iChannelSize = m_iData[wc] & ((1 << 23) - 1);
            iTTSLSBs = m_iData[wc + 1];
            iTTSMSBs = m_iData[wc + 2] & ((1 << 16) - 1);
            m_iTriggerTime = 4 * ((int64_t)((iTTSMSBs << 32) + iTTSLSBs)); //ns
            m_iStartTime = m_iTriggerTime - m_pConfig->m_iPreTrigger * 4;
            m_iBaseline = (uint16_t)((m_iData[wc + 2] & (((1 << 16) - 1) << 16)) >> 16);

            wc += 3;
            
            for (int samp = 0; samp < iChannelSize - 3; samp++) {
                iSamp0 = m_iData[wc] & ((1 << 16) - 1);
                iSamp1 = (m_iData[wc] & (((1 << 16) - 1)<<16))>>16;
                m_iWaveform.push_back(m_iBaseline - iSamp0);
                m_iWaveform.push_back(m_iBaseline - iSamp1);//Set the waveform
                wc += 1;
            }

            m_iEndTime = m_iStartTime + m_iWaveform.size() * 4;

            Hits.channels.push_back(m_iChannel); //Set the channel number
            Hits.triggerTimes.push_back(m_iTriggerTime); //Set the trigger time
            Hits.startTimes.push_back(m_iStartTime);
            Hits.endTimes.push_back(m_iEndTime);
            Hits.baselines.push_back(m_iBaseline);
            Hits.waveforms.push_back(m_iWaveform);

            if (bSave)
                m_tHitsTree->Fill();
    
            m_iWaveform.clear();
        }
    }

    if (bSave) {
        m_tHitsTree->Write();
        m_tHitsFile->Close();
        outFile.close();
    }

}