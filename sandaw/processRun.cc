#include <iostream>
#include <getopt.h>
#include <string>
#include <numeric>
#include <thread>
#include <future>
#include <sys/stat.h>
#include <filesystem>
namespace fs = std::filesystem;
#include <chrono>
using namespace std::chrono_literals;

#include "RawDataProcessor.hh"
#include "PeaksProcessor.hh"
#include "EventsProcessor.hh"
#include "Timer.hh"

void MakeOutputDir(std::string outputDir){
    struct stat sb;
    int SUCCESS_CODE;
  
    if (stat(outputDir.c_str(), &sb) != 0){
        SUCCESS_CODE = mkdir(outputDir.c_str(),0777);
        if (!SUCCESS_CODE)
            std::cout << "Output directory created" << outputDir << "\n";
        else {
            std::cout << "Unable to create output directory\n";
            exit(EXIT_FAILURE);
        }
    }
}

void ProcessData(std::string config_file, bool debug, std::string rawdata_file,
                std::string output_dir, std::string seg, 
				int thread){

	SandixConfiguration* pConfig = new SandixConfiguration(config_file, debug);
	SandixRawDataProcessor* pRawDataProcessor = new SandixRawDataProcessor(pConfig);
	SandixPeaksProcessor* pPeaksProcessor = new SandixPeaksProcessor(pConfig);
	SandixEventsProcessor* pEventsProcessor = new SandixEventsProcessor(pConfig);

	int S2Count;

	pRawDataProcessor->ProcessRawData(rawdata_file, false);
	S2Count = pPeaksProcessor->ProcessPeaks(pRawDataProcessor->Hits, true, output_dir, seg);
	pEventsProcessor->ProcessEvents(S2Count, pPeaksProcessor->Peaks, true, output_dir, seg);

	delete pConfig;
	delete pRawDataProcessor;
	delete pPeaksProcessor;
	delete pEventsProcessor;
}

int main(int argc, char* argv[]) {

	int c = 0;
	int iNThreads = 1;
	int iMaxThreads = 8;
	bool bConfigFileExists = false;
	bool bOutfileExists = false;
    bool bInputPathExists = false;
	bool bRunIDLoaded = false;
	bool bSaveHits = false; //Don't usually do this. It's slow and mainly for debugging.
	bool bDebug = false;

	std::string sConfigFile;
	std::string sInputPath; //Path to all of the input files
	std::string sOutFile;
	std::string sHitsFile;
	std::string sRunID;

	while ((c = getopt(argc, argv, "c:d:p:r:n:h:v:")) != -1)
	{
		switch (c)
		{
		case 'c':
			bConfigFileExists = true;
			sConfigFile.assign(optarg);
			std::cout << "\nConfiguration file: " << sConfigFile << '\n';
			break;
        
        case 'd':
            bInputPathExists = true;
            sInputPath.assign(optarg);
            std::cout << "\nRawdata directory: " << sInputPath << "\n";
            break;

		case 'p':
			bOutfileExists = true;
			sOutFile.assign(optarg);
			std::cout << "\nOutput file directory: " << sOutFile << '\n';
			break;

		case 'r':
			bRunIDLoaded = true;
			sRunID.assign(optarg);
			std::cout << "\nRun ID: " << sRunID << '\n';
			break;
		
		case 'n':
			iNThreads = atoi(optarg);
			std::cout << "\nNumber of threads: " << iNThreads << '\n';
			break;

		case 'h':
			bSaveHits = true;
			sHitsFile.assign(optarg);
			std::cout << "\nHits file: " << sHitsFile << '\n';
			break;

		case 'v':
			bDebug = true;
			break;

		default:
			break;
		}
	}

	if (!bConfigFileExists) {
		std::cout << "ERROR: No configuration file, cannot process data" << std::endl;
		exit(EXIT_FAILURE);
	}

    if (!bInputPathExists) {
        std::cout << "ERROR: No input path, cannot process data without raw data" <<std::endl;
        exit(EXIT_FAILURE);
    }

	if (!(bOutfileExists && bRunIDLoaded)) {
		std::cout << "ERROR: Either enter both the output file AND the run-ID" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (iNThreads>iMaxThreads) {
		std::cout << "ERROR: Maximum number of threads allowed is " << iMaxThreads << std::endl;
		exit(EXIT_FAILURE);
	}

    //Look for the run
    std::string sRunFiles;
    if (sInputPath.back() == '/'){
        sRunFiles = sInputPath + sRunID + "/";
    }
    else {
        sRunFiles = sInputPath + "/" + sRunID + "/";
    }

    //Check if output directory exists, if not, make it
    MakeOutputDir(sOutFile);
    MakeOutputDir(sOutFile+sRunID);

    std::string sOutputRunDir = sOutFile + sRunID + "/";

    std::string sMetadata;
    std::vector<std::string> sRunSegments;
    for (const auto & entry : fs::directory_iterator(sRunFiles)){
        // std::cout << entry.path() << std::endl;
        if (((std::string) entry.path()).find("metadata") != std::string::npos) {
            sMetadata = ((std::string) entry.path());
        }
        else{
            sRunSegments.push_back(((std::string) entry.path()));
        }
    }

	//Start make the 
    std::vector<std::string> sSegmentTags;
	std::string segnum;
    for (int i = 0; i<sRunSegments.size(); i++){
		std::vector<std::string> file_substrings;
		std::istringstream f(sRunSegments[i]);
		std::string s;    
		while (getline(f, s, '_')) {
			// std::cout << s << std::endl;
			file_substrings.push_back(s);
		}
		sSegmentTags.push_back(sRunID + "_" + file_substrings[file_substrings.size()-2]);
    }

	int iNProcessingChunks = std::ceil(((float) sRunSegments.size())/iNThreads);
	int iFilesThreaded = 0;

	for (int chunk = 0; chunk < iNProcessingChunks; chunk++) {
		std::cout << "Chunk " << chunk << " processed in: ";
		{
			Timer timer;
			std::vector<std::thread> tProcessingThreads;
			std::vector<std::future<void>> fFutures;
			for (int th = 0; th < iNThreads; th++) {
				if (iFilesThreaded>=sRunSegments.size()) break;

				tProcessingThreads.push_back(std::thread(ProcessData,
					sConfigFile, bDebug, sRunSegments[iFilesThreaded],
					sOutputRunDir, sSegmentTags[iFilesThreaded], th));

				// fFutures.push_back(std::async(std::launch::async, ProcessData,
				// 	sConfigFile, bDebug, sRunSegments[iFilesThreaded],
				// 	sOutputRunDir, sSegmentTags[iFilesThreaded], th));
				std::cout << "Thread "<<th<<" is processing "<< sSegmentTags[iFilesThreaded]<<"\n";
				
				iFilesThreaded++;
			}

			for (int s = 0; s<tProcessingThreads.size(); s++){
				tProcessingThreads[s].join();
			}
		}
	}

	return 0;
}