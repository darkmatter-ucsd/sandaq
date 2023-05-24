#ifndef CONFIGURATION_HH
#define CONFIGURATION_HH

#include <iostream>
#include <vector>
#include "Ini.hh"

class SandixConfiguration {
public:
	SandixConfiguration(std::string& sConfigFile, std::string& sMetaData, bool bDebug);
	~SandixConfiguration();

	std::string sConfigFile;

	//ADC Settings
	std::string m_sBoardType;
	unsigned int m_iNBoards;
	unsigned int m_iNChannelsPerBoard;
	unsigned int m_iPreTrigger;
	std::vector<unsigned int> m_iOnChannels;
	unsigned int m_iNChannels;
	unsigned int m_iRecordLength;
	unsigned int m_iLFW;
	unsigned int m_iLBK;

	//Hit processing settings (there typically aren't any with DAW)
	int64_t m_iRunStart;

	//Peak processing settings
	std::vector<float> m_dPMTGains;
	unsigned int m_iSmoothWind;
	unsigned int m_iMergingThresh;
	unsigned int m_iMaxSamples; //Size of the waveform
	unsigned int m_iMaxMergingSamples;
	size_t m_iMBPerFile;
	unsigned int m_iCoincidenceThresh; //The PMT coincidence needed to start classifying peaks into S1 or S2
	float m_fS2AreaThresh; //The minimum area of an S2
	float m_fAreaThresh; //Threshold of the area to start classifying peaks into S1s or S2s
	float m_fRiseTimeHeightThresh; //Decision boundary to classify peaks as either S1s or S2s

	//Event processing settings
	int m_iMaxDriftTime;
	unsigned int m_iMaxPeaksInEvent;
	float m_fLoneS1AreaThresh; //The minimum area to count a lone S1

private:
	const float dGainConvert14Bit = 6095.248943620215;
	const float dGainConvert12Bit = 4.*6095.248943620215;
	

};

#endif
