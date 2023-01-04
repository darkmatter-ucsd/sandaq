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

class SandixPeaksProcessor {
public:

	//iSUT = Samples Under Threshold, iSmoothWind = smoothing window
	SandixPeaksProcessor(SandixConfiguration* pConfig);
	~SandixPeaksProcessor();

	std::vector<Hits_t> HitsChannels; //A vector of Hits where HitsChannels[i] is the hits for the ith channel
	Peaks_t Peaks;

	void PeakWindows(std::vector<int64_t>& start, std::vector<int64_t>& end,
		std::vector<int64_t>& iWS, std::vector<int64_t>& iWE);

	double PercentileTime(std::vector<double>& dCumWaveform, double dPercentile);
	double HeightTime(std::vector<double>& dWf, double dPercentHeight);

	void Downsample(std::vector<double>& iWaveForm);

	void ProcessPeaks(Hits_t& Hits, bool bSave, std::string sOutFile);

private:
	SandixConfiguration* m_pConfig;
	double m_dArea; //Area of the waveforms

	//Area/percentile based times
	double m_d90pWidth; //5th to 95th = 90% width
	double m_dRiseTimeArea; //10th to 50th = rise time area
	double m_d50pWidth; //25th to 75th = 50% width
	double m_dCenterTime; //start_time + 50th = center time

	//Auxiliary parameters used for computing the above area based times
	double m_d5pTime;
	double m_d10pTime;
	double m_d25pTime;
	double m_d50pTime;
	double m_d75pTime;
	double m_d95pTime;

	//Height based times
	double m_dRiseTimeHeight; //10th to 90th = rise time height
	long int m_dMaxTime; //max = max time

	//Auxiliary parameters used for computing the above height based times
	double m_d10hTime;
	double m_d90hTime;

	std::vector<double> m_iCombinedWaveform;
	std::vector<double> m_iDownsampledWaveform;
	uint16_t m_iCoincidence;

	std::string m_sDefaultSavePath = "../../peaks_default.bin";
};

#endif