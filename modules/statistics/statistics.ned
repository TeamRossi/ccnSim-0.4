package modules.statistics;
simple statistics{
    parameters:
		double ts = default(0.1);
		double window = default (60);
		double steady = default(1000);
		double partial_n = default(10);
		string start_mode = default("cold");
		string fill_mode = default("naive");
		double variance_threshold = default(0.05);
		double lambda = default(1);
		double cvThr = default(0.005);
                double consThr = default(0.1);
		int sim_model = default(0);  // Transient by simulation is the default.

		int downsize = default(1);

		int CEXPL = default(3);
		double ttl = default(30);
		bool onlyModel = default(true);
		@display("i=block/table2;is=l");

}
