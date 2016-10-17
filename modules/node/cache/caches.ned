package modules.node.cache;

moduleinterface cache
{
    parameters:
	int C;
	int NC;
    gates:
	inout cache_port;
}

simple base_cache like cache
{
    parameters:
    	@display("i=block/buffer2;is=l");

	string DS = default("lce");
	int C = default (100);
	int NC = default (100);
	string tc_file = default("./tc_single_cache.txt");
        string tc_name_file = default("./tc_name_single_cache.txt");
    gates:
	inout cache_port;
}



simple random_cache extends base_cache{
    @class(random_cache);
}

simple lru_cache extends base_cache{
    @class(lru_cache);
}

simple two_cache extends base_cache{
    @class(two_cache);
}

simple fifo_cache extends base_cache{
    @class(fifo_cache);
}

simple ttl_cache extends base_cache{
    @class(ttl_cache);
}
