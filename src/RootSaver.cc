// $Id: RootSaver.cc 94 2010-01-26 13:18:30Z adotti $
/**
  * @file   RootSaver.cc
  *
  * @date   17 Dec 2009
  * @author adotti
  *
  * @brief  Implements class RootSaver.
  */

// Local headers
#include "DetectorConstruction.hh"
#include "RootSaver.hh"
#include "MagneticField.hh"
#include "SiHit.hh"
#include "EventAction.hh"

// Geant4 headers
#include "G4DigiManager.hh"
#include "G4UserSteppingAction.hh"
#include "G4VSolid.hh"

// Root headers
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TF1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TObject.h"
#include "TVectorD.h"

// Default headers
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;
using namespace CLHEP;

//-------------------------------------------------------------------------//

RootSaver::RootSaver() : //Initializing parameters
                         rootTree(0), runCounter(0), nStrips(0),
                         Signal0(0), Signal1(0), Signal2(0),
                         Signal3(0), Signal5(0), Signal6(0),
                         Signal7(0),
                         TruthPosx(0), TruthPosy(0), TruthPosz(0),
                         TruthAngle_theta(0), TruthAngle_phi(0),
                         Px_dssd(0), Py_dssd(0), Pz_dssd(0),
                         T_dssd(0),
                         Ekin_dssd2(0), StripNumber(0)
{
}

//-------------------------------------------------------------------------//

RootSaver::~RootSaver()
{
        //Close current file if needed
        if (rootTree)
        {
                CloseTree();
        }
}

//-------------------------------------------------------------------------//

void RootSaver::CreateTree(const std::string &fileName, const std::string &treeName)
{
        // Retrieving inputs
        Inputs *Inputs = &Inputs::GetInputs();

        // Open a list
        // Name
        stringstream f;
        f << "Data for " << Inputs->ejectile_Ex << "MeV"
          << ".txt";

        // Creating a list
        list.open(f.str().data(), ios_base::app);

        // Getting current values
        G4double Current1 = Inputs->Current1 * 1000;

        // Total hits starts at zero
        HitsOnDetector = 0;
        HitsOnTarget = 0;
        for (int i = 0; i <= 7; i++)
        {
                RecoilHit[i] = 0;
                EjectileHit[i] = 0;
        }

        if (rootTree)
        {
                std::cerr << "TTree already created, first call CloseTree" << std::endl;
                return;
        }

        // Path to where ROOT should save the files
        G4String Path = "/home/leo/Desktop/RIBRAS-1S/RIBRAS-1S/ROOT/";

        // Creating ROOT file
        std::ostringstream fn;
        fn.precision(2);
        fn.setf(std::ios::fixed, std::ios::floatfield);
        fn << Path << fileName << "_" << Current1 << ".root";

        // Create a new file and open it for writing, if the file already exists the file is overwritten
        TFile *rootFile = TFile::Open(fn.str().data(), "recreate");

        if (rootFile == 0 || rootFile->IsZombie())
        {
                G4cerr << "Error opening the file: " << fn.str() << " TTree will not be saved." << G4endl;
                return;
        }

        rootTree = new TTree(treeName.data(), treeName.data());

        nStrips = 60;
        Signal0 = new Float_t[nStrips];
        Signal1 = new Float_t[nStrips];
        Signal2 = new Float_t[nStrips];
        Signal3 = new Float_t[nStrips];
        Signal4 = new Float_t[nStrips];
        Signal5 = new Float_t[nStrips];
        Signal6 = new Float_t[nStrips];
        Signal7 = new Float_t[nStrips];

        for (Int_t strip = 0; strip < nStrips; ++strip)
        {
                Signal0[strip] = 0;
                Signal1[strip] = 0;
                Signal2[strip] = 0;
                Signal3[strip] = 0;
                Signal4[strip] = 0;
                Signal5[strip] = 0;
                Signal6[strip] = 0;
                Signal7[strip] = 0;
        }

        // Digits variables
        //-------- Total energy per strip ----------//
        rootTree->Branch("E0", Signal0, "E0[60]/F");
        rootTree->Branch("E1", Signal1, "E1[60]/F");
        rootTree->Branch("E2", Signal2, "E2[60]/F");
        rootTree->Branch("E3", Signal3, "E3[60]/F");
        rootTree->Branch("E4", Signal4, "E4[60]/F");
        rootTree->Branch("E5", Signal5, "E5[60]/F");
        rootTree->Branch("E6", Signal6, "E6[60]/F");
        rootTree->Branch("E7", Signal7, "E7[60]/F");
        //------------------------------------------//

        //--- Hit Position ( Detector Reference ) ---//
        rootTree->Branch("pos_x_det0", &Pos_x_det[0]);
        rootTree->Branch("pos_y_det0", &Pos_y_det[0]);
        rootTree->Branch("pos_z_det0", &Pos_z_det[0]);

        rootTree->Branch("pos_x_det1", &Pos_x_det[1]);
        rootTree->Branch("pos_y_det1", &Pos_y_det[1]);
        rootTree->Branch("pos_z_det1", &Pos_z_det[1]);

        rootTree->Branch("pos_x_det2", &Pos_x_det[2]);
        rootTree->Branch("pos_y_det2", &Pos_y_det[2]);
        rootTree->Branch("pos_z_det2", &Pos_z_det[2]);

        rootTree->Branch("pos_x_det3", &Pos_x_det[3]);
        rootTree->Branch("pos_y_det3", &Pos_y_det[3]);
        rootTree->Branch("pos_z_det3", &Pos_z_det[3]);

        rootTree->Branch("pos_x_det4", &Pos_x_det[4]);
        rootTree->Branch("pos_y_det4", &Pos_y_det[4]);
        rootTree->Branch("pos_z_det4", &Pos_z_det[4]);

        rootTree->Branch("pos_x_det5", &Pos_x_det[5]);
        rootTree->Branch("pos_y_det5", &Pos_y_det[5]);
        rootTree->Branch("pos_z_det5", &Pos_z_det[5]);

        rootTree->Branch("pos_x_det6", &Pos_x_det[6]);
        rootTree->Branch("pos_y_det6", &Pos_y_det[6]);
        rootTree->Branch("pos_z_det6", &Pos_z_det[6]);

        rootTree->Branch("pos_x_det7", &Pos_x_det[7]);
        rootTree->Branch("pos_y_det7", &Pos_y_det[7]);
        rootTree->Branch("pos_z_det7", &Pos_z_det[7]);

        rootTree->Branch("pos_x_target", &Pos_x_target);
        rootTree->Branch("pos_y_target", &Pos_z_target);
        rootTree->Branch("pos_z_target", &Pos_z_target);
        //-------------------------------------------//

        //------- Total energy in the detector-------//
        rootTree->Branch("ener_det0", &E_det[1]);
        rootTree->Branch("ener_det1", &E_det[2]);
        rootTree->Branch("ener_det2", &E_det[3]);
        rootTree->Branch("ener_det3", &E_det[4]);
        rootTree->Branch("ener_det4", &E_det[5]);
        rootTree->Branch("ener_det5", &E_det[6]);
        rootTree->Branch("ener_det6", &E_det[7]);
        rootTree->Branch("ener_det7", &E_det[8]);
        //-------------------------------------------//

        rootTree->Branch("truthPosx", &TruthPosx);
        rootTree->Branch("truthPosy", &TruthPosy);
        rootTree->Branch("truthPosz", &TruthPosz);

        rootTree->Branch("truthAngle_theta", &TruthAngle_theta);
        rootTree->Branch("truthAngle_phi", &TruthAngle_phi);

        rootTree->Branch("px_dssd", &Px_dssd);
        rootTree->Branch("py_dssd", &Py_dssd);
        rootTree->Branch("pz_dssd", &Pz_dssd);

        rootTree->Branch("t_dssd", &T_dssd);
        rootTree->Branch("t_sili1", &T_sili[1]);
        rootTree->Branch("t_sili2", &T_sili[2]);
        rootTree->Branch("t_dssd2", &T_dssd);
        rootTree->Branch("Ekin_dssd2", &Ekin_dssd2);

        rootTree->Branch("Strip_Number", &StripNumber);
}

//-------------------------------------------------------------------------//

void RootSaver::CloseTree()
{
        // Check if ROOT TTree exists,
        // in case get the associated file and close it.
        // Note that if a TFile goes above 2GB a new file
        // will be automatically opened. We have thus to get,
        // from the TTree the current opened file

        // Retrieving inputs
        Inputs *Inputs = &Inputs::GetInputs();

        G4int TotalRecoilHits = 0;
        G4int TotalEjectileHits = 0;
        G4int verbose = 1;

        if (rootTree)
        {
                if (verbose > 0)
                {
                        G4cout << " " << G4endl;
                        G4cout << " --------- Efficiency --------- " << G4endl;
                        for (int i = 0; i <= 7; i++)
                        {
                                if (i == 0)
                                        G4cout << " ------- Rear Detectors -------" << G4endl;
                                if (i == 4)
                                        G4cout << " ------- Frontal Detectors -------" << G4endl;

                                G4cout << "Detector " << i << ":" << G4endl;
                                G4cout << "    "
                                       << "Recoil particle: " << RecoilHit[i] * 100 / HitsOnTarget << "%" << G4endl;
                                G4cout << "    "
                                       << "Ejectile particle: " << EjectileHit[i] * 100 / HitsOnTarget << "%" << G4endl;

                                TotalRecoilHits = TotalRecoilHits + RecoilHit[i];
                                TotalEjectileHits = TotalEjectileHits + EjectileHit[i];
                        }

                        G4cout << "Total: " << HitsOnDetector * 100 / HitsOnTarget << "%" << G4endl;
                        G4cout << "Recoil particles: " << TotalRecoilHits * 100 / HitsOnTarget << "%" << G4endl;
                        G4cout << "Ejectile particles: " << TotalEjectileHits * 100 / HitsOnTarget << "%" << G4endl;
                        G4cout << " " << G4endl;
                }

                // Saving data to a txt
                list << " " << G4endl;
                list << "Current: " << Inputs->Current1 * 1000 << G4endl;
                list << " --------- Efficiency --------- " << G4endl;
                for (int i = 0; i <= 7; i++)
                {
                        if (i == 0)
                                list << " ------- Rear Detectors -------" << G4endl;
                        if (i == 4)
                                list << " ------- Frontal Detectors -------" << G4endl;

                        list << "Detector " << i << ":" << G4endl;
                        list << "    "
                             << "Recoil particle: " << RecoilHit[i] * 100 / HitsOnTarget << "%" << G4endl;
                        list << "    "
                             << "Ejectile particle: " << EjectileHit[i] * 100 / HitsOnTarget << "%" << G4endl;

                        TotalRecoilHits = TotalRecoilHits + RecoilHit[i];
                        TotalEjectileHits = TotalEjectileHits + EjectileHit[i];
                }

                list << "Total: " << HitsOnDetector * 100 / HitsOnTarget << "%" << G4endl;
                list << "Recoil particles: " << TotalRecoilHits * 100 / HitsOnTarget << "%" << G4endl;
                list << "Ejectile particles: " << TotalEjectileHits * 100 / HitsOnTarget << "%" << G4endl;
                list << "  --------------------------------- " << G4endl;

                list.close();

                // Saving total of hits on a vector
                TVectorD TotalHits(2);
                TotalHits[0] = HitsOnTarget;
                TotalHits[1] = HitsOnDetector;
                TotalHits.Write("Total Hits");

                // Write root file
                rootTree->Write();

                // Checking if there's a file
                TFile *currentFile = rootTree->GetCurrentFile();
                if (currentFile == 0 || currentFile->IsZombie())
                {
                        G4cerr << "Error closing TFile " << G4endl;
                        return;
                }
                currentFile->Close();

                //The root is automatically deleted.
                rootTree = 0;
                delete[] Signal0;
                delete[] Signal1;
                delete[] Signal2;
                delete[] Signal3;
                delete[] Signal4;
                delete[] Signal5;
                delete[] Signal6;
                delete[] Signal7;
        }
}

//-------------------------------------------------------------------------//

void RootSaver::AddEvent(const SiHitCollection *const hits, const G4ThreeVector &primPos, const G4ThreeVector &primMom)
{
        // Retrieving Inputs
        Inputs *Inputs = &Inputs::GetInputs();

        // If root TTree is not created ends
        if (rootTree == 0)
        {
                return;
        }

        // Store Hits information
        if (hits->entries())
        {
                // Getting number of hits
                G4int nHits = hits->entries();

                // Set defaults values
                // Kinect energy
                Ekin_dssd2 = 0;

                // Momentum
                Px_dssd = -1000;
                Py_dssd = -1000;
                Pz_dssd = -1000;

                // Time
                T_dssd = -1000;

                // Strip Number
                StripNumber = -1000;

                // Flags
                G4bool HitOnTargetCouted = false;
                G4bool HitOnDecCounted = false;

                // Loop on all hits, consider only the hits for secondary particles
                // Position is weighted average of hit x()
                for (G4int h = 0; (h < nHits); ++h)
                {
                        const SiHit *hit = static_cast<const SiHit *>(hits->GetHit(h));

                        // Getting logical name of the detector
                        G4String DetectorName = hit->GetLogicalVolume()->GetName();

                        // Hit on detector or on target
                        if (hit->GetHitOnTarget() && !HitOnTargetCouted)
                        {
                                HitsOnTarget++;
                                HitOnTargetCouted = true;
                        }

                        if (hit->GetHitOnDetector() && !HitOnDecCounted)
                        {
                                HitsOnDetector++;
                                HitOnDecCounted = true;

                                // Getting particle ID
                                G4int particleID = hit->GetParticleID();

                                // Getting detector ID
                                G4int detectorID = hit->GetPlaneNumber();

                                for (int i = 0; i <= 7; i++)
                                {
                                        if (detectorID == i)
                                        {
                                                if (particleID == 2)
                                                        RecoilHit[detectorID]++;
                                                if (particleID == 3)
                                                        EjectileHit[detectorID]++;
                                        }
                                }
                        }

                        // Getting what strip ocurred a hit
                        G4int stripNum = hit->GetStripNumber();
                        StripNumber = stripNum;

                        // Same for detector
                        G4int planeNum = hit->GetPlaneNumber();

                        // Getting position of hit (detector reference)
                        G4ThreeVector pos = hit->GetPosition();
                        G4double x = pos.x();
                        G4double y = pos.y();
                        G4double z = pos.z();

                        // Getting momentum
                        G4ThreeVector momentum = hit->GetIncidenceMomentumDirection();
                        G4double momet_x = momentum.x();
                        G4double momet_y = momentum.y();
                        G4double momet_z = momentum.z();

                        // Hit time
                        G4double tiempo = hit->GetIncidenceTime();

                        // We save xyz in mm (detector coordinates)
                        x /= CLHEP::mm;
                        y /= CLHEP::mm;
                        z /= CLHEP::mm;

                        // Time in nanoseconds
                        tiempo /= CLHEP::ns;

                        // We save energy in MeV
                        Float_t edep = static_cast<Float_t>(hit->GetEdep());
                        edep /= CLHEP::MeV;

                        // Saving information for each detector
                        if (DetectorName == "SensorStripD00" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal0[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD01" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal1[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD02" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal2[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD03" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal3[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD04" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal4[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD05" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal5[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD06" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal6[stripNum] += Econv;
                        }
                        else if (DetectorName == "SensorStripD07" && hit->GetIsPrimary() == 0)
                        {
                                Pos_x_det[planeNum] = x;
                                Pos_y_det[planeNum] = y;
                                Pos_z_det[planeNum] = z;
                                T_sili[planeNum] = tiempo;
                                double Econv = Digital(edep);
                                E_det[planeNum] += Econv;
                                Signal7[stripNum] += Econv;
                        }
                        else if (DetectorName == "Log_Target")
                        {
                                Pos_x_target = x;
                                Pos_y_target = y;
                                Pos_z_target = z;
                        }
                        else
                        {
                                // G4cerr << "Hit Error: Plane number " << planeNum << " expected max value: 8" << G4endl;
                                continue;
                        }
                }
                // Then fill the root tree
                rootTree->Fill();
        }

        TruthPosx = static_cast<Float_t>(primPos.x());
        TruthPosy = static_cast<Float_t>(primPos.y());
        TruthPosz = static_cast<Float_t>(primPos.z());

        //Measure angle of the beam in xz plane measured from z+ direction
        // -pi<Angle<=pi (positive when close to x positiove direction)
        Float_t sign_z = (primMom.z() >= 0) ? +1 : -1;
        Float_t sign_x = (primMom.x() >= 0) ? +1 : -1;
        TruthAngle_theta = (primMom.z() != 0) ? TMath::PiOver2() * sign_x * (1 - sign_z) + std::atan(primMom.x() / primMom.z())
                                              : sign_x * TMath::PiOver2(); //beam perpendicular to z
        TruthAngle_theta /= CLHEP::deg;

        Float_t sign_y = (primMom.y() >= 0) ? +1 : -1;
        TruthAngle_phi = (primMom.x() != 0) ? sign_y * std::abs(atan(primMom.y() / primMom.x()))
                                            : sign_y * TMath::PiOver2(); //beam perpendicular to x
        TruthAngle_phi /= CLHEP::deg;
}

//-------------------------------------------------------------------------//

double RootSaver::Digital(double Eraw)
{
        double ion_pot = 3.6e-6; //eV
        double fanofactor = 0.1;
        double sigmaElnoise = 11.75e-3; //keV
        int Nelectrons = floor(Eraw / ion_pot);
        Nelectrons = gRandom->Gaus(Nelectrons, sqrt(Nelectrons * fanofactor));
        double energypoint = Nelectrons * ion_pot;
        energypoint = gRandom->Gaus(energypoint, sigmaElnoise); //electronic noise

        return energypoint;
}
