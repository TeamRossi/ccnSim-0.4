package networks;
network tree_6_2_network extends base_network{
    parameters:
        //Number of ccn nodes
    	n = 63;
	//Number of repositories
	node_repos = "0";
	num_repos = 1;
	replicas = 1;
        //Number of clients
	num_clients = 32;
	node_clients = "31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62";
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
	node[7].face++ <--> { delay = 1ms; } <-->node[15].face++;
	node[7].face++ <--> { delay = 1ms; } <-->node[16].face++;
	node[8].face++ <--> { delay = 1ms; } <-->node[17].face++;
	node[8].face++ <--> { delay = 1ms; } <-->node[18].face++;
	node[9].face++ <--> { delay = 1ms; } <-->node[19].face++;
	node[9].face++ <--> { delay = 1ms; } <-->node[20].face++;
	node[10].face++ <--> { delay = 1ms; } <-->node[21].face++;
	node[10].face++ <--> { delay = 1ms; } <-->node[22].face++;
	node[11].face++ <--> { delay = 1ms; } <-->node[23].face++;
	node[11].face++ <--> { delay = 1ms; } <-->node[24].face++;
	node[12].face++ <--> { delay = 1ms; } <-->node[25].face++;
	node[12].face++ <--> { delay = 1ms; } <-->node[26].face++;
	node[13].face++ <--> { delay = 1ms; } <-->node[27].face++;
	node[13].face++ <--> { delay = 1ms; } <-->node[28].face++;
	node[14].face++ <--> { delay = 1ms; } <-->node[29].face++;
	node[14].face++ <--> { delay = 1ms; } <-->node[30].face++;
	node[15].face++ <--> { delay = 1ms; } <-->node[31].face++;
	node[15].face++ <--> { delay = 1ms; } <-->node[32].face++;
	node[16].face++ <--> { delay = 1ms; } <-->node[33].face++;
	node[16].face++ <--> { delay = 1ms; } <-->node[34].face++;
	node[17].face++ <--> { delay = 1ms; } <-->node[35].face++;
	node[17].face++ <--> { delay = 1ms; } <-->node[36].face++;
	node[18].face++ <--> { delay = 1ms; } <-->node[37].face++;
	node[18].face++ <--> { delay = 1ms; } <-->node[38].face++;
	node[19].face++ <--> { delay = 1ms; } <-->node[39].face++;
	node[19].face++ <--> { delay = 1ms; } <-->node[40].face++;
	node[20].face++ <--> { delay = 1ms; } <-->node[41].face++;
	node[20].face++ <--> { delay = 1ms; } <-->node[42].face++;
	node[21].face++ <--> { delay = 1ms; } <-->node[43].face++;
	node[21].face++ <--> { delay = 1ms; } <-->node[44].face++;
	node[22].face++ <--> { delay = 1ms; } <-->node[45].face++;
	node[22].face++ <--> { delay = 1ms; } <-->node[46].face++;
	node[23].face++ <--> { delay = 1ms; } <-->node[47].face++;
	node[23].face++ <--> { delay = 1ms; } <-->node[48].face++;
	node[24].face++ <--> { delay = 1ms; } <-->node[49].face++;
	node[24].face++ <--> { delay = 1ms; } <-->node[50].face++;
	node[25].face++ <--> { delay = 1ms; } <-->node[51].face++;
	node[25].face++ <--> { delay = 1ms; } <-->node[52].face++;
	node[26].face++ <--> { delay = 1ms; } <-->node[53].face++;
	node[26].face++ <--> { delay = 1ms; } <-->node[54].face++;
	node[27].face++ <--> { delay = 1ms; } <-->node[55].face++;
	node[27].face++ <--> { delay = 1ms; } <-->node[56].face++;
	node[28].face++ <--> { delay = 1ms; } <-->node[57].face++;
	node[28].face++ <--> { delay = 1ms; } <-->node[58].face++;
	node[29].face++ <--> { delay = 1ms; } <-->node[59].face++;
	node[29].face++ <--> { delay = 1ms; } <-->node[60].face++;
	node[30].face++ <--> { delay = 1ms; } <-->node[61].face++;
	node[30].face++ <--> { delay = 1ms; } <-->node[62].face++;
//Uncle-nephew redundancy
//Brothers redundancy
}
