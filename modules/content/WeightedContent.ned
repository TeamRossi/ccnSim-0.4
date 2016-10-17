package modules.content;

//<aa>
simple WeightedContentDistribution extends content_distribution{

    parameters:
		@class(WeightedContentDistribution);
		string catalog_split; 	// weights of the different repositories, in the form x_y_z. Every content is randomly
								// assigned to the different repositories. With probability x it will be placed in 
								// repository 0, with probability y in repository 1 and with probability z in repository 2
								// see [icn14]

		bool replication_admitted = default(false);
		double priceratio; // See [icn14]
		double kappa; // costaware exponent. See [icn14]
}
//</aa>


// References
// [icn14] A. Araldo, D. Rossi, F. Martignon, “Design and Evaluation of Cost-aware Information Centric Routers”, to appear in ACM Conference on Information-Centric Networking (ICN), Paris, 2014 
