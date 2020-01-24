classdef extend_visualizer < handle
	properties (Access = private)
		id_ % ID of the session
	end
	methods
		function this = extend_visualizer(cPtr)
			%GravityEngine Create a new
			assert(isinteger(cPtr))
			this.id_ = extendVisualizer('new', cPtr);
		end
		
		function delete(this)
			%delete Destructor
			extendVisualizer('delete', this.id_);
		end 
	end
end