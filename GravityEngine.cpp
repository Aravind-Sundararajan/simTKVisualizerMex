#include <Simbody.h>
#include <OpenSim/Simulation/Model/Model.h>
#include "C:/libs/mex/mexplus.h"
#include <vector>

using namespace std;
using namespace mexplus;
using namespace OpenSim;
using namespace SimTK;


class GravityEngine
{
public:
	GravityEngine(){ 
       mexPrintf("Calling constructor. Beep boop beep... \n"); 
    }
	~GravityEngine(){ 
        mexPrintf("Calling destructor. 3.. 2... 1... Goodbye. \n"); 
    }

	void setup(const string& fileName)
	{
		mexPrintf("Calling setup. \n");

		//load model
		m_model = new Model(fileName);

		//Build system and initialize state
		m_model->finalizeFromProperties();
		m_model->buildSystem();
		m_state = m_model->initializeState();

		return;
	};

	void updateState(const vector<double>& coords_in)
	{
		
		if (m_model.empty()) {
			mexPrintf("Error! 0_o Error! m_model pointer was empty. Call setup() first! \n");
			return;
		}

		// reference to writable instance of state
		m_state = m_model->updWorkingState();

		int nCoords = m_model->getNumCoordinates();
		if (coords_in.size() != nCoords) {
			mexPrintf("The number of coordinates does not agree %d != #d \n", coords_in.size(), nCoords);
		}

		for (int index = 0; index < nCoords; index++) {
			if (((m_model->updCoordinateSet()[index].isDependent(*m_state)) == 0) && (m_model->updCoordinateSet()[index].getLocked(*m_state)) == 0) {
				//mexPrintf(" value at index %d changing to %f \n", index, coords_in[index]);
				m_model->updCoordinateSet()[index].setValue(*m_state, coords_in[index], 1);//final argument sets enforceConstraints to true
			}
		}

		//realize model to position stage based on updated generalized coordinate values in m_state
		m_model->realizeVelocity(*m_state);
	};

	vector<double> getStateCoords()
	{
		if (m_model.empty()) {
			mexPrintf("Error! 0_o Error! m_model pointer was empty. Call Setup() first!\n");
			vector<double> vec{ -1.0 };
			return vec;
		}

		// reference to writable instance of state
		m_state = m_model->updWorkingState();
		m_model->realizeVelocity(*m_state);

		SimTK::Vector osimVector = m_state->getQ();

		return osimVecToStdVec(osimVector);

	};

	vector<double> calcGravForces() {
		mexPrintf("Calling calcGravForces\n");

		if ((m_model.empty()) || (m_state.empty())) {
			mexPrintf("Error! 0_o Error! m_model or m_state pointer was empty. Call setup() first! \n");
				vector<double> vec{ -1.0 };
				return vec;
		}

		SimTK::Vector forceVector;
		m_model->getMatterSubsystem().multiplyBySystemJacobianTranspose(*m_state, m_model->getGravityForce().getBodyForces(*m_state), forceVector);

		return osimVecToStdVec(forceVector);

	};


private:
	ReferencePtr<State> m_state;
	ReferencePtr<Model> m_model;

	vector<double> osimVecToStdVec(const SimTK::Vector& osimVec)
	{
		//assign SimTK::Vector contents to std::vector output by index
		vector<double> stdVec;

		for (int index = 0; index < osimVec.size(); index++) {
			stdVec.push_back(osimVec(index));
		};
		
		return stdVec;
	};

};


template class mexplus::Session<GravityEngine>;

namespace
{

	MEX_DEFINE(new) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 0);
		OutputArguments output(nlhs, plhs, 1);

		output.set(0, Session<GravityEngine>::create(new GravityEngine()));
	}

	MEX_DEFINE(delete) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 0);

		Session<GravityEngine>::destroy(input.get(0));
	}

	MEX_DEFINE(setup) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 2);
		OutputArguments output(nlhs, plhs, 0);

		GravityEngine* engine = Session<GravityEngine>::get(input.get(0));
		engine->setup(input.get< string >(1));
	}

	MEX_DEFINE(updateState) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 2);
		OutputArguments output(nlhs, plhs, 0);

		GravityEngine* engine = Session<GravityEngine>::get(input.get(0));
		engine->updateState(input.get< vector<double> >(1));
	}

	MEX_DEFINE(getStateCoords) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 1);

		GravityEngine* engine = Session<GravityEngine>::get(input.get(0));
		output.set< vector<double> >(0, engine->getStateCoords());
	}

	MEX_DEFINE(calcGravForces) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
	{
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 1);

		GravityEngine* engine = Session<GravityEngine>::get(input.get(0));
		output.set< vector<double> >(0, engine->calcGravForces());
	}


} //namespace

MEX_DISPATCH 