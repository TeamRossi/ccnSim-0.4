package networks;

network single_cache_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 1;
	
	//Number of repositories
	node_repos = "0";
	num_repos = 1;
	replicas = 1;
        
	//Number of clients
	num_clients = 1;
	node_clients = "0";


connections allowunconnected:
}
