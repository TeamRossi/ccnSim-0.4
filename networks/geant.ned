package networks;

network geant_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 22;

    connections allowunconnected:

	node[1].face++ <-->  { delay = 24.34ms; } <--> node[0].face++;
	node[10].face++ <--> { delay = 1.00ms; } <--> node[9].face++;
	node[11].face++ <--> { delay = 0.39ms; } <--> node[10].face++;
	node[12].face++ <--> { delay = 0.93ms; } <--> node[11].face++;
	node[12].face++ <--> { delay = 1.23ms; } <--> node[3].face++;
	node[12].face++ <--> { delay = 0.71ms; } <--> node[9].face++;
	node[13].face++ <--> { delay = 22.69ms; } <--> node[12].face++;
	node[14].face++ <--> { delay = 18.59ms; } <--> node[13].face++;
	node[14].face++ <--> { delay = 4.78ms; } <--> node[5].face++;
	node[15].face++ <--> { delay = 0.71ms; } <--> node[1].face++;
	node[15].face++ <--> { delay = 2.28ms; } <--> node[12].face++;
	node[15].face++ <--> { delay = 2.49ms; } <--> node[14].face++;
	node[15].face++ <--> { delay = 2.51ms; } <--> node[4].face++;
	node[16].face++ <--> { delay = 5.29ms; } <--> node[14].face++;
	node[17].face++ <--> { delay = 3.97ms; } <--> node[1].face++;
	node[17].face++ <--> { delay = 3.84ms; } <--> node[15].face++;
	node[17].face++ <--> { delay = 1.68ms; } <--> node[16].face++;
	node[18].face++ <--> { delay = 1.14ms; } <--> node[14].face++;
	node[18].face++ <--> { delay = 3.51ms; } <--> node[17].face++;
	node[18].face++ <--> { delay = 2.93ms; } <--> node[4].face++;
	node[19].face++ <--> { delay = 0.98ms; } <--> node[18].face++;
	node[2].face++ <--> { delay = 4.88ms; } <--> node[1].face++;
	node[20].face++ <--> { delay = 0.88ms; } <--> node[18].face++;
	node[20].face++ <--> { delay = 0.57ms; } <--> node[19].face++;
	node[21].face++ <--> { delay = 22.07ms; } <--> node[0].face++;
	node[21].face++ <--> { delay = 1.19ms; } <--> node[14].face++;
	node[21].face++ <--> { delay = 0.58ms; } <--> node[20].face++;
	node[21].face++ <--> { delay = 1.93ms; } <--> node[4].face++;
	node[3].face++ <--> { delay = 2.36ms; } <--> node[1].face++;
	node[3].face++ <--> { delay = 5.52ms; } <--> node[2].face++;
	node[4].face++ <--> { delay = 0.55ms; } <--> node[3].face++;
	node[5].face++ <--> { delay = 3.20ms; } <--> node[3].face++;
	node[6].face++ <--> { delay = 2.70ms; } <--> node[5].face++;
	node[7].face++ <--> { delay = 0.94ms; } <--> node[4].face++;
	node[7].face++ <--> { delay = 1.73ms; } <--> node[6].face++;
	node[8].face++ <--> { delay = 0.97ms; } <--> node[7].face++;
	node[9].face++ <--> { delay = 0.54ms; } <--> node[8].face++;

}
