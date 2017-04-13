/*
 * ccnSim is a scalable chunk-level simulator for Content Centric
 * Networks (CCN), that we developed in the context of ANR Connect
 * (http://www.anr-connect.org/)
 *
 * People:
 *    Michele Tortelli (Principal suspect 1.0, mailto michele.tortelli@telecom-paristech.fr)
 *    Andrea Araldo (Principal suspect 1.1, mailto araldo@lri.fr)
 *    Dario Rossi (Occasional debugger, mailto dario.rossi@enst.fr)
 *    Emilio Leonardi (Well informed outsider, mailto emilio.leonardi@tlc.polito.it)
 *    Giuseppe Rossini (Former lead developer, mailto giuseppe.rossini@enst.fr)
 *    Raffaele Chiocchetti (Former developer, mailto raffaele.chiocchetti@gmail.com)
 *
 * Mailing list: 
 *	  ccnsim@listes.telecom-paristech.fr
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "ccnsim.h"
#include "content_distribution.h"
#include "ShotNoiseContentDistribution.h"
#include "zipf.h"
#include "zipf_sampled.h"
#include <algorithm>
#include <boost/tokenizer.hpp>

#include <error_handling.h>

Register_Class(content_distribution);


vector<file> content_distribution::catalog;
vector<zipf_sampled*>  content_distribution::zipf;

name_t  content_distribution::stabilization_bulk = 0;
name_t  content_distribution::perfile_bulk = 0;
name_t  content_distribution::cut_off = 0;
int  *content_distribution::repositories = 0;
double  *content_distribution::repo_prices = 0;
int  *content_distribution::clients = 0;
int  *content_distribution::total_replicas_p;
vector<double>  *content_distribution::repo_popularity_p;
int content_distribution::num_repos;

// Initialize catalog, repositories, and distribute contents among them.
void content_distribution::initialize()
{
	cout << "CONTENT DISTRIBUTION" << endl;


    nodes = getAncestorPar("n");
    num_repos = getAncestorPar("num_repos"); // Number of repositories (specifically ccn_node(s) which have a repository connected to them)
    num_clients = getAncestorPar ("num_clients");
    q = par ("q");
    F = par("file_size"); 					 // Average file size expressed in number of chunks.
    stat_aggr = par("stat_aggr");			 // Percentile for which statistics are gathered.
    replicas = getAncestorPar("replicas");


    // If the Shot Noise Model is simulated, parameters like 'alpha' and 'cardF' are taken from the configuration
    // file for each class of content. As a consequence, the relative zipf and cdf vectors are initialized by the
    // shot noise model module per each content class, and not here.
    cModule* pSubModule = getParentModule()->getSubmodule("content_distribution");
    if (pSubModule)
    {
    	ShotNoiseContentDistribution* pClass2Module = dynamic_cast<ShotNoiseContentDistribution*>(pSubModule);
    	if (!pClass2Module)     	// If the SNM is NOT simulated.
    	{
    		// Take parameters from the .ini file
    		alpha = par("alpha");		// Zipf's exponent for the whole catalog.

    		unsigned long cardF_temp = par("objects");	// Original cardinality
    		cardF = (unsigned long long)cardF_temp;

    		// Retrieve the downscaling factor in order to compute the new cardinality
    		cModule* pSubModStat = getParentModule()->getSubmodule("statistics");
    		statistics* pClassStat = dynamic_cast<statistics*>(pSubModStat);

    		unsigned long down = pClassStat->par("downsize");
    		double lambda = pSubModStat->par("lambda");

    		newCardF = round(cardF*(1./(double)down));

    		cout << " DOWNSCALED CARDINALITY from ContentDistribution = " << newCardF << endl;

    		zipf.resize(1);
    		zipf[0] = new zipf_sampled((unsigned long long)newCardF,alpha,lambda,down);  // Zipf with rejection-inversion sampling

    	}
    	else		// The SNM is simulated.
    	{
    		cardF = pClass2Module->totalContents;
    		newCardF = cardF;
    		cout << "ShotNoise CARDINALITY: " << cardF;
    		perfile_bulk = cardF;  		// In case the Shot Noise Model is simulated, we have to gather statistics
    									// for all the content in the catalog (a clear correspondence with the
    									// IRM model with a single big catalog is still missing).

    		unsigned long card_temp = round(cardF*(1./(double)down));
    		zipf.resize(1);
    		zipf[0] = new zipf_sampled((unsigned long long)card_temp,alpha,lambda,down);  // Zipf with rejection-inversion sampling

    	}
    }

    if (cardF == 0){
	        std::stringstream ermsg; 
			ermsg<<"The catalog size is 0. Are you sure you intended this?If you are sure, please "<<
				" disable this exception";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
    }


    content_distribution::total_replicas_p = (int*) malloc ( sizeof(int) );
	*(content_distribution::total_replicas_p) = 0;



    catalog.resize(newCardF+1); 	// Initialize content catalog.


    // *** Repositories initialization ***
    char name[15];

    cStringTokenizer tokenizer(getAncestorPar("node_repos"),",");
    repositories = init_repos(tokenizer.asIntVector());

    repo_prices = init_repo_prices();


    // Useful for statistics: write out the name of each repository within the network.
    for (int i = 0; i < num_repos; i++){
		sprintf(name,"repo-%d",i);
		recordScalar(name,repositories[i]);
    }

    // *** Clients initialization ***
    if (num_clients < 0) 	// All nodes of the network will be clients.
		num_clients = nodes;
    tokenizer = cStringTokenizer(getAncestorPar("node_clients"),",");
    clients = init_clients (tokenizer.asIntVector());

    //Useful for statistics: write out the name of each client within the network.
    for (int i = 0; i < num_clients; i++){
		sprintf(name,"client-%d",i);
		recordScalar(name,clients[i]);
    }

	initialize_repo_popularity();

    // *** Content initialization ***
    cout<<"Start content initialization..."<<endl;
    init_content();
    cout<<"Content initialized"<<endl;

	finalize_total_replica();
}

void content_distribution::initialize_repo_popularity()
{
	// By default, the repo popularity is not computed.
	repo_popularity_p = NULL;
}

void content_distribution::finalize_total_replica(){

	//*total_replicas_p = cardF*replicas;
	*total_replicas_p = newCardF*replicas;
}

#ifdef SEVERE_DEBUG
void content_distribution::verify_replica_number(){
	if (*total_replicas_p != cardF*replicas)
	{
        std::stringstream ermsg; 
		ermsg<<"Ctlg size="<< cardF <<". total_replica="<<*total_replicas_p;
	    severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
}
#endif


/* 
 * Generate all possible combinations of binary strings of a given length with
 * a given number of bits set.
*/
vector<unsigned short> content_distribution::binary_strings(int num_ones,int len){

		#ifdef SEVERE_DEBUG
			if (num_ones <= 0){
				std::stringstream ermsg; 
				ermsg<<"You want a number of object replicas equal to "<<
					num_ones<<". It is not valid";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
		#endif

    //vector<int> bins;
    //int ones,bin;
	vector<unsigned short> bins;
	unsigned short ones,bin;
    for (int i =1;i< (1<<len);i++){
		bin = i; 	// It is the binary string.
		ones = 0;
		// Count the number of ones
		while (bin){
			ones += bin & 1;
			bin >>= 1;
		}
		// If the ones are equal to the number of repositories this is a good binary number
		if (ones == num_ones)
			bins.push_back(i);

		#ifdef SEVERE_DEBUG
			if (bins.size() == 0){
				std::stringstream ermsg; 
				ermsg<<"No binary strings were produced. The number of replicas for "<<
					"each content should be "<<num_ones;
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
		#endif
    }
    return bins;

}

// Return a string of bit representing an object placement.
// There is a 1 in the i-th position iff the object is served by the i-th repo
unsigned short content_distribution::choose_repos (int object_index )
{
	unsigned short repo_string = repo_strings[intrand(repo_strings.size())];

	#ifdef SEVERE_DEBUG
	int num_1_bits =  __builtin_popcount (repo_string); //http://stackoverflow.com/a/109069
												// Number of bits set to 1 (corresponding 
												// to the number of repositories this object was 
												// assigned to)
	if (num_1_bits != replicas){
		std::stringstream ermsg; 
		ermsg<<"an object has been assigned to "<< num_1_bits <<" repos while replicas="<<replicas;
	    severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

	*total_replicas_p += num_1_bits;
	#endif

	return repo_string;
}

//	Store information about the content.
void content_distribution::init_content()
{
	// In repo_card we count how many objects each repo is storing.
	vector<int> repo_card(num_repos,0); 

    // As the repositories are represented as a string of bits, the function
    // binary_string is used for generating binary strings of length num_repos
    // with exactly replicas ones, where "replicas" is the number of replicas for each object.
	// Each string represents a replica placement of a certain object among the repositories.
	// Given a single string, a 1 in the i-th position means that a replica of that object
	// is placed in the i-th repository.
    repo_strings = binary_strings(replicas, num_repos);

    //for (int d = 1; d <= cardF; d++)
    for (int d = 1; d <= newCardF; d++)
    {
    	// 'd' is a content.
		// Reset the information field of a given content
		__info(d) = 0;

		// F is the size of a file.
		if (F > 1){
			// Set the file size (geometrically distributed).
			filesize_t s = geometric( 1.0 / F ) + 1;
			__ssize ( d, s );
		}else 
			__ssize( d , 1);

		vector<int> chosen_repos; 

		// Set the repositories.
		if (num_repos==1){
			__srepo ( d , 1 );
			// Compute the chosen_repo
			chosen_repos.push_back(0);
		} else {
			// Choose a replica placement among all the possible ones.
			// repos is a replica placement.
			repo_t repos = choose_repos(d);
			__srepo (d ,repos);

			// Compute the chosen_repos
			repo_t repo_extracted = __repo(d);
			unsigned k = 0;
			while (repo_extracted)
			{
				if (repo_extracted & 1)
				{
					chosen_repos.push_back(k);
					//cout << "Content # " << d << " in Repo # " << repositories[k] << endl;
				}
				repo_extracted >>= 1;
				k++;
			}
		}

		// Update the repository cardinality
		for (unsigned repo_idx = 0; repo_idx < chosen_repos.size(); repo_idx++)
					repo_card[ chosen_repos[repo_idx] ] ++;

    }

	// Record the repository cardinality and price
	for (int repo_idx = 0; repo_idx < num_repos; repo_idx++){
	    char name[15];
		sprintf(name,"repo-%d_card",repo_idx);
		recordScalar(name, repo_card[repo_idx] );

		sprintf(name,"repo-%d_price",repo_idx);
		recordScalar(name, repo_prices[repo_idx] ); 

	}
}

/*
* Initialize the repositories vector. This vector is composed of the
* repositories specified in the ini file.  In addition some random repositories
* are added if one wished more repositories than the fixed number specified
* (see omnet.ini for further comments).
*
* Return value:
* 	repositories[.], where repositories[i] = d means that the i-th repository is
*					 in node[d]
*
*/

int *content_distribution::init_repos(vector<int> node_repos)
{

		if (num_repos > nodes)
			error("content_distribution::init_repos(..): You are trying to distribute more repositories than the number of nodes. This is impossible since there can only be one repository for each node");

		if (node_repos.size() > (unsigned) num_repos)
			error("content_distribution::init_repos(..): You are trying to distribute too many repositories.");

			for (unsigned i=0; i < node_repos.size(); i++ )
			{
				int r = node_repos[i];
				int max_node_index = nodes - 1;
				if (r > max_node_index)
				{
				    std::stringstream ermsg; 
					ermsg<<"You are trying to associate a repo to a node "<<r
						<<" that does not exist ";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}		
			}


	int repo_id = 0;
	int *repositories = new int[num_repos];
	// Construct repositories array. repositories[repo_id] will be the id of the
	// node which repo_id-th repository is connected to.
	while (node_repos.size() ){
			int r = node_repos[repo_id];
			node_repos.pop_back();
			repositories[repo_id++] = r;
	}

   	// We already assigned i repositories. Now we randomly assign the remaining part.
    int new_rep;
    while ( repo_id < num_repos  )
    {
		new_rep = intrand(nodes);
		if (find (repositories,repositories + repo_id , new_rep) == repositories + repo_id )
		{
			repositories[repo_id++] = new_rep;
		}
    }
    
    #ifdef SEVERE_DEBUG
		std::stringstream ss;
		ss<<"The repositories are in the following nodes";
		for (int j=0; j < repo_id; j++)
			ss<<" "<<repositories[j];
		ss<<endl;
		debug_message(__FILE__,__LINE__, ss.str().c_str() );
    #endif

    return repositories;
}


double *content_distribution::init_repo_prices()
{
	double *repo_prices = new double[num_repos];
	// Construct repo_prices array. repo_prices[i] will be the price of the i-th
	// repository.
	for (int i=0; i<num_repos; i++)
		repo_prices[i] = 0;
    
    #ifdef SEVERE_DEBUG
		std::stringstream ss;
		ss<<"Prices are";
		for (int j=0; j<num_repos; j++)
			ss<<" "<<repo_prices[j];
		ss<<endl;
		debug_message(__FILE__,__LINE__, ss.str().c_str() );
    #endif

    return repo_prices;
}


/*
* Initialize the clients vector. This vector is composed by the clients
* specified into the ini file.  In addition, some random clients are added if
* one wished more repositories than the fixed number specified (see omnet.ini
* for further comments).
*/
int *content_distribution::init_clients(vector<int> node_clients){

    if (node_clients.size() > (unsigned) num_clients)
		error("You try to distribute too much clients.");

		for (unsigned int i=0; i < node_clients.size(); i++ )
		{
			int r = node_clients[i];
			int max_node_index = nodes - 1;
			if (r > max_node_index)
			{
		        std::stringstream ermsg; 
				ermsg<<"You are trying to associate a client to a node "<<r
					<<" that does not exist ";
			    severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}		
		}

    if (clients != 0)
		return clients;

    int *clients = new int [num_clients];

    int i = 0;
    while (node_clients.size() )
	{
		int r = node_clients[i];
		node_clients.pop_back();
		clients[i++] = r;
    }

    int new_c;
    while ( i <  num_clients  ){
	new_c = intrand(nodes);
	// NOTE: in this way a client can be attached to a node
	// where a repository is already attached.
	if (find (clients,clients + i , new_c) == clients + i)
	    clients[i++] = new_c;
    }
    return clients;
}

