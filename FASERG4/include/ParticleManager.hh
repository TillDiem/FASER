#ifndef PHOTONMANAGER_H
#define PHOTONMANAGER_H 1

#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "G4Event.hh"
#include "G4GenericMessenger.hh"
#include "TFile.h"
#include "TTree.h"
#include "Track.hh"
#include "typedef.h"
#include "TcalEvent.hh"

#include <G4UserTrackingAction.hh>

/**
 * @class ParticleManager
 * @brief Manages the photons and tracks of the simulation, preparing them for output into a root file
 */
class ParticleManager {
    private:
	std::map<int, Track*> m_particleMap;  ///< Map to store track ID and corresponding Track object
	TFile* m_rootFile;		     ///< ROOT file to store the rays
	TTree* m_vertexTree;		     ///< TTree to store the information about the primary vertices of the event
	TTree* m_particleTree;		     ///< TTree to store the information about the particles of the events
	TTree* m_detectorTree;	///< TTree to store information about the simulated detector geometry to the output file. Can be actived/deactived
				///< using the macro files.
	TTree* m_metaInformationTree;  ///< TTree to store meta information about the simulation, like the number of ray vectors, size of them and the
				       ///< number of events.
	//std::vector<Track>* m_particleVector;  ///< Vector to store the tracks during simulaiton and output
	std::map<int, int>*
	    m_particleIDMap;  ///< Map to store the trackID to the position in the particle Vector for later retrieval. This is needed because the
			      ///< trackID is not the same as the position in the vector. `Track particle = particleVector[particleIDMap[trackID]]`.

	std::string m_rootOutputFileName;  ///< Name of the ROOT output file, set by the user using the CLI when calling the program

	bool initializedFile = false;  ///< Flag to check if the file is initialized, the beginOfRun action is called twice per event, for some reason. I
				       ///< currently think it is called by the worker and master thread.

	bool writtenToFile = false;  ///< This is a flag, that prevents the wiriting of duplicate informaiton. THe endOfRun action is called twice
				     ///< per event, for some reason. I currently think it is called by the worker and master thread.

	int m_numberParticleVectors = 17;
	int m_numberParticlesPerVector = 100000;

	std::vector<std::vector<Track>>* m_particleVectorOutput = nullptr;

	bool saveDetector = false;
	int m_vertexMode;				  ///< Information about the vertex mode. Either from neut or userdefined
	int m_vertexNParticles;				  ///< Number of particles in the vertex
	std::vector<XYZVector>* m_vertexPositions;	  ///< Vector of vertex positions
	std::vector<XYZVector>* m_vertexMomenta;	  ///< Vector of vertex momenta
	std::vector<double>* m_vertexTimes;		  ///< Vector of vertex times
	std::vector<double>* m_vertexEnergy;		  ///< Vector of vertex energies
	std::vector<double>* m_vertexKinEnergy; 	  ///< Vector of vertex kinetic energies
	std::vector<int>* m_vertexTrackID;		  ///< Vector of vertex track IDs
	std::vector<int>* m_vertexDecayMode;		  ///< Vector of vertex decay modes
	std::vector<int>* m_vertexPDG;			  ///< Vector of vertex PDG codes
	double m_vertexNuEnergy;			  ///< Energy of the neutrino if the vertex is from a neutrino, otherwise userdefined


	XYZVector m_size1;
	XYZVector m_size2;
	std::string m_material1;
	std::string m_material2;
	int m_nrep;

	long fTcalEvent_number = 0;
	TcalEvent *fTcalEvent;

	XYZVector primary_vertex_position;

	struct PrimaryInfo {
		G4int parentID;
		G4int primaryID;
	};
	std::map<int, PrimaryInfo> fPrimaries;      // the primary of a given secondary

    public:
	/**
	 * @brief Construct a new Particle Manager object
	 *
	 * @param number Currently not used
	 * @param rootOutputFileName Name of the ROOT output file, set by the user using the CLI when calling the program
	 */
	ParticleManager(int number, std::string rootOutputFileName);
	~ParticleManager();  ///< Destructor

	/**
	 * @brief Process a hit from a particle
	 * @details This functionis called by the Sensitive Detector whenever a particle hit is regiersted inside the scintillator. It sets the
	 * identifier based on information of the step. Check the TrackerSD class for more informaiton.
	 * @param trackID ID of the particle
	 * @param position Hit position
	 * @param direction Hit direction
	 * @param time Hit time
	 * @param energydeposit Energy deposited by the particle in this step
	 * @param process The process that caused the hit
	 * @param parentID ID of the parent particle
	 * @param pdg PDG code of the particle
	 * @param VolumeName Name of the volume where the hit occurred
	 * @param CopyNumber The replica number of the volume
	 */
	void processParticleHit(int trackID, XYZVector const& position, XYZVector const& direction, double const& time, double const& energydeposit,
				Geant4Process const& process, int const& parentID, int const& pdg, std::string const& VolumeName, G4int CopyNumber);

	/**
	 * @brief Get the Information about the primary Vertex from the Primary Generator Action
	 */
	void setVertexInformation(XYZVector vtxpos) { primary_vertex_position = vtxpos;};
	#if 0
	void setVertexInformation(int Mode, int NParticles, std::vector<XYZVector> const& VertexPositions,
				  std::vector<XYZVector> const& VertexMomenta, std::vector<double> const& VertexTimes,
				  std::vector<double> const& VertexEnergies, std::vector<double> const& VertexKinEnergy,
				  std::vector<int> const& VertexTrackID, std::vector<int> const& VertexDecayMode,
				  std::vector<int> const& VertexPDG, double const& VertexNuEnergy);
	#endif

	/**
	 * @brief Save the detetor properties in the ROOT file
	 * @details This function is called by the Detector Construction to save the detector properties in the ROOT file. This is needed to
	 * reconstruct the detector response in the analysis.
	 * @param SizeX Size of the detector in X
	 * @param SizeY Size of the detector in Y
	 * @param SizeZ Size of the detector in Z
	 * @param LightYield Light yield of the detector
	 * @param ScintillationTime Scintillation time of the detector
	 * @param RefractiveIndex Refractive index of the detector
	 * @param AbsorptionLength Absorption length of the detector
	 * @param EmissionSpectrum Emission spectrum of the detector
	 * @param PhotonEnergy Photon energy of the detector (eV)
	 * @param save Determine if the detector properties should be saved in the ROOT file, default is no
	 */
	void setDetectorInformation(std::string material1, XYZVector size1, std::string material2, XYZVector size2, int NRep);

	/**
	 * @brief Cleans up the particle maps/ vectors
	 */
	void beginOfEvent();
	/**
	 * @brief Writes particle maps/ vectors to the ROOT tree
	 */
	void endOfEvent(G4Event const* event);
    
	//////////////////////////////////////////////////////////
	///////////// RunAction functions
	//////////////////////////////////////////////////////////

	/**
	 * @brief Makes sure the manager is ready and the ROOT file is open
	 */
	void beginOfRun();

	/**
	 * @brief Writes the trees to the root file and saves the file
	 */
	void endOfRun();

	/**
	 * @brief Initializes the output trees
	 */
	void InitializeOutput();

	G4int FindPrimaryParticle(G4int trackID);
	void RecordTrack(const G4Track* track);

};


class TrackingAction : public G4UserTrackingAction {
public:
    TrackingAction(ParticleManager* eventAction) : G4UserTrackingAction(), eventAction(eventAction) {}
    virtual ~TrackingAction() {}

    virtual void PreUserTrackingAction(const G4Track* track) override {
        eventAction->RecordTrack(track);
    }

private:
    ParticleManager* eventAction;
};

#endif