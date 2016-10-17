package networks;

network chain_6_6_2_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 6;

	//Number of repositories
	node_repos = "2, 5";
	num_repos = 2;
	replicas = 1;

	//Number of clients 
	num_clients = 6;
	node_clients = "";

connections:
    for  i = 0..n-2{
	node[i].face++ <--> {delay = 1ms;} <--> node[i+1].face++;
    }
}
