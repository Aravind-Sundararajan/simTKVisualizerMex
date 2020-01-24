% get handle to mex object
opensimroot = 'C:\opensim_install\';
addpath([opensimroot 'bin'], [opensimroot 'sdk\lib']);
javaaddpath([opensimroot 'bin'], [opensimroot 'sdk\lib']);
% Set Windows System path to include OpenSim libraries
setenv('PATH', [[opensimroot 'bin'] ';' [opensimroot 'sdk\lib'] ';' getenv('PATH')]);
import org.opensim.modeling.*

modelPath = 'C:\opensim_install\Resources\Models\Gait10dof18musc\'; %get path to the models
modelName = 'subject01.osim';

myModel = org.opensim.modeling.Model([modelPath,modelName]);
myModel.setUseVisualizer(true);
state = myModel.initSystem();
myModel.getVisualizer().show(state);
osimviz = myModel.updVisualizer();
sviz =osimviz.updSimbodyVisualizer();
% sviz.setShowSimTime(true);
% sviz.setShowFrameNumber(true);
% sviz.setBackgroundTypeByInt(2);
%
osimvizCPtr = uint64(osimviz.getCPtr(osimviz)); 
extvis = extend_visualizer(osimvizCPtr);




% releases the object instance
extvis.delete() 
clear extvis;
