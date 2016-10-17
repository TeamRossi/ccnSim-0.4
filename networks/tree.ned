package networks;

network tree_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 15;

	//Number of repositories
	node_repos = "0";
	num_repos = 1;
	replicas = 1;

        //Number of clients
	num_clients = 8;
	node_clients = "7,8,9,10,11,12,13,14";



connections allowunconnected:



//Tree core
node[0].face++ <--> { delay = 1ms; } <-->node[1].face++;
node[0].face++ <--> { delay = 1ms; } <-->node[2].face++;
node[1].face++ <--> { delay = 1ms; } <-->node[3].face++;
node[1].face++ <--> { delay = 1ms; } <-->node[4].face++;
node[2].face++ <--> { delay = 1ms; } <-->node[5].face++;
node[2].face++ <--> { delay = 1ms; } <-->node[6].face++;
node[3].face++ <--> { delay = 1ms; } <-->node[7].face++;
node[3].face++ <--> { delay = 1ms; } <-->node[8].face++;
node[4].face++ <--> { delay = 1ms; } <-->node[9].face++;
node[4].face++ <--> { delay = 1ms; } <-->node[10].face++;
node[5].face++ <--> { delay = 1ms; } <-->node[11].face++;
node[5].face++ <--> { delay = 1ms; } <-->node[12].face++;
node[6].face++ <--> { delay = 1ms; } <-->node[13].face++;
node[6].face++ <--> { delay = 1ms; } <-->node[14].face++;
}
