#include "CRTEventDisplayAlg.h"

namespace sbnd{

CRTEventDisplayAlg::CRTEventDisplayAlg(const Config& config){

  this->reconfigure(config);
  
}


CRTEventDisplayAlg::CRTEventDisplayAlg(){

}


CRTEventDisplayAlg::~CRTEventDisplayAlg(){

}


void CRTEventDisplayAlg::reconfigure(const Config& config){

  fSimLabel         = config.SimLabel();
  fFEBDataLabel     = config.FEBDataLabel();
  fSimDepositsLabel = config.SimDepositsLabel();
  fCRTHitLabel      = config.CRTHitLabel();
  fCRTTrackLabel    = config.CRTTrackLabel();
  fClockSpeedCRT    = config.ClockSpeedCRT();

  fDrawTaggers      = config.DrawTaggers();
  fDrawModules      = config.DrawModules();
  fDrawFEBs         = config.DrawFEBs();
  fDrawStrips       = config.DrawStrips();
  fDrawFEBData      = config.DrawFEBData();
  fDrawSimDeposits  = config.DrawSimDeposits();
  fDrawCrtHits      = config.DrawCrtHits();
  fDrawCrtTracks    = config.DrawCrtTracks();
  fDrawTrueTracks   = config.DrawTrueTracks();
  
  fDrawIncompleteTracks = config.DrawIncompleteTracks();

  fTaggerColour      = config.TaggerColour();
  fFEBEndColour      = config.FEBEndColour();
  fStripColour       = config.StripColour();
  fFEBDataColour     = config.FEBDataColour();
  fSimDepositsColour = config.SimDepositsColour();
  fCrtHitColour      = config.CrtHitColour();
  fCrtTrackColour    = config.CrtTrackColour();
  fTrueTrackColour   = config.TrueTrackColour();

  fUseTrueID = config.UseTrueID();
  fTrueID    = config.TrueID();

  fPrint = config.Print();

  fLineWidth             = config.LineWidth();
  fIncompleteTrackLength = config.IncompleteTrackLength();
  fMinTime               = config.MinTime();
  fMaxTime               = config.MaxTime();
  
  fCrtBackTrack          = config.CrtBackTrack();

  return;

}
 
void CRTEventDisplayAlg::SetDrawTaggers(bool tf){
  fDrawTaggers = tf;
}
void CRTEventDisplayAlg::SetDrawFEBs(bool tf){
  fDrawFEBs = tf;
}
void CRTEventDisplayAlg::SetDrawStrips(bool tf){
  fDrawStrips = tf;
}
void CRTEventDisplayAlg::SetDrawSimDeposits(bool tf){
  fDrawSimDeposits = tf;
}
void CRTEventDisplayAlg::SetDrawFEBData(bool tf){
  fDrawFEBData = tf;
}
void CRTEventDisplayAlg::SetDrawCrtHits(bool tf){
  fDrawCrtHits = tf;
}
void CRTEventDisplayAlg::SetDrawCrtTracks(bool tf){
  fDrawCrtTracks = tf;
}
void CRTEventDisplayAlg::SetDrawTrueTracks(bool tf){
  fDrawTrueTracks = tf;
}
void CRTEventDisplayAlg::SetPrint(bool tf){
  fPrint = tf;
}

void CRTEventDisplayAlg::SetTrueId(int id){
  fUseTrueID = true;
  fTrueID = id;
}

bool CRTEventDisplayAlg::IsVisible(const simb::MCParticle& particle){
  int pdg = std::abs(particle.PdgCode());
  double momentum = particle.P();
  double momLimit = 0.05;
  if(momentum < momLimit) return false;
  if(pdg == 13) return true;
  if(pdg == 11) return true;
  if(pdg == 2212) return true;
  if(pdg == 211) return true;
  if(pdg == 321) return true;
  return false;
}

void CRTEventDisplayAlg::DrawCube(TCanvas *c1, double *rmin, double *rmax, int colour){

  c1->cd();
  TList *outline = new TList;
  TPolyLine3D *p1 = new TPolyLine3D(4);
  TPolyLine3D *p2 = new TPolyLine3D(4);
  TPolyLine3D *p3 = new TPolyLine3D(4);
  TPolyLine3D *p4 = new TPolyLine3D(4);
  p1->SetLineColor(colour);
  p1->SetLineWidth(fLineWidth);
  p1->Copy(*p2);
  p1->Copy(*p3);
  p1->Copy(*p4);
  outline->Add(p1);
  outline->Add(p2);
  outline->Add(p3);
  outline->Add(p4); 
  TPolyLine3D::DrawOutlineCube(outline, rmin, rmax);
  p1->Draw();
  p2->Draw();
  p3->Draw();
  p4->Draw();

}

void CRTEventDisplayAlg::Draw(detinfo::DetectorClocksData const& clockData,
                           const art::Event& event){
  // Create a canvas 
  TCanvas *c1 = new TCanvas("c1","",700,700);

  // Draw the CRT taggers
  if(fDrawTaggers){
    for(auto const &[name, tagger] : fCrtGeo.GetTaggers()){
      double rmin[3] = {tagger.minX, 
                        tagger.minY, 
                        tagger.minZ};
      double rmax[3] = {tagger.maxX, 
                        tagger.maxY, 
                        tagger.maxZ};

      std::cout<<std::endl<<"Tagger: "<<name<<", min: ("<<tagger.minX<<", "<<tagger.minY<<", "<<tagger.minZ<<"), max: ("<<tagger.maxX<<", "<<tagger.maxY<<", "<<tagger.maxZ<<")"<<std::endl;
      DrawCube(c1, rmin, rmax, fTaggerColour);
    }
  }

  // Draw individual CRT modules
  if(fDrawModules){
    for(auto const &[name, module] : fCrtGeo.GetModules()){
      double rmin[3] = {module.minX, 
                        module.minY, 
                        module.minZ};
      double rmax[3] = {module.maxX, 
                        module.maxY, 
                        module.maxZ};
      DrawCube(c1, rmin, rmax, fTaggerColour);

      if(fDrawFEBs){
        const std::array<double, 6> febPos = fCrtGeo.FEBWorldPos(module);

        double rmin1[3] = {febPos[0],
                      febPos[2],
                      febPos[4]};

        double rmax1[3] = {febPos[1],
                      febPos[3],
                      febPos[5]};

        DrawCube(c1, rmin1, rmax1, fTaggerColour);
        
        // Draw FEBEnds
        const std::array<double, 6> febCh0Pos = fCrtGeo.FEBChannel0WorldPos(module);

        double rminCh0[3] = {febCh0Pos[0],
                              febCh0Pos[2],
                              febCh0Pos[4]};

        double rmaxCh0[3] = {febCh0Pos[1],
                              febCh0Pos[3],
                              febCh0Pos[5]};

        DrawCube(c1, rminCh0, rmaxCh0, fFEBEndColour);

        

      }
    }
  }
  
  // Draw CRT Strip
  if(fDrawStrips){
    for(auto const &[name, strip] : fCrtGeo.GetStrips()){
      //if(fChoseTaggers && std::find(fChosenTaggers.begin(), fChosenTaggers.end(), fCrtGeo.ChannelToTaggerEnum(strip.channel0)) == fChosenTaggers.end()) continue;

      double rmin[3] = {strip.minX, 
                        strip.minY, 
                        strip.minZ};
      double rmax[3] = {strip.maxX, 
                        strip.maxY, 
                        strip.maxZ};
      DrawCube(c1, rmin, rmax, fStripColour);
    }
  }

  // Draw Energy depositions in the event
  if(fDrawSimDeposits){
    auto simDepositsHandle = event.getValidHandle<std::vector<sim::AuxDetSimChannel>>(fSimDepositsLabel);
    
    for(auto const simDep : *simDepositsHandle){
      for(auto const ide : simDep.AuxDetIDEs()){
        double x = (ide.entryX + ide.exitX) / 2.;
        double y = (ide.entryY + ide.exitY) / 2.;
        double z = (ide.entryZ + ide.exitZ) / 2.;
        double t = (ide.entryT + ide.exitT) / 2.;

        //if(t < fMinTime || t > fMaxTime) continue;

        double ex = std::abs(ide.entryX - ide.exitX) / 2.;
        double ey = std::abs(ide.entryY - ide.exitY) / 2.;
        double ez = std::abs(ide.entryZ - ide.exitZ) / 2.;

        ex = std::max(ex, 1.);
        ey = std::max(ey, 1.);
        ez = std::max(ez, 1.);

        double rmin[3] = { x - ex, y - ey, z - ez};
        double rmax[3] = { x + ex, y + ey, z + ez};

        if(fPrint)
          std::cout << "Sim Energy Deposit: (" << x << ", " << y << ", " << z 
                    << ")  +/- (" << ex << ", " << ey << ", " << ez << ") by trackID: " 
                    << ide.trackID << " at t = " << t << std::endl;

        DrawCube(c1, rmin, rmax, fSimDepositsColour);
      }
    } 

    /*auto simDepositsHandle = event.getValidHandle<std::vector<sim::AuxDetHit>>(fSimDepositsLabel);
    for(auto const simDep : *simDepositsHandle){
      double x = (simDep.GetEntryX() + simDep.GetExitX()) / 2.;
      double y = (simDep.GetEntryY() + simDep.GetExitY()) / 2.;
      double z = (simDep.GetEntryZ() + simDep.GetExitZ()) / 2.;
      double t = (simDep.GetEntryT() + simDep.GetExitT()) / 2.;

      double ex = std::abs(simDep.GetEntryX() - simDep.GetExitX()) / 2.;
      double ey = std::abs(simDep.GetEntryY() - simDep.GetExitY()) / 2.;
      double ez = std::abs(simDep.GetEntryZ() - simDep.GetExitZ()) / 2.;

      ex = std::max(ex, 1.);
      ey = std::max(ey, 1.);
      ez = std::max(ez, 1.);
      double rmin[3] = { x - ex, y - ey, z - ez};
      double rmax[3] = { x + ex, y + ey, z + ez};
      if(fPrint)
        std::cout << "Sim Energy Deposit: (" << x << ", " << y << ", " << z 
                  << ")  +/- (" << ex << ", " << ey << ", " << ez << ") by trackID: " 
                  << simDep.GetTrackID() << " at t = " << t << std::endl;

      DrawCube(c1, rmin, rmax, fSimDepositsColour);
    }*/
  }

  // Draw the FEB data in the event
  if(fDrawFEBData){

    if(fPrint) std::cout<<std::endl<<"FEB data in event:"<<std::endl;

    auto FEBDataHandle = event.getValidHandle<std::vector<sbnd::crt::FEBData>>(fFEBDataLabel);
    //detinfo::DetectorClocks const* fDetectorClocks = lar::providerFrom<detinfo::DetectorClocksService>();
    //detinfo::ElecClock fTrigClock = fDetectorClocks->TriggerClock();
    for(auto const& data : (*FEBDataHandle)){

      // Skip if outside specified time window if time window used
      //fTrigClock.SetTime(data.T0());
      //double time = fTrigClock.Time();
      double time = (double)(int)data.Ts0()/fClockSpeedCRT; // [tick -> us]
      if(!(fMinTime == fMaxTime || (time > fMinTime && time < fMaxTime))) continue;

      // Skip if it doesn't match the true ID if true ID is used
      int trueId = fCrtBackTrack.TrueIdFromTotalEnergy(event, data);
      if(fUseTrueID && trueId != fTrueID) continue;

      std::string stripName = fCrtGeo.ChannelToStripName(data.Coinc());
      double rmin[3] = {fCrtGeo.GetStrip(stripName).minX, 
                        fCrtGeo.GetStrip(stripName).minY, 
                        fCrtGeo.GetStrip(stripName).minZ};
      double rmax[3] = {fCrtGeo.GetStrip(stripName).maxX, 
                        fCrtGeo.GetStrip(stripName).maxY, 
                        fCrtGeo.GetStrip(stripName).maxZ};
      DrawCube(c1, rmin, rmax, fFEBDataColour);

      if(fPrint) std::cout<<"->True ID: "<<trueId<<", channel = "<<data.Coinc()<<", tagger = "
                          <<fCrtGeo.GetModule(fCrtGeo.GetStrip(stripName).moduleName).taggerName<<", time = "<<time<<""<<std::endl;
    }
  }

  // Draw the CRT hits in the event
  if(fDrawCrtHits){

    if(fPrint) std::cout<<std::endl<<"CRT hits in event:"<<std::endl;

    auto crtHitHandle = event.getValidHandle<std::vector<sbn::crt::CRTHit>>(fCRTHitLabel);
    for(auto const& hit : (*crtHitHandle)){
      // Skip if outside specified time window if time window used
      /*      double time = (double)(int)hit.ts1_ns * 1e-3;
      if(!(fMinTime == fMaxTime || (time > fMinTime && time < fMaxTime))) continue;

      // Skip if it doesn't match the true ID if true ID is used
      int trueId = fCrtBackTrack.TrueIdFromTotalEnergy(event, hit);
      if(fUseTrueID && trueId != fTrueID) continue;
      */
      double rmin[3] = {hit.x_pos - hit.x_err,
                        hit.y_pos - hit.y_err,
                        hit.z_pos - hit.z_err};
      double rmax[3] = {hit.x_pos + hit.x_err,
                        hit.y_pos + hit.y_err,
                        hit.z_pos + hit.z_err};
      DrawCube(c1, rmin, rmax, fCrtHitColour);

      if(fPrint) std::cout << "Position = (" << hit.x_pos << ", " << hit.y_pos << ", " << hit.z_pos << ")"<<std::endl;
    }
  }

  // Draw CRT tracks in the event
  if(fDrawCrtTracks){

    if(fPrint) std::cout<<std::endl<<"CRT tracks in event:"<<std::endl;

    auto crtTrackHandle = event.getValidHandle<std::vector<sbn::crt::CRTTrack>>(fCRTTrackLabel);
    for(auto const& track : (*crtTrackHandle)){

      // Skip if outside specified time window if time window used
      //double time = (double)(int)track.ts1_ns * 1e-3; 
      //if(!(fMinTime == fMaxTime || (time > fMinTime && time < fMaxTime))) continue;

      // Skip if it doesn't match the true ID if true ID is used
      //int trueId = fCrtBackTrack.TrueIdFromTotalEnergy(event, track);

      //if(fUseTrueID && trueId != fTrueID) continue;

      TPolyLine3D *line = new TPolyLine3D(2);
      line->SetPoint(0, track.x1_pos, track.y1_pos, track.z1_pos);
      line->SetPoint(1, track.x2_pos, track.y2_pos, track.z2_pos);
      line->SetLineColor(fCrtTrackColour);
      line->SetLineWidth(fLineWidth);
      if(track.complete) line->Draw();
      else if(fDrawIncompleteTracks){
        TVector3 start(track.x1_pos, track.y1_pos, track.z1_pos);
        TVector3 end(track.x2_pos, track.y2_pos, track.z2_pos);
        if(start.Y() < end.Y()) std::swap(start, end);
        TVector3 diff = (end - start).Unit();
        TVector3 newEnd = start + fIncompleteTrackLength * diff;
        line->SetPoint(0, start.X(), start.Y(), start.Z());
        line->SetPoint(1, newEnd.X(), newEnd.Y(), newEnd.Z());
        line->Draw();
      }

      if(fPrint) std::cout<<"-> start = ("<<track.x1_pos<<", "
                          <<track.y1_pos<<", "<<track.z1_pos<<"), end = ("<<track.x2_pos
                          <<", "<<track.y2_pos<<", "<<track.z2_pos<<")"<<std::endl;;
    }
  }

  // Draw true track trajectories for visible particles that cross the CRT
  if(fDrawTrueTracks){

    if(fPrint) std::cout<<std::endl<<"True tracks in event:"<<std::endl;

    auto particleHandle = event.getValidHandle<std::vector<simb::MCParticle>>(fSimLabel);
    std::vector<double> crtLims = fCrtGeo.CRTLimits();
    for(auto const& part : (*particleHandle)){

      // Skip if it doesn't match the true ID if true ID is used
      //if(fUseTrueID && part.TrackId() != fTrueID) continue;

      // Skip if outside specified time window if time window used
      //std::cout<<"Time: "<<part.T()<<std::endl;
      //if(part.T() < fMinTime || part.T() > fMaxTime) continue;
      // Skip if particle isn't visible
      if(!IsVisible(part)) continue;

      // Skip if particle doesn't cross the boundary enclosed by the CRTs
      //      if(!fCrtGeo.EntersVolume(part)) continue;

      size_t npts = part.NumberTrajectoryPoints();
      TPolyLine3D *line = new TPolyLine3D(npts);
      int ipt = 0;
      bool first = true;
      geo::Point_t start {0,0,0};
      geo::Point_t end {0,0,0};
      for(size_t i = 0; i < npts; i++){
        geo::Point_t pos {part.Vx(i), part.Vy(i), part.Vz(i)};

        // Don't draw trajectories outside of the CRT volume
        //if(pos.X() < crtLims[0] || pos.X() > crtLims[3] || pos.Y() < crtLims[1] || pos.Y() > crtLims[4] || pos.Z() < crtLims[2] || pos.Z() > crtLims[5]) continue;

        if(first){
          first = false;
          start = pos;
        }
        end = pos;

        line->SetPoint(ipt, pos.X(), pos.Y(), pos.Z());
        ipt++;
      }
      line->SetLineColor(fTrueTrackColour);
      line->SetLineWidth(fLineWidth);
      line->Draw();

      if(fPrint) std::cout<<"MCParticle, Track ID: " << part.TrackId() << " PDG: " << part.PdgCode() << ", traj points: "<<npts<<", start = ("<<start.X()<<", "<<start.Y()<<", "<<start.Z()<<"), end = ("<<end.X()<<", "<<end.Y()<<", "<<end.Z()<<")"<<std::endl;
    }
  }

  c1->SaveAs("crtEventDisplay.root");

}

}
