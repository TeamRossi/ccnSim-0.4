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
#ifndef STATISTICS_H_
#define STATISTICS_H_
#include <omnetpp.h>
#if OMNETPP_VERSION < 0x0500
    #include <ctopology.h>
#else
    #include <omnetpp/ctopology.h>
#endif
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <vector>
#include <chrono>


class client;
class core_layer;
class base_cache;



using namespace std;
using namespace boost;

#if OMNETPP_VERSION >= 0x0500
    using namespace omnetpp;
#endif

/*
 * This is the central class for managing statistics collection.
 */
class statistics : public cSimpleModule{

	public:
		virtual void registerIcnChannel(cChannel* icn_channel);

		void cacheFillNaive(); 					// Fill the caches with |cache_size| most popular contents.

		void checkStability();

		int downsize;

		void cacheFillModel_Scalable_Approx(const char*);
		void cacheFillModel_Scalable_Approx_NRR(const char*);


		double stabilization_time;

    protected:
		virtual void initialize(int stage);		// Multi-stage initialization.
		int numInitStages() const;

		virtual void handleMessage(cMessage *);
		virtual void finish();

		virtual bool stable(int);		// Check if the hit rate of the specified cache is stable.

		void clear_stat();		// Each component (cache, client, etc) is asked to clear its statistics.
	
		void stability_has_been_reached();

		// Added for hybridization
		double calculate_phit_neigh (int, int, float**, float**, float**, double*, double, double, long, bool*, vector<vector<map<int,int> > > &);	// Calculate the phit of the neighbor using the conditional probabilities.
		double MeanSquareDistance(uint32_t, double **, double **, int);
		double calculate_phit_neigh_scalable (int, int, float**, float**, float**, double*, double, double, long, bool*, vector<map<int,int> > &);	// Calculate the phit of the neighbor using the conditional probabilities.


    private:
		cMessage *full_check;			// Scheduled message to check the cache occupation.
		cMessage *stable_check;			// Scheduled message to check the stability of the hit rate.
		cMessage *end;

		//	Vectors to access statistics of the different modules.
		client** clients;
		core_layer** cores;
		base_cache** caches;

		vector<cChannel*> icn_channels;
	
		// Network information
		int num_nodes;
		int num_clients;
		int num_repos;

		// Supported Meta-Caching Algorithms for Model Execution
		typedef enum {LCE,fixP} policy;
		policy meta_cache;
		const char *dpString;

		double q;			// caching probability of the fixP algorithm;

		// Stabilization parameters
		double ts;						// Frequency of stable_check message.
		double window;					// Time window for stability checking [s].
		double partial_n;				// Number of nodes whose state is checked.
		double time_steady;				// Simulation time after stabilization is reached.
		//double stabilization_time;

		double variance_threshold;		// Threshold under which the hit rate stability is checked.

		//	Stabilization samples
		vector< vector <double> > samples;
		vector<double> events; 	// Takes track of the changes (hit or miss) of each node.
		vector<bool> stable_nodes;
		unordered_map <int, unordered_set <int> > level_union;
		unordered_map <int, int> level_same;

		int total_replicas;

		vector<int> clients_id;

		int sim_model;				// Binary value indicating the execution of the analytical model instead of the sim transient time.

		// Timestamps
		chrono::high_resolution_clock::time_point tStartGeneral;
		chrono::high_resolution_clock::time_point tStartHot;
		chrono::high_resolution_clock::time_point tStartCold;
		chrono::high_resolution_clock::time_point tEndHot;
		chrono::high_resolution_clock::time_point tEndFill;
		chrono::high_resolution_clock::time_point tEndStable;
		chrono::high_resolution_clock::time_point tEndGeneral;

		bool scheduleEnd;
		bool onlyModel;

		int sim_cycles = 1; 			// Track the number of simulation cycles
		bool dynamic_tc = true;
                
		double cvThr;                   // Threshold to compare the Coefficient of Variation (CV) against.
                double consThr;                 // Consistency Check threshold; (default = 0.1)
};
#endif
