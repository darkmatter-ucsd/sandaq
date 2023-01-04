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

	//Hit processing settings (there typically aren't any with DAW)

	//Peak processing settings
	std::vector<double> m_dPMTGains;
	unsigned int m_iSmoothWind;
	unsigned int m_iMergingThresh;
	unsigned int m_iMaxSamples; //Size of the waveform
	

};

#endif