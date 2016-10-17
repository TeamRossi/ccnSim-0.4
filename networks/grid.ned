package networks;


network grid_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 16;

connections allowunconnected:

    node[0].face++ <--> node[1].face++;
    node[1].face++ <--> node[2].face++;
    node[2].face++ <--> node[3].face++;
    node[4].face++ <--> node[5].face++;
    node[5].face++ <--> node[6].face++;
    node[6].face++ <--> node[7].face++;
    node[8].face++ <--> node[9].face++;
    node[9].face++ <--> node[10].face++;
    node[10].face++ <--> node[11].face++;
    node[12].face++ <--> node[13].face++;
    node[13].face++ <--> node[14].face++;
    node[14].face++ <--> node[15].face++;

    node[0].face++ <--> node[4].face++;
    node[4].face++ <--> node[8].face++;
    node[8].face++ <--> node[12].face++;

    node[1].face++ <--> node[5].face++;
    node[5].face++ <--> node[9].face++;
    node[9].face++ <--> node[13].face++;

    node[2].face++ <--> node[6].face++;
    node[6].face++ <--> node[10].face++;
    node[10].face++ <--> node[14].face++;

    node[3].face++ <--> node[7].face++;
    node[7].face++ <--> node[11].face++;
    node[11].face++ <--> node[15].face++;

}
