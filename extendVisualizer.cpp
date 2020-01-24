#include <Simbody.h>
#include <OpenSim.h>
#include "C:/Users/asundar4/Documents/libs/include/mexplus.h"
#include <vector>
using namespace std;
using namespace mexplus;
using namespace OpenSim;
using namespace SimTK;
class extendVisualizer {
public:
	extendVisualizer(const int cPtr){
        
		mexPrintf("Calling constructor.\n");
        m_visualizer = (SimTK::Visualizer *)cPtr;
        mexPrintf("Ground Height:'%s'.\n", m_visualizer->getMode());
	}
	virtual ~extendVisualizer(){
		mexPrintf("Calling destructor.\n");
	}
private:
	ReferencePtr<SimTK::Visualizer> m_visualizer;
};
template class mexplus::Session<extendVisualizer>;
namespace {
	MEX_DEFINE(new) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 1);
		output.set(0, Session<extendVisualizer>::create(new extendVisualizer(input.get<int>(0))));
	}
	MEX_DEFINE(delete) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 0);
		Session<extendVisualizer>::destroy(input.get(0));
	}
} //namespace
MEX_DISPATCH