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
#ifndef TWO_CACHE_H_
#define TWO_CACHE_H_

#include "base_cache.h"
#include <deque>
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;

/*Power of two replacement: elements are pushed back into the cache If the
 * cache if filled replacement is fulfilled in this way:
 *    a) two random elements are taken from the cache 
 *    b) the "most popular" (out of the two) element is replaced.
*/
class two_cache: public base_cache{
    public:

	virtual void data_store(chunk_t);
	virtual bool data_lookup(chunk_t);
	virtual double get_tc_node(){;};
	virtual bool full();

    private:
	deque<uint64_t> deq;
	unordered_map<uint64_t,bool> cache;
};
#endif
