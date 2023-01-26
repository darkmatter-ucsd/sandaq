#ifndef CONFIGURATION_HH
#define CONFIGURATION_HH

#include <iostream>
#include <vector>
#include "Ini.hh"

class SandixConfiguration {
public:
	SandixConfiguration(std::string& sConfigFile, bool bDebug);
	~SandixConfiguration();

	std::string sConfigFile;

	//ADC Settings
	unsigned int m_iNBoards;
	unsigned int m_iNChannelsPerBoard;
	unsigned int m_iPreTrigger;
	std::vector<unsigned int> m_iOnChannels;
	unsigned int m_iNChannels;
	unsigned int m_iRecordLength;

	//Hit processing settings (there typically aren't any with DAW)

	//Peak processing settings
	std::vector<float> m_dPMTGains;
	unsigned int m_iSmoothWind;
	unsigned int m_iMergingThresh;
	unsigned int m_iMaxSamples; //Size of the waveform
	float m_fAreaThresh; //Threshold of the area to start classifying peaks into S1s or S2s
	float m_fRiseTimeHeightThresh; //Decision boundary to classify peaks as either S1s or S2s

	//Event processing settings
	int m_iMaxDriftTime;
	unsigned int m_iMaxPeaksInEvent;

private:
	const float dGainConvert = 6095.248943620215;
	

};

#endif