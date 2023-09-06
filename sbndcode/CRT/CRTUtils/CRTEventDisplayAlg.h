#ifndef CRTEVENTDISPLAYALG_H_SEEN
#define CRTEVENTDISPLAYALG_H_SEEN


///////////////////////////////////////////////
// CRTEventDisplayAlg.h
//
// Quick and dirty event display for SBND CRT
// T Brooks (tbrooks@fnal.gov), November 2018
///////////////////////////////////////////////

// framework
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h" 
#include "art/Framework/Principal/Handle.h" 
#include "canvas/Persistency/Common/Ptr.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "canvas/Persistency/Common/FindManyP.h"

// LArSoft
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Hit.h"
//#include "lardataobj/Simulation/AuxDetHit.h"
#include "larcore/CoreUtils/ServiceUtil.h" // lar::providerFrom()
namespace detinfo { class DetectorClocksData; }

// Utility libraries
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/Atom.h"

#include "sbnobj/SBND/CRT/FEBData.hh"
#include "sbnobj/Common/CRT/CRTHit.hh"
#include "sbnobj/Common/CRT/CRTTrack.hh"
#include "sbndcode/CRT/CRTUtils/CRTBackTracker.h"
#include "sbndcode/Geometry/GeometryWrappers/CRTGeoAlg.h"
#include "sbndcode/RecoUtils/RecoUtils.h"

// c++
#include <vector>

// ROOT
#include "TVector3.h"
#include "TPolyMarker3D.h"
#include "TPolyLine3D.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TLine.h"
#include "TMarker.h"
#include "TBox.h"
#include "TPad.h"


namespace sbnd{

  class CRTEventDisplayAlg {
  public:

    struct Config {
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;

      fhicl::Atom<art::InputTag> SimLabel {
        Name("SimLabel")
      };
      fhicl::Atom<art::InputTag> FEBDataLabel {
        Name("FEBDataLabel")
      };
      fhicl::Atom<art::InputTag> SimDepositsLabel {
        Name("SimDepositsLabel")
      };
      fhicl::Atom<art::InputTag> CRTHitLabel {
        Name("CRTHitLabel")
      };
      fhicl::Atom<art::InputTag> CRTTrackLabel {
        Name("CRTTrackLabel")
      };
      fhicl::Atom<double> ClockSpeedCRT {
        Name("ClockSpeedCRT")
      };

      fhicl::Atom<bool> DrawTaggers {
        Name("DrawTaggers")
      };
      fhicl::Atom<bool> DrawModules {
        Name("DrawModules")
      };
      fhicl::Atom<bool> DrawFEBs {
        Name("DrawFEBs")
      };
      fhicl::Atom<bool> DrawStrips {
        Name("DrawStrips")
      };
      fhicl::Atom<bool> DrawFEBData {
        Name("DrawFEBData")
      };
      fhicl::Atom<bool> DrawSimDeposits {
        Name("DrawSimDeposits")
      };
      fhicl::Atom<bool> DrawCrtHits {
        Name("DrawCrtHits")
      };
      fhicl::Atom<bool> DrawCrtTracks {
        Name("DrawCrtTracks")
      };
      fhicl::Atom<bool> DrawIncompleteTracks {
        Name("DrawIncompleteTracks")
      };
      fhicl::Atom<bool> DrawTrueTracks {
        Name("DrawTrueTracks")
      };

      fhicl::Atom<int> TaggerColour {
        Name("TaggerColour")
      };
      fhicl::Atom<int> FEBEndColour {
        Name("FEBEndColour")
      };
      fhicl::Atom<int> StripColour {
        Name("StripColour")
      };
      fhicl::Atom<int> FEBDataColour {
        Name("FEBDataColour")
      };
      fhicl::Atom<int> SimDepositsColour {
        Name("SimDepositsColour")
      };
      fhicl::Atom<int> CrtHitColour {
        Name("CrtHitColour")
      };
      fhicl::Atom<int> CrtTrackColour {
        Name("CrtTrackColour")
      };
      fhicl::Atom<int> TrueTrackColour {
        Name("TrueTrackColour")
      };

      fhicl::Atom<bool> UseTrueID {
        Name("UseTrueID")
      };
      fhicl::Atom<int> TrueID {
        Name("TrueID")
      };

      fhicl::Atom<bool> Print {
        Name("Print")
      };

      fhicl::Atom<double> LineWidth {
        Name("LineWidth")
      };
      fhicl::Atom<double> IncompleteTrackLength {
        Name("IncompleteTrackLength")
      };
      fhicl::Atom<double> MinTime {
        Name("MinTime")
      };
      fhicl::Atom<double> MaxTime {
        Name("MaxTime")
      };

      fhicl::Table<CRTBackTracker::Config> CrtBackTrack {
        Name("CrtBackTrack"),
      };

    };

    CRTEventDisplayAlg(const Config& config);

    CRTEventDisplayAlg(const fhicl::ParameterSet& pset) :
      CRTEventDisplayAlg(fhicl::Table<Config>(pset, {})()) {}

    CRTEventDisplayAlg();

    ~CRTEventDisplayAlg();

    void reconfigure(const Config& config);

    void SetDrawTaggers(bool tf);
    void SetDrawFEBs(bool tf);
    void SetDrawStrips(bool tf);
    void SetDrawFEBData(bool tf);
    void SetDrawSimDeposits(bool tf);
    void SetDrawCrtHits(bool tf);
    void SetDrawCrtTracks(bool tf);
    void SetDrawTrueTracks(bool tf);
    void SetPrint(bool tf);

    void SetTrueId(int id);

    bool IsVisible(const simb::MCParticle& particle);

    void DrawCube(TCanvas *c1, double *rmin, double *rmax, int colour);

    void Draw(detinfo::DetectorClocksData const& clockData, const art::Event& event);

  private:
    CRTGeoAlg fCrtGeo;

    CRTBackTracker fCrtBackTrack;

    art::InputTag fSimLabel;
    art::InputTag fFEBDataLabel;
    art::InputTag fSimDepositsLabel;
    art::InputTag fCRTHitLabel;
    art::InputTag fCRTTrackLabel;

    double fClockSpeedCRT;

    bool fDrawTaggers;
    bool fDrawModules;
    bool fDrawStrips;
    bool fDrawFEBs;
    bool fDrawFEBData;
    bool fDrawSimDeposits;
    bool fDrawCrtHits;
    bool fDrawCrtTracks;
    bool fDrawIncompleteTracks;
    bool fDrawTrueTracks;

    int fTaggerColour;
    int fStripColour;
    int fFEBEndColour;
    int fFEBDataColour;
    int fSimDepositsColour;
    int fCrtHitColour;
    int fCrtTrackColour;
    int fTrueTrackColour;

    bool fUseTrueID;
    int fTrueID;

    bool fPrint;

    double fLineWidth;
    double fIncompleteTrackLength;
    double fMinTime;
    double fMaxTime;

  };

}

#endif
