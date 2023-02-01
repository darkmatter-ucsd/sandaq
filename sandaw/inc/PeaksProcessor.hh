#ifndef SANDAW_PEAKS_HH
#define SANDAW_PEAKS_HH

//Internal
#include "Types.hh"
#include "Configuration.hh"

//Standard C++ Library
#include <numeric>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>

class SandixPeaksProcessor {
public:

	//iSUT = Samples Under Threshold, iSmoothWind = smoothing window
	SandixPeaksProcessor(SandixConfiguration* pConfig);
	~SandixPeaksProcessor();

	std::vector<Hits_t> HitsChannels; //A vector of Hits where HitsChannels[i] is the hits for the ith channel
	Peaks_t Peaks;

	void PeakWindows(std::vector<int64_t>& start, std::vector<int64_t>& end,
		std::vector<int64_t>& iWS, std::vector<int64_t>& iWE);

	float PercentileTime(std::vector<float>& dCumWaveform, float dPercentile, int iSize);
	float HeightTime(std::vector<float>& dWf, float dPercentHeight, int iSize);

	void Downsample(std::vector<float>& dWaveForm, unsigned int iSize, uint32_t &iDt, unsigned int &iNumBiggerSamples);

	int ProcessPeaks(Hits_t& Hits, bool bSave, std::string sOutFile, std::string sRunID);

private:
	SandixConfiguration* m_pConfig;
	float m_dArea; //Area of the waveforms

	//Area/percentile based times
	float m_d90pWidth; //5th to 95th = 90% width
	float m_dRiseTimeArea; //10th to 50th = rise time area
	float m_d50pWidth; //25th to 75th = 50% width
	double m_dCenterTime; //start_time + 50th = center time

	//Auxiliary parameters used for computing the above area based times
	float m_d5pTime;
	float m_d10pTime;
	float m_d25pTime;
	float m_d50pTime;
	float m_d75pTime;
	float m_d95pTime;

	//Height based times
	float m_dRiseTimeHeight; //10th to 90th = rise time height
	int64_t m_dMaxTime; //max = max time

	//Auxiliary parameters used for computing the above height based times
	float m_d10hTime;
	float m_d90hTime;

	std::vector<float> m_iCombinedWaveform;
	std::vector<float> m_iDownsampledWaveform;
	uint16_t m_iCoincidence;

	std::string m_sDefaultSavePath = "../../peaks_default.bin";
};

#endif