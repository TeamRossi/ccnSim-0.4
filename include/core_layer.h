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

#ifndef CCN_NODE_H
#define CCN_NODE_H

#include <omnetpp.h>
#include "ccnsim.h"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
//#include "strategy_layer.h"

using namespace std;
using namespace boost;

class ccn_interest;
class ccn_data;

class strategy_layer;
class base_cache;


//	Struct reproducing a PIT entry.
struct pit_entry
{
    interface_t interfaces;			// Incoming interfaces.
    unordered_set<int> nonces;		// Nonces of the Interest packets aggregated inside the same PIT entry.
    simtime_t time; 				// Last update time of the PIT entry.
    std::bitset<1> cacheable;		// Bit indicating if the retrieved Data packet should be cached or not.
};


class core_layer : public abstract_node{
    friend class statistics;
    
    public:
    	void check_if_correct(int line);

    	#ifdef SEVERE_DEBUG
		bool it_has_a_repo_attached;

		vector<int> get_interfaces_in_PIT(chunk_t chunk);
		bool is_it_initialized;
		#endif

		double get_repo_price();
		int getOutInt(int dest);

		// *** Added for model execution with NRR
		virtual strategy_layer* get_strategy() const;

		bool stable;   // Used for collecting load samples only after the stabilization;
		double datarate;
		// *** Link Load Evaluation ***
		bool llEval;

    protected:
		virtual void initialize();
		virtual void handleMessage(cMessage *);
		virtual void finish();

		bool interest_aggregation;
		bool transparent_to_hops;
		double repo_price;
		void add_to_pit(chunk_t chunk, int gate);

		void handle_interest(ccn_interest *);
		void handle_ghost(ccn_interest *);
		void handle_data(ccn_data *);
		void handle_decision(bool *, ccn_interest *);


		bool check_ownership(vector<int>);
		ccn_data *compose_data(uint64_t);	
		void clear_stat();

		long catCard;

		// *** Link Load Evaluation ***
		//bool llEval;
		double maxInterval; // max length of measurement interval (measurement ends
		// if either batchSize or maxInterval is reached, whichever is reached first)


		double simDuration;

		// global statistics
		// With percentiles
		//double** numPackets;
		double** numBits; // double to avoid overflow
		// Wothout percentiles
		//double* numPackets;
		//double* numBits; // double to avoid overflow

		// current measurement interval
		// With percentiles
		//unsigned long** intvlNumPackets;
		//unsigned long** intvlNumBits;
		// Without percentiles
		//unsigned long* intvlNumPackets;
		//unsigned long* intvlNumBits;

		double percentiles [11] = {0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1};
		unsigned int* percID;
		int numPercentiles = 11;

    private:
		unsigned long max_pit;
		unsigned short nodes;
		unsigned int my_bitmask;
		double my_btw;
		double RTT;

		static int repo_interest; 	// Total number of Interest packets sent to the attached repository (if present)

		// Number of chunks satisfied by the attached repository (if present).
		int repo_load; 
	

		// Architecture data structures
		boost::unordered_map <chunk_t, pit_entry > PIT;
		base_cache *ContentStore;
		strategy_layer *strategy;

		// Statistics
		int interests;
		int data;

		int	send_data (ccn_data* msg, const char *gatename, int gateindex, int line_of_the_call);

		//*** Link Load Evaluation ***
		cMessage *load_check;
		void evaluateLinkLoad();


		#ifdef SEVERE_DEBUG
		int unsolicited_data;	// Data received by the node but not requested by anyone

		int discarded_interests; //number of incoming interests discarded
								 // because their TTL is > max hops
		int unsatisfied_interests;	//number of interests for contents that are neither
									//in the cache nor in the repository of this node	
		int interests_satisfied_by_cache;

		#endif
};
#endif

