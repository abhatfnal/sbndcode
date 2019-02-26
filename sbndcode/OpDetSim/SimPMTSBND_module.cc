////////////////////////////////////////////////////////////////////////
// Class:       SimPMTSBND
// Module Type: producer
// File:        SimPMTSBND_module.cc
//
// Created by L. Paulucci and F. Marinho
// Based on OpDetDigitizerDUNE_module.cc
// Triggering based on SimPMTIcarus_module.cc
////////////////////////////////////////////////////////////////////////

#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <sstream>
#include <fstream>

#include "lardataobj/RawData/OpDetWaveform.h"
#include "lardata/DetectorInfoServices/DetectorClocksServiceStandard.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/Simulation/sim.h"
#include "lardataobj/Simulation/SimChannel.h"
#include "lardataobj/Simulation/SimPhotons.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardata/DetectorInfoServices/LArPropertiesService.h"
#include "larsim/Simulation/LArG4Parameters.h"

#include "TMath.h"
#include "TRandom3.h"
#include "TF1.h"
#include "TH1D.h"

#include "sbndPDMapAlg.h" 

namespace opdet{

  class SimPMTSBND;

  class SimPMTSBND : public art::EDProducer {
  public:
    explicit SimPMTSBND(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
    SimPMTSBND(SimPMTSBND const &) = delete;
    SimPMTSBND(SimPMTSBND &&) = delete;
    SimPMTSBND & operator = (SimPMTSBND const &) = delete;
    SimPMTSBND & operator = (SimPMTSBND &&) = delete;

  // Required functions.
    void produce(art::Event & e) override;

    opdet::sbndPDMapAlg map; // map for photon detector types

  private:

  // Declare member data here.
    std::string fInputModuleName;
    double fSampling;       //wave sampling frequency (GHz)
    double fReadoutWindow;  //waveform time interval (ns)
    unsigned int fNsamples; //Samples per waveform
    double fPreTrigger;     //(ns)
    double fQE;             //PMT quantum efficiency

  //Single PE parameters
    double fFallTime;       //fall time of 1PE in ns
    double fRiseTime;      //rise time in ns
    double fTransitTime;   //to be added to pulse minimum time in ns
    double sigma1;
    double sigma2;
    double tadd; //to add pre trigger considering there is electron transit time
    double fMeanAmplitude;  //in pC

    void AddSPE(size_t time_bin, std::vector<double>& wave); // add single pulse to auxiliary waveform
    double Pulse1PE(double time) ;

    std::vector<double> wsp; //single photon pulse vector

    int pulsesize; //size of 1PE waveform

    TH1D* timeTPB; //histogram for getting the TPB emission time for coated PMTs

    double fADC;      //charge to ADC convertion scale
    double fBaseline; //waveform baseline
    double fBaselineRMS; //amplitude of gaussian noise
    double fDarkNoiseRate; //in Hz
    double fSaturation; //equivalent to the number of p.e. that saturates the electronic signal
    int fUseLitePhotons; //1 for using SimLitePhotons and 0 for SimPhotons (more complete)
    std::unordered_map< raw::Channel_t,std::vector<double> > fFullWaveforms;  

    bool EnergyRange(int type, double energy);//Testing if the photon has the energy to be detected by coated or uncoated PMT
    void CreatePDWaveform(sim::SimPhotons const& SimPhotons, double t_min, std::vector<double>& wave, std::string pdtype); 
    void CreatePDWaveformLite(std::map< int, int > const& photonMap, double t_min, std::vector<double>& wave); 
    void CreateSaturation(std::vector<double>& wave);//Including saturation effects
    void AddLineNoise(std::vector<double>& wave); //add noise to baseline
    void AddDarkNoise(std::vector<double>& wave); //add dark noise
    double FindMinimumTime(sim::SimPhotons const&, std::string pdtype);
    double FindMinimumTimeLite(std::map< int, int > const& photonMap);  
  };

  SimPMTSBND::SimPMTSBND(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
  {
  // Call appropriate produces<>() functions here.
    produces< std::vector< raw::OpDetWaveform > >();

    fInputModuleName = p.get< std::string >("InputModule" );
    fTransitTime     = p.get< double >("TransitTime"   ); //ns
    fADC             = p.get< double >("ADC"           ); //voltage to ADC factor
    fBaseline        = p.get< double >("Baseline"      ); //in ADC
    fFallTime        = p.get< double >("FallTime"      ); //in ns
    fRiseTime        = p.get< double >("RiseTime"      ); //in ns
    fMeanAmplitude   = p.get< double >("MeanAmplitude" ); //in pC
    fBaselineRMS     = p.get< double >("BaselineRMS"   ); //in ADC
    fDarkNoiseRate   = p.get< double >("DarkNoiseRate" ); //in Hz
    fReadoutWindow   = p.get< double >("ReadoutWindow" ); //in ns
    fPreTrigger      = p.get< double >("PreTrigger"    ); //in ns
    fSaturation      = p.get< double >("Saturation"    ); //in number of p.e.
    fUseLitePhotons  = p.get< int    >("UseLitePhotons"); //whether SimPhotonsLite or SimPhotons will be used
    double temp_fQE  = p.get< double >("QE"            ); //PMT quantum efficiency

//Correction due to scalling factor applied during simulation if any
    auto const *LarProp = lar::providerFrom<detinfo::LArPropertiesService>();
//    fQE = temp_fQE/(LarProp->ScintPreScale());
      fQE = temp_fQE; //not using scintprescale yet
  
    std::cout << "PMT corrected efficiency = " << fQE << std::endl;

    if(fQE>1.0001)
	std::cout << "WARNING: Quantum efficiency set in fhicl file " << temp_fQE << " seems to be too large! Final QE must be equal or smaller than the scintillation pre scale applied at simulation time. Please check this number (ScintPreScale): " << LarProp->ScintPreScale() << std::endl;

    auto const *timeService = lar::providerFrom< detinfo::DetectorClocksService >();
    fSampling = (timeService->OpticalClock().Frequency())/1000.0; //in GHz
  
    std::cout << "Sampling = " << fSampling << " GHz." << std::endl;
  
    fNsamples = (int)((fPreTrigger+fReadoutWindow)*fSampling);
  
//Random number engine initialization
    int seed = time(NULL);
    gRandom = new TRandom3(seed);

    timeTPB = new TH1D("Time TPB", "", 1000, 0.0, 1000.0);//histogram that stores the emission time of photons converted by TPB

    double x[1000]={12321, 10239, 8303, 6975, 5684, 4667, 4031, 3446, 2791, 2485, 2062, 1724, 1419, 1367, 1111, 982, 974, 822, 732, 653, 665, 511, 500, 452, 411, 439, 409, 357, 342, 357, 302, 296, 316, 271, 286, 265, 260, 288, 279, 238, 214, 242, 232, 238, 251, 239, 200, 225, 182, 190, 206, 194, 188, 227, 210, 198, 170, 184, 158, 160, 170, 183, 168, 143, 158, 140, 167, 145, 154, 162, 155, 115, 143, 148, 124, 126, 133, 122, 91, 130, 90, 124, 135, 112, 94, 81, 107, 99, 109, 78, 83, 75, 68, 97, 69, 74, 91, 84, 84, 74, 68, 73, 71, 55, 68, 40, 55, 63, 71, 62, 63, 60, 71, 55, 62, 53, 54, 58, 63, 39, 42, 56, 44, 33, 36, 43, 60, 49, 50, 51, 52, 49, 47, 57, 39, 45, 41, 23, 41, 26, 29, 51, 23, 45, 26, 50, 39, 20, 44, 27, 14, 17, 13, 35, 20, 25, 26, 26, 29, 31, 20, 17, 28, 24, 28, 34, 22, 16, 17, 21, 23, 33, 15, 30, 8, 20, 15, 20, 14, 17, 18, 21, 16, 20, 22, 24, 14, 18, 25, 13, 10, 13, 11, 18, 9, 4, 13, 23, 10, 13, 15, 26, 21, 18, 15, 17, 6, 15, 9, 13, 14, 6, 13, 9, 9, 6, 8, 7, 13, 13, 11, 13, 8, 5, 8, 13, 7, 9, 6, 14, 11, 11, 9, 10, 13, 9, 4, 3, 17, 3, 5, 1, 5, 5, 5, 15, 4, 6, 3, 11, 3, 10, 8, 8, 7, 5, 8, 7, 13, 7, 7, 15, 5, 6, 9, 9, 7, 4, 9, 7, 5, 7, 5, 5, 6, 3, 8, 6, 4, 12, 7, 4, 4, 6, 7, 9, 3, 2, 3, 4, 4, 1, 9, 9, 2, 2, 2, 4, 3, 3, 1, 5, 1, 7, 4, 6, 4, 6, 7, 4, 4, 5, 2, 3, 2, 8, 4, 9, 4, 4, 8, 2, 2, 2, 0, 2, 14, 4, 3, 2, 3, 4, 5, 3, 7, 1, 4, 1, 1, 8, 3, 5, 2, 1, 7, 4, 5, 0, 5, 6, 4, 2, 6, 1, 4, 5, 0, 0, 4, 1, 4, 6, 2, 0, 4, 3, 4, 3, 3, 8, 4, 1, 2, 3, 2, 6, 7, 4, 2, 5, 6, 3, 2, 6, 5, 3, 1, 4, 6, 3, 0, 2, 2, 1, 0, 0, 5, 4, 3, 3, 3, 9, 0, 4, 2, 6, 0, 2, 6, 4, 6, 1, 0, 5, 3, 1, 1, 4, 0, 1, 1, 2, 2, 4, 5, 7, 5, 3, 7, 6, 3, 2, 1, 3, 0, 4, 4, 1, 2, 4, 6, 11, 7, 5, 5, 5, 4, 2, 5, 2, 2, 3, 0, 6, 3, 2, 3, 3, 8, 0, 0, 1, 2, 1, 0, 3, 6, 1, 6, 1, 4, 5, 0, 2, 6, 0, 3, 7, 0, 2, 5, 2, 6, 3, 5, 2, 2, 1, 5, 5, 0, 3, 3, 2, 3, 6, 0, 0, 1, 0, 1, 4, 4, 2, 3, 4, 3, 7, 1, 1, 3, 2, 2, 2, 2, 4, 9, 4, 8, 2, 2, 6, 5, 2, 6, 1, 2, 6, 7, 0, 5, 0, 4, 0, 1, 4, 1, 2, 2, 1, 0, 2, 4, 1, 0, 3, 3, 0, 6, 2, 0, 0, 3, 0, 2, 2, 3, 3, 2, 0, 2, 1, 3, 1, 2, 1, 1, 2, 4, 3, 0, 2, 4, 2, 3, 3, 3, 5, 5, 2, 2, 1, 0, 2, 0, 1, 0, 0, 5, 1, 3, 8, 4, 3, 2, 6, 4, 1, 3, 2, 0, 9, 4, 2, 7, 2, 0, 0, 2, 1, 3, 4, 2, 3, 3, 3, 2, 8, 6, 3, 1, 3, 3, 0, 0, 3, 0, 6, 1, 0, 2, 0, 0, 1, 2, 7, 2, 1, 0, 1, 6, 3, 2, 0, 1, 0, 2, 3, 5, 3, 6, 4, 1, 1, 0, 0, 7, 1, 1, 1, 1, 8, 1, 1, 0, 0, 0, 1, 2, 2, 7, 1, 1, 0, 1, 1, 3, 1, 1, 1, 3, 5, 2, 1, 0, 3, 2, 5, 0, 5, 4, 2, 5, 3, 3, 0, 0, 5, 0, 5, 1, 4, 0, 1, 6, 1, 6, 1, 2, 1, 2, 4, 0, 8, 3, 1, 7, 1, 2, 4, 4, 2, 3, 5, 0, 4, 5, 2, 1, 1, 5, 2, 0, 4, 2, 0, 2, 4, 4, 4, 4, 5, 0, 3, 0, 2, 3, 3, 0, 0, 6, 1, 1, 6, 10, 0, 2, 0, 1, 4, 1, 0, 1, 3, 2, 0, 1, 1, 0, 0, 8, 4, 0, 0, 3, 0, 0, 4, 0, 1, 4, 0, 1, 0, 2, 1, 5, 2, 2, 0, 0, 0, 2, 1, 0, 0, 4, 0, 3, 1, 1, 2, 1, 1, 2, 0, 1, 3, 3, 0, 2, 2, 3, 2, 0, 1, 2, 0, 0, 1, 0, 4, 2, 3, 0, 0, 1, 2, 2, 6, 2, 3, 4, 2, 3, 1, 2, 7, 3, 2, 3, 3, 1, 0, 0, 4, 2, 9, 0, 3, 2, 2, 0, 1, 1, 1, 8, 1, 4, 2, 0, 4, 0, 2, 0, 0, 0, 3, 0, 0, 4, 0, 2, 2, 5, 1, 3, 6, 0, 0, 0, 0, 0, 3, 2, 1, 0, 1, 1, 1, 1, 2, 0, 1, 4, 4, 1, 1, 0, 0, 9, 2, 1, 1, 2, 0, 2, 2, 2, 2, 0, 1, 7, 0, 7, 0, 5, 0, 5, 1, 1, 0, 1, 1, 1, 0, 1, 0, 2, 3, 2, 1, 1, 0, 0, 2, 4, 0, 0, 2, 4, 2, 5, 2, 2, 1, 1, 4, 0, 1, 2, 1, 3, 3, 0, 2, 1, 3, 0, 2, 2, 1, 0, 4, 5, 3, 0, 0, 2, 1, 3, 0, 2, 0, 3, 2, 2, 0, 3, 1, 0, 5, 2, 2, 4, 6, 3, 2, 2, 2, 1, 4, 6, 1, 2, 1, 2, 6, 1, 2};

  for(int i=1; i<1000; i++)timeTPB->SetBinContent(i,x[i]);

  //shape of single pulse
    sigma1 = fRiseTime/1.687;
    sigma2 = fFallTime/1.687;

    double ttop = 1.272*fRiseTime; //time it takes to go from 10% of MaxAmplitude to MaxAmplitude
    if(fPreTrigger<(fTransitTime-ttop)) tadd=(fTransitTime-ttop-fPreTrigger);
    else tadd=(fPreTrigger-fTransitTime+ttop);

    pulsesize=(int)((6*sigma2+fTransitTime)*fSampling);
    wsp.resize(pulsesize);

    for(int i=0; i<pulsesize; i++){
	wsp[i]=(Pulse1PE(static_cast< double >(i)/fSampling));
	//	std::cout << wsp[i] << std::endl;
    }

  }

  void SimPMTSBND::produce(art::Event & e)
  {
    std::unique_ptr< std::vector< raw::OpDetWaveform > > pulseVecPtr(std::make_unique< std::vector< raw::OpDetWaveform > > ());

  // Implementation of required member function here.
    std::cout <<"Event: " << e.id().event() << std::endl;

    int ch;
    double t_min = 1e15;

 //   art::ServiceHandle< sim::LArG4Parameters > lgp;
//    std::cout << "UseLitePhotons= " << lgp->UseLitePhotons() <<std::endl;

    if(fUseLitePhotons==1){//using SimPhotonsLite (no info on energy)
     // std::cout << "WARNING: With SimPhotonsLite, only waveforms for coated PMTs are generated. For uncoated PMTs please use SimPhotons to account for the photon energy." << std::endl;

    //  art::Handle< std::vector<sim::SimPhotonsLite> > pmtHandle;
    //  e.getByLabel(fInputModuleName, pmtHandle);

      //Get *ALL* SimPhotonsCollectionLite from Event
      std::vector< art::Handle< std::vector< sim::SimPhotonsLite > > > photon_handles;
      e.getManyByType(photon_handles);
      if (photon_handles.size() == 0)
        throw art::Exception(art::errors::ProductNotFound)<<"sim SimPhotons retrieved and you requested them.";
      
      
     // Loop over direct/reflected photons
        for (auto pmtHandle: photon_handles) {
          // Do some checking before we proceed
          if (!pmtHandle.isValid()) continue;  
          if (pmtHandle.provenance()->moduleLabel() != fInputModuleName) continue;   //not the most efficient way of doing this, but preserves the logic of the module. Andrzej
 
      //this now tells you if light collection is reflected
          bool Reflected = (pmtHandle.provenance()->productInstanceName() == "Reflected");
      
//       if(!pmtHandle.isValid()){
//         std::cout <<Form("Did not find any G4 photons from a producer: %s", "largeant") << std::endl;
//       }
    
	  if(Reflected)
	  {std::cout << "looking at reflected/visible lite photons" << std::endl; }
	  else
	  {std::cout << "looking at direct/vuv lite photons" << std::endl;  }  
	  
	  
      std::cout << "Number of photon channels: " << pmtHandle->size() << std::endl;
   // unsigned int nChannels = pmtHandle->size();
      unsigned int nChannels = 272;

      std::vector<std::vector<short unsigned int>> waveforms(nChannels,std::vector<short unsigned int> (fNsamples,0));
      std::vector<std::vector<double>> waves(nChannels,std::vector<double>(fNsamples,fBaseline));

      for (auto const& litesimphotons : (*pmtHandle)){
	ch = litesimphotons.OpChannel;
	if(map.pdType(ch, "pmt")){ //only getting PMT TPB coated channels
  	  std::map< int, int > const& photonMap = litesimphotons.DetectedPhotons;
  	  t_min=FindMinimumTimeLite(photonMap);
	  CreatePDWaveformLite(photonMap, t_min, waves[ch]);
	  waveforms[ch] = std::vector<short unsigned int> (waves[ch].begin(), waves[ch].end());
	  raw::OpDetWaveform adcVec(t_min, (unsigned int)ch, waveforms[ch]);//including pre trigger window and transit time
	  pulseVecPtr->emplace_back(std::move(adcVec));
	  
	}
      }
     }  //end loop on simphoton lite collections
      e.put(std::move(pulseVecPtr));
    }else{ //for SimPhotons
      //art::Handle< std::vector<sim::SimPhotons> > pmtHandle;
      //e.getByLabel(fInputModuleName, pmtHandle);

      //Get *ALL* SimPhotonsCollection from Event
      std::vector< art::Handle< std::vector< sim::SimPhotons > > > photon_handles;
      e.getManyByType(photon_handles);
      if (photon_handles.size() == 0)
	throw art::Exception(art::errors::ProductNotFound)<<"sim SimPhotons retrieved and you requested them.";
      
      // Loop over direct/reflected photons
      for (auto pmtHandle: photon_handles) {
       // Do some checking before we proceed
        if (!pmtHandle.isValid()) continue;  
        if (pmtHandle.provenance()->moduleLabel() != fInputModuleName) continue;   //not the most efficient way of doing this, but preserves the logic of the module. Andrzej
 
      //this now tells you if light collection is reflected
        bool Reflected = (pmtHandle.provenance()->productInstanceName() == "Reflected");
      
//       if(!pmtHandle.isValid()){
//         std::cout <<Form("Did not find any G4 photons from a producer: %s", "largeant") << std::endl;
//       }
       if(Reflected)
	  {std::cout << "looking at reflected/visible photons" << std::endl; }
	  else
	  {std::cout << "looking at direct/vuv photons" << std::endl;  }  
	
	
      std::cout << "Number of photon channels: " << pmtHandle->size() << std::endl;
      unsigned int nChannels = pmtHandle->size();

      std::vector<std::vector<short unsigned int>> waveforms(nChannels,std::vector<short unsigned int> (fNsamples,0));
      std::vector<std::vector<double>> waves(nChannels,std::vector<double>(fNsamples,fBaseline));

      for (auto const& simphotons : (*pmtHandle)){
	ch = simphotons.OpChannel();

	if(map.pdType(ch, "pmt") || map.pdType(ch, "barepmt")){ //all PMTs
  	  t_min=FindMinimumTime(simphotons, map.pdName(ch));
	  CreatePDWaveform(simphotons, t_min, waves[ch], map.pdName(ch));
 	  waveforms[ch] = std::vector<short unsigned int> (waves[ch].begin(), waves[ch].end());
	  raw::OpDetWaveform adcVec(t_min, (unsigned int)ch, waveforms[ch]);//including pre trigger window and transit time
	  pulseVecPtr->emplace_back(std::move(adcVec));
	}
      }
     }  //end loop on photon collections.
    e.put(std::move(pulseVecPtr));
    }
  }

  DEFINE_ART_MODULE(SimPMTSBND)

  bool SimPMTSBND::EnergyRange(int type, double energy)//Testing if the photon has the energy to be detected by coated or uncoated PMT
  {
    if(type==1){ //Coated PMT (assuming they don't detect TPB converted light
	if(energy>9.5e-6 && energy<10.0e-6) return true;
	else return false;
    }else{ //Uncoated PMT
	if(energy>=2.0e-6 && energy<=3.22e-6) return true;
	else return false;
    }
    return false;
  }

  double SimPMTSBND::Pulse1PE(double time)//single pulse waveform
  {
    if (time < fTransitTime) return (fADC*fMeanAmplitude*std::exp(-1.0*pow(time - fTransitTime,2.0)/(2.0*pow(sigma1,2.0))));
    else return (fADC*fMeanAmplitude*std::exp(-1.0*pow(time - fTransitTime,2.0)/(2.0*pow(sigma2,2.0))));
  }

  void SimPMTSBND::AddSPE(size_t time_bin, std::vector<double>& wave){

    size_t min=0;
    size_t max=0;

    if(time_bin<fNsamples){
	min=time_bin;
	max=time_bin+pulsesize < fNsamples ? time_bin+pulsesize : fNsamples;
	for(size_t i = min; i<= max; i++){
	  wave[i]+= wsp[i-min];	
	}	
    }
  }

  void SimPMTSBND::CreatePDWaveform(sim::SimPhotons const& simphotons, double t_min, std::vector<double>& wave, std::string pdtype){

    if(pdtype=="pmt"){
      for(size_t i=0; i<simphotons.size(); i++){
//	double ttpb = timeTPB->GetRandom(); //for including TPB emission time
	double ttpb = 0; //not including TPB emission time
	if((gRandom->Uniform(1.0))<fQE) AddSPE((tadd+simphotons[i].Time+ttpb-t_min)*fSampling,wave);
      }
    }else{   
      for(size_t i=0; i<simphotons.size(); i++){
	if(EnergyRange(2,simphotons[i].Energy) && ((gRandom->Uniform(1.0))<fQE)) 
	  AddSPE((tadd+simphotons[i].Time-t_min)*fSampling,wave);
      }
    }
    if(fBaselineRMS>0.0) AddLineNoise(wave);
    if(fDarkNoiseRate > 0.0) AddDarkNoise(wave);
    CreateSaturation(wave);
  }

  void SimPMTSBND::CreatePDWaveformLite(std::map< int, int > const& photonMap, double t_min, std::vector<double>& wave){

    for (auto const& mapMember: photonMap){
      for(int i=0; i<mapMember.second; i++){
   	 if((gRandom->Uniform(1.0))<(fQE)){
//	double ttpb = timeTPB->GetRandom(); //for including TPB emission time
	double ttpb = 0; //not including TPB emission time
 	   AddSPE((tadd+mapMember.first+ttpb-t_min)*fSampling,wave);
         }
      }
    }
    if(fBaselineRMS>0.0) AddLineNoise(wave);
    if(fDarkNoiseRate > 0.0) AddDarkNoise(wave);
    CreateSaturation(wave);
  }

  void SimPMTSBND::CreateSaturation(std::vector<double>& wave){ //Implementing saturation effects

    for(size_t k=0; k<fNsamples; k++){ 
	if(wave[k]<(fBaseline+fSaturation*fADC*fMeanAmplitude))
	  wave[k]=fBaseline+fSaturation*fADC*fMeanAmplitude;	  
    }
  }

  void SimPMTSBND::AddLineNoise(std::vector<double>& wave){

    double noise = 0.0;

    for(size_t i = 0; i<wave.size(); i++){
	noise = gRandom->Gaus(0,fBaselineRMS); //gaussian noise
	wave[i] += noise;
    }
  }

  void SimPMTSBND::AddDarkNoise(std::vector< double >& wave)
  {
    size_t timeBin=0;

    // Multiply by 10^9 since fDarkNoiseRate is in Hz (conversion from s to ns)
    double darkNoiseTime = static_cast< double >(gRandom->Exp((1.0/fDarkNoiseRate)*1000000000.0));
    while (darkNoiseTime < wave.size()){
	timeBin = (darkNoiseTime);
	AddSPE(timeBin,wave);
        // Find next time to add dark noise
        darkNoiseTime += static_cast< double >(gRandom->Exp((1.0/fDarkNoiseRate)*1000000000.0));
    }
  }

  double SimPMTSBND::FindMinimumTime(sim::SimPhotons const& simphotons, std::string pdtype){
    double t_min=1e15;
    if(pdtype=="pmt"){
      for(size_t i=0; i<simphotons.size(); i++){	 	 
      	if(simphotons[i].Time<t_min) t_min = simphotons[i].Time;
      }
    }else{
      for(size_t i=0; i<simphotons.size(); i++){	 	 
        if(EnergyRange(2,simphotons[i].Energy) && simphotons[i].Time<t_min) t_min = simphotons[i].Time;
      }
    }
    return t_min;
  }

  double SimPMTSBND::FindMinimumTimeLite(std::map< int, int > const& photonMap){

    for (auto const& mapMember: photonMap){
 	 if(mapMember.second!=0) return (double)mapMember.first;
    }
    return 1e5;
  }

}
