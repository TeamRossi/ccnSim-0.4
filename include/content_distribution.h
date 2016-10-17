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
#ifndef CONTENT_DISTRIBUTION_H
#define CONTENT_DISTRIBUTION_H
#include <omnetpp.h>
#include "ccnsim.h"
#include "zipf.h"
#include "zipf_sampled.h"


#pragma pack(push)
#pragma pack(1)
//
//This structure is very critical in terms of space. 
//In fact, it accounts for the startup memory requirement
//of the simulator, and should be keep as small as possible.
//
//
struct file{
    info_t info;
};
#pragma pack(pop)


using namespace std;



class content_distribution : public cSimpleModule{
    protected:
		virtual void initialize();
		void handleMessage(cMessage *){;}

		virtual unsigned short choose_repos(int object_index);
		virtual void initialize_repo_popularity();

		virtual void finalize_total_replica();

		#ifdef SEVERE_DEBUG
		virtual void verify_replica_number();
		#endif

		virtual vector<unsigned short> binary_strings(int,int);
		int replicas; 	// Number of replicas for each object. If set to -1, the value will be ignored.
		long cardF;
		long newCardF; 	// Downsized catalog in case of TTL-based scenario.



    public:
		void init_content();
		int *init_repos(vector<int>);
		virtual double *init_repo_prices();
		int *init_clients(vector<int>);


		static vector<file> catalog;
		//static zipf_distribution zipf;
		static zipf_sampled* zipf;

		static name_t perfile_bulk;			// Content ID after which per file statistics will not be gathered.
		static name_t stabilization_bulk; 
		static name_t cut_off;

		static int  *repositories;	 	// repositories[i] = d means that the i-th repository is attached to node[d].
		static double  *repo_prices; 	// repo_prices[i] is the price associated to the i-th repo.
		static int  *clients;

		static int *total_replicas_p; 	// Number of replicas that are distributed among all the repos.

		static vector<double>* repo_popularity_p;  // Value associated to each repository representing the sum
												   // of the popularity of the contained objects.

		static int num_repos;

    private:
		vector<unsigned short> repo_strings; 	// Temporary variable used to generate content displacement among repos.
		
		int num_clients;
		int nodes;
		int F;

		double alpha;
		double q;

		double perc_aggr;
		double stat_aggr;    // It specifies the percentile of the content catalog for which statistics are gathered.

};
#endif
