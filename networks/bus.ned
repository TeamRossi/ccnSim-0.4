package networks;

network bus_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 10;

	//Number of repositories
	node_repos = "9";
	num_repos = 1;
	replicas = 1;

	//Number of clients 
	num_clients = 10;
	node_clients = "";

connections:
    for  i = 0..n-2{
	node[i].face++ <--> {delay = 1ms;} <--> node[i+1].face++;
    }
}
