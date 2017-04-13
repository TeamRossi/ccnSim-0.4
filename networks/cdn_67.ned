package networks;

network cdn_67_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 67;

		//Number of repositories
		node_repos = "0,2,5,7,10";
		num_repos = 5;
		replicas = 1;

        //Number of clients
		num_clients = 32;
		node_clients = "17,18,19,20,21,22,23,24,31,32,33,34,35,36,37,38,45,46,47,48,49,50,51,52,59,60,61,62,63,64,65,66";

connections allowunconnected:
	// Abilene
	node[1].face++ <--> { delay = 1ms; } <--> node[0].face++;
	node[10].face++ <--> { delay = 1ms; } <--> node[0].face++;
	node[10].face++ <--> { delay = 1ms; } <--> node[1].face++;
	node[10].face++ <--> { delay = 1ms; } <--> node[9].face++;
	node[2].face++ <--> { delay = 1ms; } <--> node[1].face++;
	node[3].face++ <--> { delay = 1ms; } <--> node[2].face++;
	node[4].face++ <--> { delay = 1ms; } <--> node[3].face++;
	node[5].face++ <--> { delay = 1ms; } <--> node[4].face++;
	node[6].face++ <--> { delay = 1ms; } <--> node[5].face++;
	node[7].face++ <--> { delay = 1ms; } <--> node[3].face++;
	node[7].face++ <--> { delay = 1ms; } <--> node[6].face++;
	node[8].face++ <--> { delay = 1ms; } <--> node[2].face++;
	node[8].face++ <--> { delay = 1ms; } <--> node[7].face++;
	node[9].face++ <--> { delay = 1ms; } <--> node[8].face++;
	// Tree rooted at '1'
	node[1].face++ <--> { delay = 1ms; } <-->node[11].face++;
	node[1].face++ <--> { delay = 1ms; } <-->node[12].face++;
	node[11].face++ <--> { delay = 1ms; } <-->node[13].face++;
	node[11].face++ <--> { delay = 1ms; } <-->node[14].face++;
	node[12].face++ <--> { delay = 1ms; } <-->node[15].face++;
	node[12].face++ <--> { delay = 1ms; } <-->node[16].face++;
	node[13].face++ <--> { delay = 1ms; } <-->node[17].face++;
	node[13].face++ <--> { delay = 1ms; } <-->node[18].face++;
	node[14].face++ <--> { delay = 1ms; } <-->node[19].face++;
	node[14].face++ <--> { delay = 1ms; } <-->node[20].face++;
	node[15].face++ <--> { delay = 1ms; } <-->node[21].face++;
	node[15].face++ <--> { delay = 1ms; } <-->node[22].face++;
	node[16].face++ <--> { delay = 1ms; } <-->node[23].face++;
	node[16].face++ <--> { delay = 1ms; } <-->node[24].face++;
	// Tree rooted at '3'
	node[3].face++ <--> { delay = 1ms; } <-->node[25].face++;
	node[3].face++ <--> { delay = 1ms; } <-->node[26].face++;
	node[25].face++ <--> { delay = 1ms; } <-->node[27].face++;
	node[25].face++ <--> { delay = 1ms; } <-->node[28].face++;
	node[26].face++ <--> { delay = 1ms; } <-->node[29].face++;
	node[26].face++ <--> { delay = 1ms; } <-->node[30].face++;
	node[27].face++ <--> { delay = 1ms; } <-->node[31].face++;
	node[27].face++ <--> { delay = 1ms; } <-->node[32].face++;
	node[28].face++ <--> { delay = 1ms; } <-->node[33].face++;
	node[28].face++ <--> { delay = 1ms; } <-->node[34].face++;
	node[29].face++ <--> { delay = 1ms; } <-->node[35].face++;
	node[29].face++ <--> { delay = 1ms; } <-->node[36].face++;
	node[30].face++ <--> { delay = 1ms; } <-->node[37].face++;
	node[30].face++ <--> { delay = 1ms; } <-->node[38].face++;
	// Tree rooted at '6'
	node[6].face++ <--> { delay = 1ms; } <-->node[39].face++;
	node[6].face++ <--> { delay = 1ms; } <-->node[40].face++;
	node[39].face++ <--> { delay = 1ms; } <-->node[41].face++;
	node[39].face++ <--> { delay = 1ms; } <-->node[42].face++;
	node[40].face++ <--> { delay = 1ms; } <-->node[43].face++;
	node[40].face++ <--> { delay = 1ms; } <-->node[44].face++;
	node[41].face++ <--> { delay = 1ms; } <-->node[45].face++;
	node[41].face++ <--> { delay = 1ms; } <-->node[46].face++;
	node[42].face++ <--> { delay = 1ms; } <-->node[47].face++;
	node[42].face++ <--> { delay = 1ms; } <-->node[48].face++;
	node[43].face++ <--> { delay = 1ms; } <-->node[49].face++;
	node[43].face++ <--> { delay = 1ms; } <-->node[50].face++;
	node[44].face++ <--> { delay = 1ms; } <-->node[51].face++;
	node[44].face++ <--> { delay = 1ms; } <-->node[52].face++;
	// Tree rooted at '9'
	node[9].face++ <--> { delay = 1ms; } <-->node[53].face++;
	node[9].face++ <--> { delay = 1ms; } <-->node[54].face++;
	node[53].face++ <--> { delay = 1ms; } <-->node[55].face++;
	node[53].face++ <--> { delay = 1ms; } <-->node[56].face++;
	node[54].face++ <--> { delay = 1ms; } <-->node[57].face++;
	node[54].face++ <--> { delay = 1ms; } <-->node[58].face++;
	node[55].face++ <--> { delay = 1ms; } <-->node[59].face++;
	node[55].face++ <--> { delay = 1ms; } <-->node[60].face++;
	node[56].face++ <--> { delay = 1ms; } <-->node[61].face++;
	node[56].face++ <--> { delay = 1ms; } <-->node[62].face++;
	node[57].face++ <--> { delay = 1ms; } <-->node[63].face++;
	node[57].face++ <--> { delay = 1ms; } <-->node[64].face++;
	node[58].face++ <--> { delay = 1ms; } <-->node[65].face++;
	node[58].face++ <--> { delay = 1ms; } <-->node[66].face++;
}