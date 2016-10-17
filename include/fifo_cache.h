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
#ifndef FIFO_CACHE_H_
#define FIFO_CACHE_H_

#include "base_cache.h"
#include "ccnsim.h"
#include <deque>
#include <boost/unordered_map.hpp>
using namespace std;
using namespace boost;


/*
 * FIFO replacement cache: each new chunk is pushed in front of the cache and
 * the back element is evicted.
 */
class fifo_cache: public base_cache
{
	friend class statistics;
    public:
		fifo_cache():base_cache(),actual_size(0){;}

		double nodeTc = 0;
		double tcSamples = 0;

		bool full();
		void dump();
		void flush();

		chunk_t get_toErase();   		  // Get the chunk to be erased if the cache is full.

		bool check_if_eraseElement(chunk_t);    // Check if the number of replicas inside the deque is zero

	//Polymorphic methods
    protected:
		void data_store (chunk_t);
		bool data_lookup (chunk_t);
		bool fake_lookup(chunk_t);

		double get_tc_node();


		void finish();

    private:
		uint32_t actual_size; 				//	Actual size of the cache (# objects).
		deque<chunk_t> deq;					//	Deque for the order
		unordered_map<chunk_t,int> cache;	//	Map for a look up

		// Collect info about the Tc
		map < chunk_t, double> monitored_contents;


};
#endif
