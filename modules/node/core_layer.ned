package modules.node;

simple core_layer{
    parameters:
    	@display("i=block/cogwheel;is=l");
		double RTT = default(0.1);

		//<aa> if true, if an interest comes and there already is an entry in the PIT
		// for the same content, the new interest will not be forwarded. In false, The 
		// interest will be forwarded in any case
		bool interest_aggregation = default(true);

		// If true, the hop count of interests and data passing through this node
		// will not be incremented 
		bool transparent_to_hops = default(false);
		//</aa>

		// *** Link Load Evaluation ***
		bool llEval = default(false);
		double maxInterval = default(1.0);
		double datarate = default(1000000); // 1Mbps


    gates:
    	inout strategy_port;
	inout client_port;
	inout cache_port;
	inout face[];
}


