package networks;

network tandem_net_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 2;

	//Number of repositories
	node_repos = "1";
	num_repos = 1;
	replicas = 1;

	//Number of clients 
	num_clients = 1;
	node_clients = "0";

connections:
    for  i = 0..n-2{
	node[i].face++ <--> {delay = 1ms;} <--> node[i+1].face++;
    }
}
