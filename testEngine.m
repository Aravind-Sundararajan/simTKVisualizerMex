% get handle to mex object
opensimroot = 'C:\opensim 4.0\'; %create a char array that has the opensim path toplevel directory
addpath([opensimroot 'bin'], [opensimroot 'sdk\lib']); %add the opensim path to the
javaaddpath([opensimroot 'bin'], [opensimroot 'sdk\lib']); %add opensimroot bin and the java path to MATLAB's dynamic path path
setenv('PATH', [[opensimroot 'bin'] ';' [opensimroot 'sdk\lib'] ';' getenv('PATH')]);% Set Windows System path to include OpenSim libraries
import org.opensim.modeling.* %import opensim api library

osimModel = Model([modelPath,modelName]);
state = osimModel.initSystem();

nY = state.getNY;
obj = gravity_engine();
modelPath = 'models\Gait10dof18musc\'; %get path to the models
modelName = 'subject01.osim';
qVec = zeros(1,nY);
%load and initialize your model
obj.setup([modelPath,modelName]);

%update the independent coordinates
obj.updateState(qVec);

%get the current coordinates (joint angles)
qVecNew = obj.getStateCoords();

%get the vector of generalized forces/torques due to gravity in ground frame
gravF = obj.calcGravForces();

% releases the object instance
obj.delete 
clear obj;
disp(gravF)
