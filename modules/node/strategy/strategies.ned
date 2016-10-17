package modules.node.strategy;

moduleinterface strategy
{
    gates:
    	inout strategy_port;
}

simple strategy_layer like strategy
{
    parameters:
        double fail_time;
        double fail_duration;
        double fail_transient;
        bool fail_scenario = default(false);
        
    	@display("i=block/buffer2;is=l");
	string routing_file=default("");
    gates:
	inout strategy_port;
}

simple spr extends MonopathStrategyLayer
{
    @class(spr);
}

simple random_repository extends MonopathStrategyLayer
{
    @class(random_repository);
}

simple parallel_repository extends MonopathStrategyLayer
{
    @class(parallel_repository);
}


simple nrr1 extends MonopathStrategyLayer
{
    parameters:
	int cut_off = default(1);
	int TTL1 = default(1000);
    @class(nrr1);
}

simple nrr extends MonopathStrategyLayer{
    parameters:
	int TTL2 = default(1000);
    @class(nrr);
}

simple MultipathStrategyLayer extends strategy_layer{
    parameters:
    @class(MultipathStrategyLayer);
}

simple MonopathStrategyLayer extends strategy_layer{
    parameters:
    @class(MultipathStrategyLayer);
}

simple ProbabilisticSplitStrategy extends MultipathStrategyLayer{
	parameters:
    	@class(ProbabilisticSplitStrategy);
		string split_factors;
}
