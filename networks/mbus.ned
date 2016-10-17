package networks;

network mbus_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 8;

	//Number of repositories
	node_repos = "4,7";//n-1
	num_repos = 2;
	replicas  = 2;

	//Number of clients 
	num_clients = 1;
	node_clients = "0";


connections allowunconnected:

    for  i = 1..3{
	node[i].face++ <--> {delay = 1ms;} <--> node[i+1].face++;
    }

    for  i = 1..2{
	node[i+4].face++ <--> {delay = 1ms;} <--> node[i+5].face++;
    }

    node[0].face++ <--> {delay = 1ms;} <--> node[1].face++;
    node[0].face++ <--> {delay = 1ms;} <--> node[5].face++;

}
