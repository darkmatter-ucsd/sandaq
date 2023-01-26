#ifndef SANDAW_RAWDATA_HH
#define SANDAW_RAWDATA_HH

//Internal
#include "Types.hh"
#include "Configuration.hh"

//ROOT
#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TTree.h>
#include <TBranch.h>
#include <TArray.h>
#include <TFile.h>

//C++ packages
#include <fstream>

class SandixRawDataProcessor {
public:
    SandixRawDataProcessor(SandixConfiguration* pConfig);
    ~SandixRawDataProcessor();

    void SetRootFile();
    void SetData(std::string& sBinFile);
    void SetOutputFile(std::string& sOutputFile) { m_sOutputFile = sOutputFile; };

    void ProcessRawData(std::string& sBinFile, bool bSave);

    Hits_t Hits;
    

private:
    SandixConfiguration* m_pConfig;

    unsigned int iNumWords;

    std::vector<uint32_t> m_iData;

    TFile* m_tHitsFile;
    TTree* m_tHitsTree;

    std::string m_sOutputFile = "output.root";

    uint16_t m_iChannel;
    int64_t m_iTriggerTime;
    int64_t m_iStartTime;
    int64_t m_iEndTime;
    uint16_t m_iBaseline;
    std::vector<int> m_iWaveform;
};

#endif