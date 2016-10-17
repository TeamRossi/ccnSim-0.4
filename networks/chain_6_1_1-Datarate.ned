package networks;

network chain_6_1_1_network extends base_network{


    parameters:
        //Number of ccn nodes
    	n = 6;

	//Number of repositories
	node_repos = "5";
	num_repos = 1;
	replicas = 1;

	//Number of clients 
	num_clients = 1;
	node_clients = "0";

    types:
        channel C extends ned.DelayChannel{
            datarate = 100Mbps;
            delay = 1ms;
        }

connections:
    for  i = 0..n-2{
	node[i].face++ <--> C <--> node[i+1].face++;
    }
}
