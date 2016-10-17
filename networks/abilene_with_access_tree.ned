package networks;

network abilene_with_access_tree_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 165;

		//Number of repositories
		node_repos = "0,2,5,7,10";
		num_repos = 5;
		replicas = 1;

        //Number of clients
		num_clients = 88;
		node_clients = "17,18,19,20,21,22,23,24,31,32,33,34,35,36,37,38,45,46,47,48,49,50,51,52,59,60,61,62,63,64,65,66,73,74,75,76,77,78,79,80,87,88,89,90,91,92,93,94,101,102,103,104,105,106,107,108,115,116,117,118,119,120,121,122,129,130,131,132,133,134,135,136,143,144,145,146,147,148,149,150,157,158,159,160,161,162,163,164";

connections allowunconnected:
	node[1].face++ <--> { delay = 1ms; } <--> node[0].face++;
	// tree rooted at '0'
	node[0].face++ <--> { delay = 1ms; } <-->node[11].face++;
	node[0].face++ <--> { delay = 1ms; } <-->node[12].face++;
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
	//
	node[10].face++ <--> { delay = 1ms; } <--> node[0].face++;
	node[10].face++ <--> { delay = 1ms; } <--> node[1].face++;
	// tree rooted at '1'
	node[1].face++ <--> { delay = 1ms; } <-->node[25].face++;
	node[1].face++ <--> { delay = 1ms; } <-->node[26].face++;
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
	//
	node[10].face++ <--> { delay = 1ms; } <--> node[9].face++;
	node[2].face++ <--> { delay = 1ms; } <--> node[1].face++;
	// tree rooted at '2'
	node[2].face++ <--> { delay = 1ms; } <-->node[39].face++;
	node[2].face++ <--> { delay = 1ms; } <-->node[40].face++;
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
	//
	node[3].face++ <--> { delay = 1ms; } <--> node[2].face++;
	// tree rooted at '3'
	node[3].face++ <--> { delay = 1ms; } <-->node[53].face++;
	node[3].face++ <--> { delay = 1ms; } <-->node[54].face++;
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
	//
	node[4].face++ <--> { delay = 1ms; } <--> node[3].face++;
	// tree rooted at '4'
	node[4].face++ <--> { delay = 1ms; } <-->node[67].face++;
	node[4].face++ <--> { delay = 1ms; } <-->node[68].face++;
	node[67].face++ <--> { delay = 1ms; } <-->node[69].face++;
	node[67].face++ <--> { delay = 1ms; } <-->node[70].face++;
	node[68].face++ <--> { delay = 1ms; } <-->node[71].face++;
	node[68].face++ <--> { delay = 1ms; } <-->node[72].face++;
	node[69].face++ <--> { delay = 1ms; } <-->node[73].face++;
	node[69].face++ <--> { delay = 1ms; } <-->node[74].face++;
	node[70].face++ <--> { delay = 1ms; } <-->node[75].face++;
	node[70].face++ <--> { delay = 1ms; } <-->node[76].face++;
	node[71].face++ <--> { delay = 1ms; } <-->node[77].face++;
	node[71].face++ <--> { delay = 1ms; } <-->node[78].face++;
	node[72].face++ <--> { delay = 1ms; } <-->node[79].face++;
	node[72].face++ <--> { delay = 1ms; } <-->node[80].face++;
	//
	node[5].face++ <--> { delay = 1ms; } <--> node[4].face++;
	// tree rooted at '5'
	node[5].face++ <--> { delay = 1ms; } <-->node[81].face++;
	node[5].face++ <--> { delay = 1ms; } <-->node[82].face++;
	node[81].face++ <--> { delay = 1ms; } <-->node[83].face++;
	node[81].face++ <--> { delay = 1ms; } <-->node[84].face++;
	node[82].face++ <--> { delay = 1ms; } <-->node[85].face++;
	node[82].face++ <--> { delay = 1ms; } <-->node[86].face++;
	node[83].face++ <--> { delay = 1ms; } <-->node[87].face++;
	node[83].face++ <--> { delay = 1ms; } <-->node[88].face++;
	node[84].face++ <--> { delay = 1ms; } <-->node[89].face++;
	node[84].face++ <--> { delay = 1ms; } <-->node[90].face++;
	node[85].face++ <--> { delay = 1ms; } <-->node[91].face++;
	node[85].face++ <--> { delay = 1ms; } <-->node[92].face++;
	node[86].face++ <--> { delay = 1ms; } <-->node[93].face++;
	node[86].face++ <--> { delay = 1ms; } <-->node[94].face++;
	//
	node[6].face++ <--> { delay = 1ms; } <--> node[5].face++;
	// tree rooted at '6'
	node[6].face++ <--> { delay = 1ms; } <-->node[95].face++;
	node[6].face++ <--> { delay = 1ms; } <-->node[96].face++;
	node[95].face++ <--> { delay = 1ms; } <-->node[97].face++;
	node[95].face++ <--> { delay = 1ms; } <-->node[98].face++;
	node[96].face++ <--> { delay = 1ms; } <-->node[99].face++;
	node[96].face++ <--> { delay = 1ms; } <-->node[100].face++;
	node[97].face++ <--> { delay = 1ms; } <-->node[101].face++;
	node[97].face++ <--> { delay = 1ms; } <-->node[102].face++;
	node[98].face++ <--> { delay = 1ms; } <-->node[103].face++;
	node[98].face++ <--> { delay = 1ms; } <-->node[104].face++;
	node[99].face++ <--> { delay = 1ms; } <-->node[105].face++;
	node[99].face++ <--> { delay = 1ms; } <-->node[106].face++;
	node[100].face++ <--> { delay = 1ms; } <-->node[107].face++;
	node[100].face++ <--> { delay = 1ms; } <-->node[108].face++;
	//
	node[7].face++ <--> { delay = 1ms; } <--> node[3].face++;
	node[7].face++ <--> { delay = 1ms; } <--> node[6].face++;
	// tree rooted at '7'
	node[7].face++ <--> { delay = 1ms; } <-->node[109].face++;
	node[7].face++ <--> { delay = 1ms; } <-->node[110].face++;
	node[109].face++ <--> { delay = 1ms; } <-->node[111].face++;
	node[109].face++ <--> { delay = 1ms; } <-->node[112].face++;
	node[110].face++ <--> { delay = 1ms; } <-->node[113].face++;
	node[110].face++ <--> { delay = 1ms; } <-->node[114].face++;
	node[111].face++ <--> { delay = 1ms; } <-->node[115].face++;
	node[111].face++ <--> { delay = 1ms; } <-->node[116].face++;
	node[112].face++ <--> { delay = 1ms; } <-->node[117].face++;
	node[112].face++ <--> { delay = 1ms; } <-->node[118].face++;
	node[113].face++ <--> { delay = 1ms; } <-->node[119].face++;
	node[113].face++ <--> { delay = 1ms; } <-->node[120].face++;
	node[114].face++ <--> { delay = 1ms; } <-->node[121].face++;
	node[114].face++ <--> { delay = 1ms; } <-->node[122].face++;
	//
	node[8].face++ <--> { delay = 1ms; } <--> node[2].face++;
	// tree rooted at '8'
	node[8].face++ <--> { delay = 1ms; } <-->node[123].face++;
	node[8].face++ <--> { delay = 1ms; } <-->node[124].face++;
	node[123].face++ <--> { delay = 1ms; } <-->node[125].face++;
	node[123].face++ <--> { delay = 1ms; } <-->node[126].face++;
	node[124].face++ <--> { delay = 1ms; } <-->node[127].face++;
	node[124].face++ <--> { delay = 1ms; } <-->node[128].face++;
	node[125].face++ <--> { delay = 1ms; } <-->node[129].face++;
	node[125].face++ <--> { delay = 1ms; } <-->node[130].face++;
	node[126].face++ <--> { delay = 1ms; } <-->node[131].face++;
	node[126].face++ <--> { delay = 1ms; } <-->node[132].face++;
	node[127].face++ <--> { delay = 1ms; } <-->node[133].face++;
	node[127].face++ <--> { delay = 1ms; } <-->node[134].face++;
	node[128].face++ <--> { delay = 1ms; } <-->node[135].face++;
	node[128].face++ <--> { delay = 1ms; } <-->node[136].face++;
	//
	node[8].face++ <--> { delay = 1ms; } <--> node[7].face++;
	// tree rooted at '9'
	node[9].face++ <--> { delay = 1ms; } <-->node[137].face++;
	node[9].face++ <--> { delay = 1ms; } <-->node[138].face++;
	node[137].face++ <--> { delay = 1ms; } <-->node[139].face++;
	node[137].face++ <--> { delay = 1ms; } <-->node[140].face++;
	node[138].face++ <--> { delay = 1ms; } <-->node[141].face++;
	node[138].face++ <--> { delay = 1ms; } <-->node[142].face++;
	node[139].face++ <--> { delay = 1ms; } <-->node[143].face++;
	node[139].face++ <--> { delay = 1ms; } <-->node[144].face++;
	node[140].face++ <--> { delay = 1ms; } <-->node[145].face++;
	node[140].face++ <--> { delay = 1ms; } <-->node[146].face++;
	node[141].face++ <--> { delay = 1ms; } <-->node[147].face++;
	node[141].face++ <--> { delay = 1ms; } <-->node[148].face++;
	node[142].face++ <--> { delay = 1ms; } <-->node[149].face++;
	node[142].face++ <--> { delay = 1ms; } <-->node[150].face++;
	//
	node[9].face++ <--> { delay = 1ms; } <--> node[8].face++;
	// tree rooted at '10'
	node[10].face++ <--> { delay = 1ms; } <-->node[151].face++;
	node[10].face++ <--> { delay = 1ms; } <-->node[152].face++;
	node[151].face++ <--> { delay = 1ms; } <-->node[153].face++;
	node[151].face++ <--> { delay = 1ms; } <-->node[154].face++;
	node[152].face++ <--> { delay = 1ms; } <-->node[155].face++;
	node[152].face++ <--> { delay = 1ms; } <-->node[156].face++;
	node[153].face++ <--> { delay = 1ms; } <-->node[157].face++;
	node[153].face++ <--> { delay = 1ms; } <-->node[158].face++;
	node[154].face++ <--> { delay = 1ms; } <-->node[159].face++;
	node[154].face++ <--> { delay = 1ms; } <-->node[160].face++;
	node[155].face++ <--> { delay = 1ms; } <-->node[161].face++;
	node[155].face++ <--> { delay = 1ms; } <-->node[162].face++;
	node[156].face++ <--> { delay = 1ms; } <-->node[163].face++;
	node[156].face++ <--> { delay = 1ms; } <-->node[164].face++;
	//
}