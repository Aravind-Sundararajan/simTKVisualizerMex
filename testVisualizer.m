clear all
format long
opensimroot = 'C:\opensim_install\';
addpath([opensimroot 'bin'], [opensimroot 'sdk\lib']);
javaaddpath([opensimroot 'bin'], [opensimroot 'sdk\lib']);
% Set Windows System path to include OpenSim libraries
setenv('PATH', [[opensimroot 'bin'] ';' [opensimroot 'sdk\lib'] ';' getenv('PATH')]);
import org.opensim.modeling.*
geometryPath='C:\opensim_install\Geometry';
ModelVisualizer.addDirToGeometrySearchPaths(geometryPath);
modelPath = 'C:\opensim_install\Resources\Models\Gait10dof18musc\'; %get path to the models
modelName = 'subject01.osim';

myModel = org.opensim.modeling.Model([modelPath,modelName]);
myModel.setUseVisualizer(true);
state = myModel.initSystem();
myModel.getVisualizer().show(state);
osimviz = myModel.updVisualizer();
osimvizCPtr = uint64(osimviz.getCPtr(osimviz));%we have to pass this
extvis = extend_visualizer(osimvizCPtr);
% releases the object instance
pause(1)
sviz = myModel.getVisualizer.getSimbodyVisualizer();
sviz.setBackgroundColor(org.opensim.modeling.Vec3(255));
pause(1)
extvis.delete();
clear('extvis');
sviz.setBackgroundColor(org.opensim.modeling.Vec3(0,255,0));
sviz.shutdown();
