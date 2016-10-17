package networks;


network ndntestbed_26_rand_network extends base_network{

    parameters:
        //Number of ccn nodes
        n = 26;

    // Repos and clients are assigned randomly                    

connections allowunconnected:

node[0].face++ <--> { delay = 1ms; } <--> node[1].face++;
node[0].face++ <--> { delay = 1ms; } <--> node[2].face++;
node[0].face++ <--> { delay = 1ms; } <--> node[4].face++;
node[0].face++ <--> { delay = 1ms; } <--> node[22].face++;
node[1].face++ <--> { delay = 1ms; } <--> node[2].face++;
node[1].face++ <--> { delay = 1ms; } <--> node[3].face++;
node[2].face++ <--> { delay = 1ms; } <--> node[3].face++;
node[2].face++ <--> { delay = 1ms; } <--> node[6].face++;
node[2].face++ <--> { delay = 1ms; } <--> node[21].face++;
node[3].face++ <--> { delay = 1ms; } <--> node[4].face++;
node[3].face++ <--> { delay = 1ms; } <--> node[5].face++;
node[3].face++ <--> { delay = 1ms; } <--> node[6].face++;
node[4].face++ <--> { delay = 1ms; } <--> node[5].face++;
node[4].face++ <--> { delay = 1ms; } <--> node[6].face++;
node[4].face++ <--> { delay = 1ms; } <--> node[7].face++;
node[4].face++ <--> { delay = 1ms; } <--> node[8].face++;
node[4].face++ <--> { delay = 1ms; } <--> node[25].face++;
node[5].face++ <--> { delay = 1ms; } <--> node[6].face++;
node[6].face++ <--> { delay = 1ms; } <--> node[9].face++;
node[6].face++ <--> { delay = 1ms; } <--> node[11].face++;
node[6].face++ <--> { delay = 1ms; } <--> node[24].face++;
node[7].face++ <--> { delay = 1ms; } <--> node[8].face++;
node[7].face++ <--> { delay = 1ms; } <--> node[9].face++;
node[7].face++ <--> { delay = 1ms; } <--> node[10].face++;
node[7].face++ <--> { delay = 1ms; } <--> node[13].face++;
node[8].face++ <--> { delay = 1ms; } <--> node[10].face++;
node[8].face++ <--> { delay = 1ms; } <--> node[11].face++;
node[8].face++ <--> { delay = 1ms; } <--> node[12].face++;
node[8].face++ <--> { delay = 1ms; } <--> node[17].face++;
node[9].face++ <--> { delay = 1ms; } <--> node[11].face++;
node[9].face++ <--> { delay = 1ms; } <--> node[19].face++;
node[10].face++ <--> { delay = 1ms; } <--> node[11].face++;
node[11].face++ <--> { delay = 1ms; } <--> node[12].face++;
node[11].face++ <--> { delay = 1ms; } <--> node[14].face++;
node[12].face++ <--> { delay = 1ms; } <--> node[15].face++;
node[12].face++ <--> { delay = 1ms; } <--> node[21].face++;
node[13].face++ <--> { delay = 1ms; } <--> node[14].face++;
node[13].face++ <--> { delay = 1ms; } <--> node[17].face++;
node[13].face++ <--> { delay = 1ms; } <--> node[18].face++;
node[13].face++ <--> { delay = 1ms; } <--> node[19].face++;
node[14].face++ <--> { delay = 1ms; } <--> node[15].face++;
node[14].face++ <--> { delay = 1ms; } <--> node[16].face++;
node[14].face++ <--> { delay = 1ms; } <--> node[17].face++;
node[14].face++ <--> { delay = 1ms; } <--> node[18].face++;
node[15].face++ <--> { delay = 1ms; } <--> node[16].face++;
node[15].face++ <--> { delay = 1ms; } <--> node[18].face++;
node[15].face++ <--> { delay = 1ms; } <--> node[21].face++;
node[16].face++ <--> { delay = 1ms; } <--> node[17].face++;
node[17].face++ <--> { delay = 1ms; } <--> node[18].face++;
node[17].face++ <--> { delay = 1ms; } <--> node[19].face++;
node[17].face++ <--> { delay = 1ms; } <--> node[25].face++;
node[18].face++ <--> { delay = 1ms; } <--> node[19].face++;
node[18].face++ <--> { delay = 1ms; } <--> node[21].face++;
node[20].face++ <--> { delay = 1ms; } <--> node[21].face++;
node[20].face++ <--> { delay = 1ms; } <--> node[22].face++;
node[20].face++ <--> { delay = 1ms; } <--> node[23].face++;
node[20].face++ <--> { delay = 1ms; } <--> node[24].face++;
node[20].face++ <--> { delay = 1ms; } <--> node[25].face++;
node[21].face++ <--> { delay = 1ms; } <--> node[22].face++;
node[21].face++ <--> { delay = 1ms; } <--> node[23].face++;
node[22].face++ <--> { delay = 1ms; } <--> node[23].face++;
node[22].face++ <--> { delay = 1ms; } <--> node[25].face++;
node[23].face++ <--> { delay = 1ms; } <--> node[24].face++;
node[23].face++ <--> { delay = 1ms; } <--> node[25].face++;
node[24].face++ <--> { delay = 1ms; } <--> node[25].face++;
}

// NODE MAPPING
// The ndn testbed topology is taken from http://www.arl.wustl.edu/~jdd/ndnstatus/ndn_prefix/tbs_ndnx.html at the current date (09/10/15) 
// A number is assigned to each node in order to describe it inside the .ned file

// 0   =   CAIDA
// 1   =   UCI
// 2   =   UCLA
// 3   =   REMAP
// 4   =   UA
// 5   =   BYU
// 6   =   CSU
// 7   =   WU
// 8   =   UM
// 9   =   UIUC
// 10  =   VERISIGN
// 11  =   MICH
// 12  =   NEU
// 13  =   URJC
// 14  =   LIP6
// 15  =   NTNU
// 16  =   SYSTEMX
// 17  =   ORANGE
// 18  =   BASEL    
// 19  =   PADUA
// 20  =   BUPT
// 21  =   PKU
// 22  =   TONJI   
// 23  =   ANYANG
// 24  =   KISTI
// 25  =   WASEDA