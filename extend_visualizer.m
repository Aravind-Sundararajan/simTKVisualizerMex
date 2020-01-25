classdef extend_visualizer < handle
	properties (Access = private)
		id_ % ID of the session
	end
	methods
		function this = extend_visualizer(cPtr)
			assert(isinteger(cPtr))
			this.id_ = extendVisualizer('new', cPtr);
		end
		
		function delete(this)
			extendVisualizer('delete', this.id_);
		end 
	end
end