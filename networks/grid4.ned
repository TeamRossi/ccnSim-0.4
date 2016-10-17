package networks;


network grid4_network extends base_network{

    parameters:
        //Number of ccn nodes
    	n = 4;

connections allowunconnected:
    node[0].face++  <--> node[1].face++;
    node[2].face++  <--> node[3].face++;
    node[0].face++  <--> node[2].face++;
    node[1].face++  <--> node[3].face++;

}
