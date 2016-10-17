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

#ifndef TTL_CACHE_H_
#define TTL_CACHE_H_
#include <boost/unordered_map.hpp>
#include "base_cache.h"
#include "two_ttl_policy.h"
#include "ccnsim.h"


using namespace std;
using namespace boost;

// A simple TTL cache is defined by using an unordered map (position is not as important as in LRU).
// Expired contents will be removed by means of a periodic check.
class ttl_cache:public base_cache
{
    friend class statistics;
    public:
		ttl_cache():base_cache(){;}
	
		void dump(){;};

		void flush();

		bool full();

    protected:
    	virtual void initialize();
		void data_store(chunk_t);
		bool data_lookup(chunk_t);
		bool fake_lookup(chunk_t);

		void extend_sim(); 			// Correct the Tc (TTL) value.

		virtual void handleMessage(cMessage *);

		void finish();

		virtual double get_tc_node(){;};

    private:
		uint32_t actual_size = 0; 		//	Actual size of the cache (# objects).
		double avg_as_curr;				//  Online avg of the actual cache size.
		double avg_as_prev;
		double target_cache;			// Target cache size of the downscaled system (C/Delta).
		double time_extend;				// Start time of a new cycle
		double avg_meas_time;			// Absolute measuring time of the last sample
		double cycle_avg_meas_time;		// Relative measuring time of the last sample (i.e., avg_meas_time - time_extended)
		double cycle_curr_time;			// Relative current time inside a cycle (i.e., simTime() - time_extended)


		unordered_map<chunk_t, simtime_t> cache; 	// Implemented LRU cache.
		cMessage *ttl_check_msg;
		simtime_t ttl_check_timer;

		uint32_t max_num_event_online_avg = 10;
		uint32_t num_event_online_avg = 0;

		bool change_tc = true;			// Connected to the statistic module in order o change tc only where needed
		bool conv_online_avg = false;

		uint32_t max_as = 0;

		Two_TTL* twoTTLDecisor;
};
#endif
