package networks;

network chain_6_1_2_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 6;

	//Number of repositories
	node_repos = "2, 5";
	num_repos = 2;
	replicas = 1;

	//Number of clients 
	num_clients = 1;
	node_clients = "0";

connections:
    for  i = 0..n-2{
	node[i].face++ <--> {delay = 1ms;} <--> node[i+1].face++;
    }
}
