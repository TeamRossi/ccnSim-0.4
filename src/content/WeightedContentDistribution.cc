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
//<aa>

#include "ccnsim.h"
#include "WeightedContentDistribution.h"
#include "content_distribution.h"
#include <error_handling.h>
#include <core_layer.h>

Register_Class(WeightedContentDistribution);

void WeightedContentDistribution::initialize()
{
	const char *str = par("catalog_split").stringValue();
	catalog_split = cStringTokenizer(str,"_").asDoubleVector();
	replication_admitted = par("replication_admitted");
	priceratio = par("priceratio");
	kappa = par("kappa");
	alpha = par("alpha");

	unsigned repo_num = catalog_split.size();

	std::stringstream ermsg; 

	double sum = 0;
	for (unsigned i=0; i < repo_num; i++)
	{
		{// Input consistency check
			if ( catalog_split[i] > 1 ||  catalog_split[i] < 0){
				ermsg<<"Weight : "<<catalog_split[i]<<" is invalid";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
		}
		
		sum += catalog_split[i];
	}

    probabilities = (double* ) malloc(repo_num * sizeof(double) );
	for (unsigned repo_idx = 0; repo_idx < repo_num; repo_idx++)
		probabilities[repo_idx] = catalog_split[repo_idx] / sum;

	content_distribution::initialize();


	{// Other checks
		if (!replication_admitted && replicas != 1 ){
			ermsg<<"When you set replication_admitted=false you MUST set replicas=1.";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		} else if (replication_admitted && replicas != -1){
			ermsg<<"When you set replication_admitted=true The of the parameter \"replicas\" is meaningless. "<<
				" You must set it to -1";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}

		#ifdef SEVERE_DEBUG
		if (!replication_admitted && *total_replicas_p != cardF)
		{
	        std::stringstream ermsg; 
			ermsg<<"total_replicas="<<*total_replicas_p<<"; cardF="<<cardF<<". ";
			ermsg<<"Since replication_admitted==false, There MUST be 1 replica for each content ";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		
	
		// Verify popularity indication
		double repo_popularity_sum = 0;
		for (unsigned repo_idx = 0; repo_idx < repo_popularity_p->size(); repo_idx++){
			repo_popularity_sum += (*repo_popularity_p)[repo_idx];
		}
		if ( !double_equality(repo_popularity_sum, 1) )
		{
	        std::stringstream ermsg; 
			ermsg<<"The sum of the popularity indications must be 1, but it is "<<
					repo_popularity_sum<<". The differencer is "<< 1-repo_popularity_sum;
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );		
		}
		#endif
	}
	#ifdef SEVERE_DEBUG
	initialized = true;
	#endif
}



void WeightedContentDistribution::initialize_repo_popularity()
{
	unsigned repo_num = catalog_split.size();
	repo_popularity_p = new vector<double>(repo_num); //http://stackoverflow.com/a/970555
}

//<aa>
// Overwrite content_distribution::init_repo_prices()
double *WeightedContentDistribution::init_repo_prices()
{
	if (num_repos != 3)
	{
	    std::stringstream ermsg; 
		ermsg<<"num_repos="<<num_repos<<"; while, for the time being, this class works only with 3 repos";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

	double price_permutations[6][3] = { 
		{0,1,priceratio},
		{0,priceratio,1},
		{1,0,priceratio},
		{1,priceratio,0},
		{priceratio,0,1},
		{priceratio,1,0}
	}; // -1 stands for priceratio

	double* selected_price_permutation = price_permutations[intrand(6)];

	double *repo_prices = new double[num_repos];
	for (int repo_idx=0; repo_idx < num_repos; repo_idx++)
	{
		repo_prices[repo_idx] = selected_price_permutation[repo_idx];
	}

	cout<<"repo_prices is ";
	for (int i=0; i<3; i++) cout<<repo_prices[i] <<"; ";	
	cout<<endl;
    return repo_prices;
}
//</aa>


#ifdef SEVERE_DEBUG
// Override content_distribution::verify_replica_number()
void WeightedContentDistribution::verify_replica_number(){
	// Do nothing
}
#endif

#ifdef SEVERE_DEBUG
bool WeightedContentDistribution::isInitialized(){
	return initialized;
}
#endif

const vector<double> WeightedContentDistribution::get_catalog_split(){
	#ifdef SEVERE_DEBUG
	if (!isInitialized() ){
        std::stringstream ermsg; 
		ermsg<<"module must be initialized before calling this method";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	return catalog_split;
}


const double WeightedContentDistribution::get_priceratio(){
	#ifdef SEVERE_DEBUG
	if (!isInitialized() ){
        std::stringstream ermsg; 
		ermsg<<"module must be initialized before calling this method";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	return priceratio;
}

const double WeightedContentDistribution::get_kappa(){
	#ifdef SEVERE_DEBUG
	if (!isInitialized() ){
        std::stringstream ermsg; 
		ermsg<<"module must be initialized before calling this method";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	return kappa;
}

const double WeightedContentDistribution::get_alpha(){
	#ifdef SEVERE_DEBUG
	if (!isInitialized() ){
        std::stringstream ermsg; 
		ermsg<<"module must be initialized before calling this method";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	return alpha;
}

// Override content_distribution::finalize_total_replica()
void WeightedContentDistribution::finalize_total_replica(){
	// Do nothing
}



// Override content_distribution::binary_strings(..)
vector<unsigned short> WeightedContentDistribution::binary_strings(int num_ones,int len){
	// Do nothing as we don't need this in this case
	vector<unsigned short> v;
	return v;
}

// The repository with bigger weight will have the more contents
//		PAY ATTENTION: 
//			- Verify the correctness of catalog_split before calling
//				this method. Their sum must be 1 and 
unsigned short WeightedContentDistribution::choose_repos (int object_index )
{
	int assigned_repo = -1;

	if (replication_admitted) 
	{
		for (int repo_idx = 0; repo_idx < num_repos; repo_idx++)
		{
			if (dblrand() < catalog_split[repo_idx] ){
				(*total_replicas_p) = (*total_replicas_p) +1;
				if ( assigned_repo==-1 )
					// The object has not been assigned yet. Assign it to repo_idx
					assigned_repo = repo_idx;

				// else: do nothing
					// The object has already been assigned to a previous repository.
					// Since the previous repositories are cheaper than the current
					// repo_idx, the copy on the current repository will be ignored.
					// It is like it does not exist.
			}
		}
	} 	// else: leave the object unassigned.
			// If replication is not admitted, we assign the object to one and only one repository.
			// To do so, we leverage the following code

	if ( assigned_repo == -1 )
	{
		// The object has not been assigned yet. We have to force it in some
		// repository
		double rand_num = dblrand();
		double accumulated_prob = 0;
		assigned_repo = 0;
		while (rand_num > accumulated_prob){
			accumulated_prob += probabilities[ assigned_repo ];
			assigned_repo++;
		}
		assigned_repo--;
		(*total_replicas_p) ++;
	}
	// The object will be assigned to the repo_idx-th repository.
	// Set the repo_idx-th bit in the binary string
	unsigned short repo_string = 0;
	repo_string |= 1 << assigned_repo; //http://stackoverflow.com/a/47990

	#ifdef SEVERE_DEBUG
		int num_1_bits =  __builtin_popcount (repo_string); //http://stackoverflow.com/a/109069
												// Number of bits set to 1 (corresponding 
												// to the number of repositories this object was 
												// assigned to)
		if ( assigned_repo > (int) num_repos-1 || assigned_repo < 0){
		    std::stringstream ermsg; 
			ermsg<<"assigned_repo="<<assigned_repo;
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		if (num_1_bits < 1){
		    std::stringstream ermsg; 
			ermsg<<"No repo has been assigned ";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}

		if (num_1_bits != 1 && !replication_admitted){
		    std::stringstream ermsg; 
			ermsg<<"repo_string="<<repo_string;
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	#endif

	//Update the repo_popularity
	(*repo_popularity_p)[assigned_repo] += 
			zipf[0]->get_normalization_constant() / pow(object_index,alpha);

	return repo_string;
}


