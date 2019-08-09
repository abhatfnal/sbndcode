// SBNDThermalNoiseService_InTime.h
// Andrew Scarff (University of Sheffield)
// July 2019

// Based upon SPhaseChannelNoiseService.h created by Jingbo Wang (UC Davis) for ProtoDUNE.
// This Service runs the thermal (random) noise model, and adds it to the channel in time.
//
// The default parameters set in: sbndcode/DetectorSim/Services/noiseservice_sbnd.fcl
//

#ifndef SBNDThermalNoiseServiceInTime_H
#define SBNDThermalNoiseServiceInTime_H

#include "sbndcode/DetectorSim/Services/ChannelNoiseService.h"
#include <vector>
#include <iostream>
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/RandomEngine.h"
#include "nutools/RandomUtils/NuRandomService.h"
#include "fhiclcpp/ParameterSet.h"

class TH1;
namespace CLHEP {
class HepRandomEngine;
}

class SBNDThermalNoiseServiceInTime : public ChannelNoiseService {

public:

  // Ctor.
  SBNDThermalNoiseServiceInTime(fhicl::ParameterSet const& pset);

  // Ctor.
  SBNDThermalNoiseServiceInTime(fhicl::ParameterSet const& pset, art::ActivityRegistry&);

  // Dtor.
  ~SBNDThermalNoiseServiceInTime();

  // Add noise to a signal array.
  int addNoise(Channel chan, AdcSignalVector& sigs) const;

  // Print the configuration.
  std::ostream& print(std::ostream& out =std::cout, std::string prefix ="") const;

private:
  
  // General parameters
  unsigned int fNoiseArrayPoints;  ///< number of points in randomly generated noise array
  int          fRandomSeed;        ///< Seed for random number service. If absent or zero, use SeedSvc.
  int          fLogLevel;          ///< Log message level: 0=quiet, 1=init only, 2+=every event
  std::map< double, int > fShapingTimeOrder;
  
  // Declare noise engines.
  CLHEP::HepRandomEngine* m_pran;
  CLHEP::HepRandomEngine* fNoiseEngine;

  //Function to allow use of noise engine in ChannelNoiseService setup.
  void InitialiseProducerDeps(art::EDProducer * EDProdPointer, fhicl::ParameterSet const& pset) override{
    
    CLHEP::HepRandomEngine& NoiseEngine((art::ServiceHandle<rndm::NuRandomService>{}->createEngine(*EDProdPointer,"HepJamesRandom","noise",pset,"Seed")));
    fNoiseEngine = &NoiseEngine;
    return; 
  } 

};

DECLARE_ART_SERVICE_INTERFACE_IMPL(SBNDThermalNoiseServiceInTime, ChannelNoiseService, LEGACY)

#endif
