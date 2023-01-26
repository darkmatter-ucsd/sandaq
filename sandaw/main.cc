#include <iostream>
#include <getopt.h>
#include <string>
#include <numeric>

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TTree.h>
#include <TBranch.h>
#include <TArray.h>
#include <TFile.h>

#include "RawDataProcessor.hh"
#include "PeaksProcessor.hh"
#include "EventsProcessor.hh"
#include "Timer.hh"

template<typename T>
std::vector<size_t> argsort(const std::vector<T>& array) {
	std::vector<size_t> indices(array.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(),
		[&array](int left, int right) -> bool {
		// sort indices according to corresponding array element
		return array[left] < array[right];
	});

	return indices;
}

int main(int argc, char* argv[]) {
	int c = 0;
	bool bConfigFileExists = false;
	bool bBinfileExists = false;
	bool bOutfileExists = false;
	bool bRunIDLoaded = false;
	bool bSaveHits = false; //Don't usually do this. It's slow and mainly for debugging.
	bool bDebug = false;

	std::string sConfigFile;
	std::string sBinFile; //Binary input file
	std::string sOutFile;
	std::string sHitsFile;
	std::string sRunID;
	std::string sPeaksPath;

	while ((c = getopt(argc, argv, "c:f:p:r:h:v:")) != -1)
	{
		switch (c)
		{
		case 'c':
			bConfigFileExists = true;
			sConfigFile.assign(optarg);
			std::cout << "Configuration file: " << sConfigFile << '\n';
			break;

		case 'f':
			bBinfileExists = true;
			sBinFile.assign(optarg);
			std::cout << "Binary file: " << sBinFile << '\n';
			break;

		case 'p':
			bOutfileExists = true;
			sOutFile.assign(optarg);
			std::cout << "Output file directory: " << sOutFile << '\n';
			break;

		case 'r':
			bRunIDLoaded = true;
			sRunID.assign(optarg);
			std::cout << "Run ID: " << sRunID << '\n';
			break;

		case 'h':
			bSaveHits = true;
			sHitsFile.assign(optarg);
			std::cout << "Hits file: " << sHitsFile << '\n';
			break;

		case 'v':
			bDebug = true;
			break;

		default:
			break;
		}
	}

	if (!bBinfileExists) {
		std::cout << "ERROR: No input file" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!bConfigFileExists) {
		std::cout << "ERROR: No configuration file, cannot process data" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!((bOutfileExists & bRunIDLoaded) | ((!bOutfileExists) & (!bRunIDLoaded)))) {
		std::cout << "ERROR: Either enter both the output file AND the run-ID, OR enter neither" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (bOutfileExists & bRunIDLoaded) {
		sPeaksPath = sOutFile + "peaks_" + sRunID + ".bin";
	}

	SandixConfiguration* pConfig = new SandixConfiguration(sConfigFile, bDebug);
	SandixRawDataProcessor* pRawDataProcessor = new SandixRawDataProcessor(pConfig);
	SandixPeaksProcessor* pPeaksProcessor = new SandixPeaksProcessor(pConfig);
	SandixEventsProcessor* pEventsProcessor = new SandixEventsProcessor(pConfig);

	if (bSaveHits) {
		pRawDataProcessor->SetOutputFile(sHitsFile);
	}

	Peaks_t PeaksBuffer;
	Events_t EventsBuffer;
	int S2Count;

	std::cout << "PMT Gains has size of "<<pConfig->m_dPMTGains.size()<<": ";
	for (int g: pConfig->m_dPMTGains){
		std::cout<<g<<" ";
	}
	std::cout << "\n";

	std::cout<<"Hit processing took: ";
	{
		Timer timer;
		pRawDataProcessor->ProcessRawData(sBinFile, bSaveHits);
	}

	std::cout<<"Peak processing took: ";
	{
		Timer timer;
		S2Count = pPeaksProcessor->ProcessPeaks(pRawDataProcessor->Hits, PeaksBuffer, bOutfileExists, sOutFile, sRunID);
	}

	std::cout<<"Event processing took: ";
	{
		Timer timer;
		pEventsProcessor->ProcessEvents(S2Count, PeaksBuffer, EventsBuffer, bOutfileExists, sOutFile, sRunID);
	}

	delete pConfig;
	delete pRawDataProcessor;
	delete pPeaksProcessor;
	delete pEventsProcessor;
	return 0;
}