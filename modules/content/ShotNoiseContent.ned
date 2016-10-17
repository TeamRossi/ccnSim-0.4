package modules.content;

simple ShotNoiseContentDistribution extends content_distribution{

    parameters:
		@class(ShotNoiseContentDistribution);
		
		string shot_noise_file;
		int toff_mult_factor;
		double num_tot_req; 	
}
