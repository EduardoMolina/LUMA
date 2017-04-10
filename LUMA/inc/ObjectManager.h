/*
 * --------------------------------------------------------------
 *
 * ------ Lattice Boltzmann @ The University of Manchester ------
 *
 * -------------------------- L-U-M-A ---------------------------
 *
 *  Copyright (C) 2015, 2016
 *  E-mail contact: info@luma.manchester.ac.uk
 *
 * This software is for academic use only and not available for
 * distribution without written consent.
 *
 */
#ifndef OBJMAN_H
#define OBJMAN_H

#include "stdafx.h"
#include "IVector.h"
#include "IBInfo.h"
#include "IBMarker.h"
#include "IBBody.h"
#include "BFLBody.h"

class PCpts;
class GridObj;

/// \brief	Object Manager class.
///
///			Class to manage all objects in the domain from creation through 
///			manipulation to destruction.
class ObjectManager
{

	// Make Grid a friend so boundary conditions can access the body data
	friend class GridObj;

	/// \brief	Nested Geometry data structure class.
	///
	///			Stores information about a body in a coherent manner but not 
	///			intended to be instantiated outside the Object Manager.
	class GeomPacked
	{
	public:

		GeomPacked();
		GeomPacked(
			eObjectType objtype, int bodyID, std::string fileName, 
			int on_grid_lev, int on_grid_reg,
			double body_start_x, double body_start_y, double body_centre_z, 
			double body_length, eCartesianDirection scale_direction, 
			eMoveableType moveProperty, bool clamped
			);
		~GeomPacked();
		
	
		// Members
		eObjectType objtype;
		int bodyID;
		std::string fileName;
		int on_grid_lev;
		int on_grid_reg;
		double body_start_x;
		double body_start_y;
		double body_centre_z;
		double body_length;
		eCartesianDirection scale_direction;
		eMoveableType moveProperty;
		bool clamped;

	};

	/* Members */

private:

	// Private file stream for debugging momentum exchange
	std::ofstream debugstream;

	// Bounce-back object fields
	double bbbForceOnObjectX = 0.0;			///< Instantaneous X-direction force on BB bodies in domain
	double bbbForceOnObjectY = 0.0;			///< Instantaneous Y-direction force on BB bodies in domain
	double bbbForceOnObjectZ = 0.0;			///< Instantaneous Z-direction force on BB bodies in domain
	int bbbOnGridLevel = -1;				///< Grid level on which the BB body resides
	int bbbOnGridReg = -1;					///< Grid region on which the BB body resides

	// Objects (could be stored in a single Body array if we use pointers)
	std::vector<IBBody> iBody;				///< Array of immersed boundary bodies
	std::vector<BFLBody> pBody;				///< Array of BFL bodies

	/// Pointer to grid hierarchy
	GridObj* _Grids;

	/// Pre-stream distribution functions for applying BFL BCs
	DEPRECATED IVector<double> f_prestream;

	/// Pointer to self
	static ObjectManager* me;


	/* Methods */

private:
	ObjectManager(void);		///< Private constructor
	~ObjectManager(void);		///< Private destructor
	/// Overloaded constructor to set pointer to grid hierarchy
	ObjectManager(GridObj* _Grids);		

public:
	// Singleton design
	static ObjectManager *getInstance();			///< Get instance method
	static void destroyInstance();					///< Destroy instance method
	static ObjectManager *getInstance(GridObj* g);	///< Overloaded get instance passing in pointer to grid hierarchy

	// IBM methods //
	void ibm_apply();						// Apply interpolate, compute and spread operations for all bodies.
	void ibm_initialise();					// Initialise a built immersed body with support.
	double ibm_deltaKernel(double rad, double dilation);	// Evaluate kernel (delta function approximation).
	void ibm_interpol(int ib);				// Interpolation of velocity field onto markers of ib-th body.
	void ibm_spread(int ib);				// Spreading of restoring force from ib-th body.
	void ibm_findSupport(int ib, int m);	// Populates support information for the m-th marker of ib-th body.
	void ibm_initialiseSupport(int ib, int m, 
		int s, double estimated_position[]);		// Initialises data associated with the support points.
	void ibm_computeForce(int ib);			// Compute restorative force at each marker in ib-th body.
	double ibm_findEpsilon(int ib);			// Method to find epsilon weighting parameter for ib-th body.
	void ibm_moveBodies();					// Update all IBBody positions and support.
	double ibm_bicgstab(std::vector< std::vector<double> >& Amatrix,
		std::vector<double>& bVector, std::vector<double>& epsilon,
						   double tolerance, int maxiterations);	// Biconjugate gradient stablised method for solving asymmetric 
																	// linear system required by finding epsilon

	// Flexible body methods
	void ibm_jacowire(int ib);					// Computes the tension and position of a 2D inextensible, flexible filament.
	void ibm_positionUpdate(int ib);			// Updates the position of movable body markers.
	void ibm_positionUpdateGroup(int group);	// Updates the positions of movable bodies in a group.
	// Methods to solve the Jacobian system associated with Jacowire
	void ibm_banbks(double **a, long n, int m1, int m2, double **al,
		unsigned long indx[], double b[]);
	void ibm_bandec(double **a, long n, int m1, int m2, double **al,
		unsigned long indx[], double *d);

	// Bounceback Body Methods
	void addBouncebackObject(GridObj *g, GeomPacked *geom, PCpts *_PCpts);	// Method to add a BBB from the cloud reader.
	void computeLiftDrag(int i, int j, int k, GridObj *g);			// Compute force using Momentum Exchange for BBB on supplied grid.
	void computeLiftDrag(int v, int id, GridObj *g, int markerID);	// Compute force using Momentum Exchange for BFL on supplied grid.
	void resetMomexBodyForces(GridObj * grid);						// Reset the force stores for Momentum Exchange

	// IO methods //
	void io_vtkBodyWriter(int tval);				// VTK body writer wrapper
	void io_writeBodyPosition(int timestep);		// Write out IBBody positions at specified timestep to text files
	void io_writeLiftDrag(int timestep);			// Write out IBBody lift and drag at specified timestep
	void io_restart(eIOFlag IO_flag, int level);	// Restart read and write for IBBodies given grid level
	void io_readInCloud(PCpts*& _PCpts, GeomPacked *geom);	// Method to read in Point Cloud data
	void io_writeForcesOnObjects(double tval);		// Method to write object forces to a csv file
	void io_readInGeomConfig();						// Read in geometry configuration file

	// Debug
	void toggleDebugStream(GridObj *g);		// Method to open/close a debugging file
};

#endif
