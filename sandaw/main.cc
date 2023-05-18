#include <iostream>
#include <getopt.h>
#include <string>
#include <numeric>
// #include <filesystem>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

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

int main(int argc, char* argv[]) {
	int c = 0;
	bool bConfigFileExists = false;
	bool bBinfileExists = false;
	bool bOutfileExists = false;
	bool bRunIDLoaded = false;
	bool bMetaDataLoaded = false;
	bool bSaveHits = false; //Don't usually do this. It's slow and mainly for debugging.
	bool bDebug = false;

	std::string sConfigFile;
	std::string sBinFile; //Binary input file
	std::string sOutFile;
	std::string sHitsFile = "output.root";
	std::string sMetaData;
	std::string sRunID;
	std::string sPeaksPath;

	while ((c = getopt(argc, argv, "c:f:p:r:m:h:v:")) != -1)
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

		case 'm':
			bMetaDataLoaded = true;
			sMetaData.assign(optarg);
			std::cout << "Metadata: " << sMetaData << '\n';
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

	if (!bMetaDataLoaded) {
		std::cout << "ERROR: No metadata, cannot process data" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (bOutfileExists & bRunIDLoaded) {
		sPeaksPath = sOutFile + "peaks_" + sRunID + ".bin";
	}

	SandixConfiguration* pConfig = new SandixConfiguration(sConfigFile, sMetaData, bDebug);
	SandixRawDataProcessor* pRawDataProcessor = new SandixRawDataProcessor(pConfig);
	SandixPeaksProcessor* pPeaksProcessor = new SandixPeaksProcessor(pConfig);
	SandixEventsProcessor* pEventsProcessor = new SandixEventsProcessor(pConfig);

	if (bSaveHits) {
		pRawDataProcessor->SetOutputFile(sHitsFile);
	}

	int S2Count;

	std::cout << "PMT Gains has size of "<<pConfig->m_dPMTGains.size()<<": ";
	for (int g: pConfig->m_dPMTGains){
		std::cout<<g<<" ";
	}
	std::cout << "\n";

	std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;

	std::cout<<"Hit processing took: ";
	// start_time = std::chrono::high_resolution_clock::now();
	{
		Timer timer;
		pRawDataProcessor->ProcessRawData(sBinFile, bSaveHits);
	}
	// end_time = std::chrono::high_resolution_clock::now();
	// auto start = std::chrono::time_point_cast<std::chrono::microseconds> (start_time).time_since_epoch().count();
	// auto end = std::chrono::time_point_cast<std::chrono::microseconds> (end_time).time_since_epoch().count();
	// auto duration = end - start;
	// double ms = duration*0.001;
	// std::cout << ms <<"ms\n\n";

	std::cout << "Finished processing hits\n";

	
	std::cout<<"Peak processing took: ";
	// start_time = std::chrono::high_resolution_clock::now();
	{
		Timer timer;
		S2Count = pPeaksProcessor->ProcessPeaks(pRawDataProcessor->Hits, bOutfileExists, sOutFile, sRunID);
	}
	// end_time = std::chrono::high_resolution_clock::now();
	// start = std::chrono::time_point_cast<std::chrono::microseconds> (start_time).time_since_epoch().count();
	// end = std::chrono::time_point_cast<std::chrono::microseconds> (end_time).time_since_epoch().count();
	// duration = end - start;
	// ms = duration*0.001;
	// std::cout << ms <<"ms\n\n";
	
	std::cout<<"Event processing took: ";
	{
		Timer timer;
		pEventsProcessor->ProcessEvents(S2Count, pPeaksProcessor->Peaks, bOutfileExists, sOutFile, sRunID);
	}

	delete pConfig;
	delete pRawDataProcessor;
	delete pPeaksProcessor;
	delete pEventsProcessor;
	return 0;
}