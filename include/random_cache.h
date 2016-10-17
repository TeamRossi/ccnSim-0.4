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
#ifndef R_CACHE_H_
#define R_CACHE_H_

#include "base_cache.h"
#include <boost/unordered_map.hpp>
#include <omnetpp.h>
#include <deque>

using namespace std;
using boost::unordered_map;

/* Random cache: new elements are pushed back in the cache when the cache is not
 * full.  Otherwise an element is randomly replaced by the incoming one.
 */

class random_cache: public base_cache{
    protected:
	virtual void initialize();

	//Polymorphic functions
	bool data_lookup(chunk_t);
	void data_store(chunk_t);
	virtual double get_tc_node(){;};
	bool full();

	//Deprecated
	bool warmup();

    private:
	deque<chunk_t> deq;
	unordered_map<chunk_t, bool> cache;

};
#endif
