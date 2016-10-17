package networks;
import modules.content.IContentDistribution;
import modules.statistics.statistics;
//import modules.clients.client;
import modules.clients.IClient;
import modules.node.node;
import modules.node.Inode;
import modules.node.BorderNode;

network base_network {

    parameters:
        //Number of ccn nodes
    	int n = default(1);

		//Number of repositories
		string node_repos = default("");//n-1
		int num_repos = default(1);
		int replicas = default(1);

		//Number of clients 
		int num_clients = default(1);
		string node_clients = default("");

		//<aa>
		string content_distribution_type = default("content_distribution");
		//</aa>

		//**mt**
		string client_type = default("client_IRM");

    submodules:
    	content_distribution: <content_distribution_type> like IContentDistribution{
			parameters:
			    @display("p=900,200");
		}

 	statistics: statistics{
	    parameters:
	        @display("p=900,100");
	}

        node [n]: <default("node")> like Inode; // BorderNode;
	//client[n]: client;
	//**mt**
	client[n]: <client_type> like IClient {
		parameters:
			@display("p=900,50");
	   }


connections allowunconnected:
	for i = 0..n-1{
	    node[i].face++ <--> { delay = 0ms; } <--> client[i].client_port;
	}

}
