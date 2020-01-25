#include <Simbody.h>
#include <OpenSim.h>
#include "C:\Users\aravi\Documents\MATLAB\mexplus-master\include\mexplus.h"
#include <vector>
using namespace std;
using namespace mexplus;
using namespace OpenSim;
using namespace SimTK;
class extendVisualizer {
public:
	extendVisualizer(const __int64 cPtr){        
  mexPrintf("Calling constructor.\n");
  m_visualizer = (ModelVisualizer *)cPtr;
  simbodyVisualizer = &(m_visualizer->updSimbodyVisualizer());
  simbodyVisualizer->setShowSimTime(false);
  simbodyVisualizer->setShowFrameNumber(false);
  simbodyVisualizer->setBackgroundType(SimTK::Visualizer::BackgroundType(2));
  simbodyVisualizer->setBackgroundColor(SimTK::Vec3(0,0,0));
  mexPrintf("Model:'%s'.\n", (m_visualizer->getModel()).getName());
	}
	virtual ~extendVisualizer(){
		mexPrintf("Calling destructor.\n");
  delete m_visualizer;
  delete simbodyVisualizer;
	}
private:
	ModelVisualizer* m_visualizer = NULL;
 SimTK::Visualizer* simbodyVisualizer = NULL;
};
template class mexplus::Session<extendVisualizer>;
namespace {
	MEX_DEFINE(new) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 1);
		output.set(0, Session<extendVisualizer>::create(new extendVisualizer(input.get<__int64>(0))));
	}
	MEX_DEFINE(delete) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
		InputArguments input(nrhs, prhs, 1);
		OutputArguments output(nlhs, plhs, 0);
		Session<extendVisualizer>::destroy(input.get(0));
	}
} //namespace
MEX_DISPATCH