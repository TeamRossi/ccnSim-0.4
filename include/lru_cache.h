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

#ifndef LRU_CACHE_H_
#define LRU_CACHE_H_
#include <boost/unordered_map.hpp>
#include "base_cache.h"
#include "ccnsim.h"


using namespace std;
using namespace boost;


//	Struct used to keep track the position of an element inside the lru cache.
//  In case of a hit, the element will be removed from the current position and inserted at the head of the list.
struct lru_pos
{
    lru_pos* older;			// Immediately least recently used element with respect to the current one.
    lru_pos* newer;			// Immediately most recently used element with respect to the current one.
    chunk_t k;				// Content name of the current element.
    simtime_t hit_time;		// Time of the hit event.
	double cost; 			// Used only with cost aware caching.
};

//	A simple LRU cache is defined by using a map and a list of positions within the map.
class lru_cache:public base_cache
{
    friend class statistics;
    public:
		lru_cache():base_cache(),actual_size(0),lru(0),mru(0){;}

		lru_pos* get_mru();
		lru_pos* get_lru();
		const lru_pos* get_eviction_candidate();
	
		bool full();
		void dump();

		void flush();

		double nodeTc = 0;
		double tcSamples = 0;


    protected:
		void data_store(chunk_t);
		bool data_lookup(chunk_t);
		bool fake_lookup(chunk_t);
		double get_tc_node();

		void finish();


    private:
		uint32_t actual_size; 	//	Actual size of the cache (# objects).
		lru_pos* lru; 			//	Actual Least Recently Used object.
		lru_pos* mru; 			//	Actual Most Recently Used object.

		unordered_map<chunk_t, lru_pos*> cache; 	// Implemented LRU cache.

		// Collect info about the Tc
		map < chunk_t, double> monitored_contents;
		//double nodeTc = 0;
		//double tcSamples = 0;
};
#endif
