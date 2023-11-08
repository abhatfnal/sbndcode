////////////////////////////////////////////////////////////////////////
// Class:       CRTEventDisplay
// Plugin Type: analyzer (Unknown Unknown)
// File:        CRTEventDisplay_module.cc
//
// Generated at Thu Oct  6 09:32:09 2022 by Henry Lay using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "lardata/DetectorInfoServices/DetectorClocksService.h"
#include "sbndcode/CRT/CRTUtils/CRTEventDisplayAlg.h"

class CRTEventDisplay;


class CRTEventDisplay : public art::EDAnalyzer {
public:

  struct Config {
    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
 
    fhicl::Table<sbnd::CRTEventDisplayAlg::Config> EventDisplayConfig {
      Name("EventDisplayConfig"),
	  };
    fhicl::Atom<bool> SetEventManually{ 
      Name("SetEventManually"),
      false
    };
    fhicl::Sequence<int> Run_SubRun_Event{ // (run, subrun, event)
      Name("Run_SubRun_Event"),
      {1, 1, 1}
    };
  };

  using Parameters = art::EDAnalyzer::Table<Config>;

  explicit CRTEventDisplay(Parameters const &config);

  CRTEventDisplay(CRTEventDisplay const&) = delete;
  CRTEventDisplay(CRTEventDisplay&&) = delete;
  CRTEventDisplay& operator=(CRTEventDisplay const&) = delete;
  CRTEventDisplay& operator=(CRTEventDisplay&&) = delete;

  void analyze(art::Event const& e) override;
  void beginRun(art::Run const& run) override;
  void beginSubRun(art::SubRun const& sr) override;

private:
  sbnd::CRTEventDisplayAlg fCRTEventDisplayAlg;
  bool fSetEventManually;
  std::vector<int> frun_subrun_event;
};


CRTEventDisplay::CRTEventDisplay(Parameters const& config)
  : EDAnalyzer{config}
  , fCRTEventDisplayAlg(config().EventDisplayConfig())
  , fSetEventManually(config().SetEventManually())
  , frun_subrun_event(config().Run_SubRun_Event())
{
}

void CRTEventDisplay::beginRun(art::Run const& run)
{
  if(fSetEventManually) {
    if (run.id().run() != static_cast<art::RunNumber_t>(frun_subrun_event.at(0))) return;
  }
}

void CRTEventDisplay::beginSubRun(art::SubRun const& sr) 
{
  if (fSetEventManually){
    if (sr.id().subRun() != static_cast<art::SubRunNumber_t>(frun_subrun_event.at(1))) return;
  }
}

void CRTEventDisplay::analyze(art::Event const& e)
{ 
  if(fSetEventManually){
    if (e.event() != static_cast<art::EventNumber_t>(frun_subrun_event.at(2))) return;
    else {
      std::cout<<"Run:subrun:event: "<<frun_subrun_event.at(0)<<":"<<frun_subrun_event.at(1)<<":"<<e.event()<<", "<<static_cast<art::EventNumber_t>(frun_subrun_event.at(2))<<std::endl;
    }
  }
  auto const clockData = art::ServiceHandle<detinfo::DetectorClocksService const>()->DataFor(e);
  fCRTEventDisplayAlg.Draw(clockData, e);
  
}

DEFINE_ART_MODULE(CRTEventDisplay)
