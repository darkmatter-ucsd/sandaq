#include "PeaksProcessor.hh"

SandixPeaksProcessor::SandixPeaksProcessor(SandixConfiguration* pConfig) {
	m_pConfig = pConfig;
	HitsChannels.resize(pConfig->m_iNChannels);
}

SandixPeaksProcessor::~SandixPeaksProcessor() {}

void SandixPeaksProcessor::PeakWindows(std::vector<int64_t>& start, std::vector<int64_t>& end,
	std::vector<int64_t>& iWS, std::vector<int64_t>& iWE) {

	int64_t l = start[0], r = end[0]; //current left and current right

	for (int i = 0; i < start.size()-1; i++) {
		if ((start[i + 1] - r) < m_pConfig->m_iMergingThresh) {
			if (end[i + 1] > r) {
				r = end[i + 1];
			}
		}
		else {
			iWS.push_back(l);
			iWE.push_back(r);
			l = start[i + 1];
			r = end[i + 1];
		}
	}
	
	iWS.push_back(l);
	iWE.push_back(r);

}

void SandixPeaksProcessor::Downsample(std::vector<float>& dWaveForm, unsigned int iSize, uint32_t &iDt, unsigned int &iNumBiggerSamples) {
	//The number of samples within this bigger sample
	unsigned int iBiggerSample = (unsigned int) std::ceil((float)iSize/m_pConfig->m_iMaxSamples);
	iDt = iBiggerSample * 4; //ns
	iNumBiggerSamples = (unsigned int) std::ceil((float) iSize / iBiggerSample);

	for (int samp = 0; samp<iSize; samp++){
		m_iDownsampledWaveform[samp/iBiggerSample]+=dWaveForm[samp];
	}
}

float SandixPeaksProcessor::PercentileTime(std::vector<float>& dCWf, float dPercentile, int iSize) {
	/*
	:param: dCWf: The cumulative waveform
	:param: dPercentile: The percentile for which to see when the waveform reaches
	*/

	// float dTotalArea = dCWf[dCWf.size() - 1];
	float dTotalArea = dCWf[iSize - 1];
	int s = 0;
	while (dCWf[s] / dTotalArea <= dPercentile) {
		s++;
	}
	s--;

	//Interpolate over the boundary
	return 4 * (s + (dPercentile - dCWf[s] / dTotalArea) / ((dCWf[s + 1] - dCWf[s]) / dTotalArea));
}

template <typename T, typename A>
int arg_max(std::vector<T, A> const& vec, int iSize) {
	return static_cast<int>(std::distance(vec.begin(), max_element(vec.begin(), vec.begin()+iSize)));
}

template <typename T, typename A>
int arg_min(std::vector<T, A> const& vec, int iSize) {
	return static_cast<int>(std::distance(vec.begin(), min_element(vec.begin(), vec.begin()+iSize)));
}


float SandixPeaksProcessor::HeightTime(std::vector<float>& dWf, float dPercentHeight, int iSize) {
	float dMaxHeight = *std::max_element(dWf.begin(), dWf.begin()+iSize);
	int s = 0;
	while (dWf[s] / dMaxHeight <= dPercentHeight) {
		s++;
	}
	s--;

	//Interpolate over the boundary
	return 4 * (s + (dPercentHeight - dWf[s] / dMaxHeight) / ((dWf[s + 1] - dWf[s]) / dMaxHeight));
}


int SandixPeaksProcessor::ProcessPeaks(Hits_t& Hits, Peaks_t& Peaks, bool bSave = false,
 std::string sOutFile = "", std::string sRunID = "") {
	
	//Setup the output file saving
	std::string sOutFilePeaks, sOutFileWaveforms;

	if (sOutFile == "" || sRunID =="") {
		std::cout << "ERROR: Please provide BOTH a Run ID AND an output file" << std::endl;
		exit(EXIT_FAILURE);
	}

	sOutFilePeaks = sOutFile + "peaks_" + sRunID + ".bin";
	sOutFileWaveforms = sOutFile + "waveforms_" + sRunID + ".bin";
	std::ofstream OutFileWaveforms(sOutFileWaveforms, std::ios::out | std::ios::binary);
	
	//Find the windows/timing based information
	unsigned int iNumHits = Hits.channels.size();
	
	std::vector<int64_t> iStartTimes = Hits.startTimes, iEndTimes = Hits.endTimes;
	std::vector<int64_t> iTemp;
	std::vector<int64_t> iWindowStarts, iWindowEnds;
	std::vector<unsigned long int> iIndices;
	
	std::sort(iStartTimes.begin(), iStartTimes.end());
	std::sort(iEndTimes.begin(), iEndTimes.end());

	PeakWindows(iStartTimes, iEndTimes, iWindowStarts, iWindowEnds);

	unsigned int iNumPeaks = iWindowStarts.size();
	Peaks.startTimes = iWindowStarts;
	Peaks.endTimes = iWindowEnds;

	//Separate the hits into hits per channel
	for (int i = 0; i < iNumHits; i++) {
		HitsChannels[Hits.channels[i]].channels.push_back(Hits.channels[i]);
		HitsChannels[Hits.channels[i]].triggerTimes.push_back(Hits.triggerTimes[i]);
		HitsChannels[Hits.channels[i]].startTimes.push_back(Hits.startTimes[i]);
		HitsChannels[Hits.channels[i]].endTimes.push_back(Hits.endTimes[i]);
		HitsChannels[Hits.channels[i]].baselines.push_back(Hits.baselines[i]);
		HitsChannels[Hits.channels[i]].waveforms.push_back(Hits.waveforms[i]);
	}

	//
	int iCoincidence = 0;
	int iChannelCount = 0;
	int iS2Count = 0;
	int iSampleShift;
	int64_t iCurrentStart;
	int64_t iCurrentEnd;
	int iWindowSize;
	uint32_t iCurrentDt, iNDownsamples;

	float dCumSum = 0;
	
	bool bInWindow = false;
	
	std::vector<int> iChannelHitIndex(m_pConfig->m_iNChannels, 0);
	std::vector<float> dMergedWaveform;
	std::vector<float> dCumWaveform;
	float dTempArea;
	float dChannelArea;
	// std::vector<float> dAreaPerChannel(m_pConfig->m_iNChannels, 0.);

	int iMaxWfSamples = 20000;
	unsigned int iDownsampleSamples;
	dMergedWaveform.resize(iMaxWfSamples, 0.);
	dCumWaveform.resize(iMaxWfSamples, 0.);
	m_iDownsampledWaveform.resize(m_pConfig->m_iMaxSamples, 0.);

	Peaks.startTimes.reserve(iWindowStarts.size());
	Peaks.endTimes.reserve(iWindowStarts.size());
	Peaks.coincidences.reserve(iWindowStarts.size());
	Peaks.types.reserve(iWindowStarts.size());
	Peaks.areas.reserve(iWindowStarts.size());
	Peaks.centerTimes.reserve(iWindowStarts.size());
	Peaks.maxTimes.reserve(iWindowStarts.size());
	Peaks.range50pArea.reserve(iWindowStarts.size());
	Peaks.range90pArea.reserve(iWindowStarts.size());
	Peaks.riseTimeHeight.reserve(iWindowStarts.size());
	Peaks.riseTimeArea.reserve(iWindowStarts.size());
	Peaks.wfSamples.reserve(iWindowStarts.size());
	Peaks.dt.reserve(iWindowStarts.size());
	Peaks.areaPerChannel.reserve(iWindowStarts.size() * m_pConfig->m_iOnChannels.size());

	//Downsampling numbers
	unsigned int iBiggerSample, iDt, iNumBiggerSamples;

	//Window loop, this determines the number of hits
	for (int w = 0; w < iWindowStarts.size(); w++) {

		iWindowSize = (iWindowEnds[w] - iWindowStarts[w]) / 4 ;//+ 1;
		// dMergedWaveform.resize(iWindowSize, 0.);
		std::fill(dMergedWaveform.begin(), dMergedWaveform.begin() + iWindowSize, 0.);
		std::fill(dCumWaveform.begin(), dCumWaveform.begin() + iWindowSize, 0.);
		std::fill(m_iDownsampledWaveform.begin(), m_iDownsampledWaveform.begin() + m_pConfig->m_iMaxSamples, 0.);
		// std::fill(dAreaPerChannel.begin(), dAreaPerChannel.end(), 0.);

		//Channel loop
		for (unsigned int ch : m_pConfig->m_iOnChannels) {
			//The start time of the current hit
			iCurrentStart = HitsChannels[ch].startTimes[iChannelHitIndex[ch]];
			iCurrentEnd = HitsChannels[ch].endTimes[iChannelHitIndex[ch]];
			bInWindow = (iWindowStarts[w] <= iCurrentStart) && (iCurrentEnd <= iWindowEnds[w]);

			dChannelArea = 0.;
			//Hit loop per channel
			while (iCurrentStart < iWindowEnds[w]) {
				if (bInWindow) {
					//Add the channel waveforms together for a combined waveform
					iSampleShift = ((iCurrentStart - iWindowStarts[w]) / 4);
					for (int s = 0; s < HitsChannels[ch].waveforms[iChannelHitIndex[ch]].size(); s++) {
						dTempArea = (float) HitsChannels[ch].waveforms[iChannelHitIndex[ch]][s]/m_pConfig->m_dPMTGains[ch];
						dMergedWaveform[iSampleShift + s] += dTempArea;
						dChannelArea += dTempArea;
						// dAreaPerChannel[ch] += dTempArea;
					}
					iChannelCount++;
				}

				iChannelHitIndex[ch]++;
				iCurrentStart = HitsChannels[ch].startTimes[iChannelHitIndex[ch]];
				iCurrentEnd = HitsChannels[ch].endTimes[iChannelHitIndex[ch]];
				bInWindow = (iWindowStarts[w] <= iCurrentStart) && (iCurrentEnd <= iWindowEnds[w]);
			}//End hit loop

			Peaks.areaPerChannel.push_back(dChannelArea);

			if (iChannelCount > 0) iCoincidence++;
			iChannelCount = 0;
		}//End channel loop

		//Compute the cumulative waveform for area based times (width, etc.)
		dCumSum = 0.;
		iBiggerSample = (unsigned int) std::ceil((float)iWindowSize/m_pConfig->m_iMaxSamples);
		iNumBiggerSamples = (unsigned int) std::ceil((float) iWindowSize / iBiggerSample);
		for (int samp = 0; samp < iWindowSize; samp++) {
			dCumSum += dMergedWaveform[samp];
			dCumWaveform[samp] = dCumSum;
			m_iDownsampledWaveform[samp/iBiggerSample]+=dMergedWaveform[samp];
		}

		//Auxiliary parameters for computing the area based widths and times
		m_d5pTime = PercentileTime(dCumWaveform, 0.05, iWindowSize);
		m_d10pTime = PercentileTime(dCumWaveform, 0.10, iWindowSize);
		m_d25pTime = PercentileTime(dCumWaveform, 0.25, iWindowSize);
		m_d50pTime = PercentileTime(dCumWaveform, 0.50, iWindowSize);
		m_d75pTime = PercentileTime(dCumWaveform, 0.75, iWindowSize);
		m_d95pTime = PercentileTime(dCumWaveform, 0.95, iWindowSize);

		//Area based widths and times
		m_d90pWidth = m_d95pTime - m_d5pTime;
		m_dRiseTimeArea = m_d50pTime - m_d10pTime;
		m_d50pWidth = m_d75pTime - m_d25pTime;
		m_dCenterTime = ((double) iWindowStarts[w]) + ((double) m_d50pTime);

		//Compute the height based times
		m_d10hTime = HeightTime(dMergedWaveform, 0.10, iWindowSize);
		m_d90hTime = HeightTime(dMergedWaveform, 0.90, iWindowSize);

		//Height based widths and times
		m_dRiseTimeHeight = m_d90hTime - m_d10hTime;
		m_dMaxTime = iWindowStarts[w] + arg_max(dMergedWaveform, iWindowSize) * 4;

		OutFileWaveforms.write((char*)&m_iDownsampledWaveform[0], sizeof(float) * iNumBiggerSamples);
		Peaks.dt.push_back(iBiggerSample*4);
		Peaks.wfSamples.push_back((uint32_t )iNumBiggerSamples);

		//Load everything to the Peaks member
		Peaks.coincidences.push_back(iCoincidence);
		Peaks.areas.push_back(dCumSum);
		Peaks.centerTimes.push_back(m_dCenterTime);
		Peaks.maxTimes.push_back(m_dMaxTime);
		Peaks.range50pArea.push_back(m_d50pWidth);
		Peaks.range90pArea.push_back(m_d90pWidth);
		Peaks.riseTimeHeight.push_back(m_dRiseTimeHeight);
		Peaks.riseTimeArea.push_back(m_dRiseTimeArea);

		//Peak classification
		if (dCumSum<m_pConfig->m_fAreaThresh){
			Peaks.types.push_back(0);
		}
		else{
			if (m_dRiseTimeHeight<m_pConfig->m_fRiseTimeHeightThresh)
				Peaks.types.push_back(1);
			else{
				Peaks.types.push_back(2);
				iS2Count++;
			}
		}

		iCoincidence = 0;
	}

	//Peak classification
	// for (int w = 0; w < iWindowStarts.size(); w++) {
	// 	if (Peaks.areas[w]<m_pConfig->m_fAreaThresh){
	// 		Peaks.types.push_back(0);
	// 	}
	// 	else{
	// 		if (Peaks.riseTimeHeight[w]<m_pConfig->m_fRiseTimeHeightThresh)
	// 			Peaks.types.push_back(1);
	// 		else
	// 			Peaks.types.push_back(2);
	// 	}
	// }


	if (bSave){
		std::ofstream outFilePeaks(sOutFilePeaks, std::ios::out | std::ios::binary);

		outFilePeaks.write((char*)&iNumPeaks, sizeof(unsigned int));
		outFilePeaks.write((char*)&Peaks.startTimes[0], sizeof(int64_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.endTimes[0], sizeof(int64_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.coincidences[0], sizeof(uint16_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.types[0], sizeof(uint8_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.areas[0], sizeof(float) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.centerTimes[0], sizeof(double) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.maxTimes[0], sizeof(int64_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.range50pArea[0], sizeof(float) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.range90pArea[0], sizeof(float) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.riseTimeHeight[0], sizeof(float) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.riseTimeArea[0], sizeof(float) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.wfSamples[0], sizeof(uint32_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.dt[0], sizeof(uint32_t) * iNumPeaks);
		outFilePeaks.write((char*)&Peaks.areaPerChannel[0], sizeof(float) * m_pConfig->m_iNChannels * iNumPeaks);

		outFilePeaks.close();
	}

	return iS2Count;
}