#include "Configuration.hh"

SandixConfiguration::SandixConfiguration(std::string& sConfigFile, bool bDebug = false) {
	inih::INIReader r{ sConfigFile };

    //ADC settings
    m_iNBoards = r.Get<unsigned int>("adc", "NBoards");
    m_iNChannelsPerBoard = r.Get<unsigned int>("adc", "NChannelsPerBoard");
    m_iPreTrigger = r.Get<unsigned int>("adc", "PreTrigger");
    m_iOnChannels = r.GetVector<unsigned int>("adc", "OnChannels");
    m_iRecordLength = r.Get<unsigned int>("adc", "RecordLength");
    m_iNChannels = m_iNBoards * m_iNChannelsPerBoard;
    
    //Peak processing settings
    const auto& dGains = r.GetVector<float>("peaks", "PMTGains");
    m_dPMTGains.resize(m_iNChannels);
    for (int i = 0; i < m_iOnChannels.size(); i++) {
        m_dPMTGains[m_iOnChannels[i]] =  ((float) dGains[i])/dGainConvert;
    }
    m_iSmoothWind = r.Get<unsigned int>("peaks", "SmoothWind");
    m_iMergingThresh = r.Get<unsigned int>("peaks", "MergingThresh");
    m_iMaxSamples = r.Get<unsigned int>("peaks", "MaxSamples");
    m_fAreaThresh = r.Get<unsigned int>("peaks", "AreaThresh");
    m_fRiseTimeHeightThresh = r.Get<unsigned int>("peaks", "RiseTimeHeightThresh");

    //Event processing settings
    m_iMaxDriftTime = r.Get<int>("events", "MaxDriftTime");
    m_iMaxPeaksInEvent = r.Get<int>("events", "MaxPeaksInEvent");

    /*****CHECKS*****/

    for (unsigned int ch : m_iOnChannels) {
        if (ch >= m_iNChannels) {
            std::cout << "ERROR: PMT out of range" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < m_iOnChannels.size() - 1; i++) {
        if (m_iOnChannels[i] >= m_iOnChannels[i + 1]) {
            std::cout << "ERROR: On PMTs are not in increasing order, or there are duplicates" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

SandixConfiguration::~SandixConfiguration() {}