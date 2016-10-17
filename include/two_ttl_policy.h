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
#ifndef TWO_TTL_POLICY_H_
#define TWO_TTL_POLICY_H_

#include "decision_policy.h"
#include "base_cache.h"
//#include "lru_cache.h"
#include "ttl_name_cache.h"

#include "error_handling.h"

/*
 * 2-TTL policy: although being a meta-caching algorithm, it requires nodes to allocate a second cache, namely
 * 				 Name Cache (always with TTL replacement), in order to keep track of the IDs of the received Interest packets.
 * 				 In case of a HIT inside the Name Cache, the retrieved Data packet will be cached in the normal
 * 				 cache (i.e., the one that contains real contents); otherwise, it will be just forwarded back.
 */

class Two_TTL: public DecisionPolicy
{
    public:
	// WITH TTL
	Two_TTL(double tc_nameNode):tcName(tc_nameNode){
			base_cache* bcPointer = new ttl_name_cache();	// Create a new TTL cache that will act as a Name Cache.
			name_cache = dynamic_cast<ttl_name_cache *> (bcPointer);
			name_cache->initialize_name_cache(tcName);}				// Set the size of the Name Cache.

	/*
	 * Cache decision of the 2-LRU. Since the flag that indicates the decision (cache or not) is present inside
	 * the PIT entry, and it has already been set by the core_layer, this function returns always true.
	 */
	virtual bool data_to_cache(ccn_data *){return true;}

	void set_target_name_cache(double tnc)
	{
		name_cache->set_target_name_cache(tnc);
	}


	/*
	 * Check the ttl name cache
	 */
	void check_name_cache()
	{
		name_cache->check_cache();
	}


	/*
	 *  Check the presence of the content ID inside the Name Cache, and eventually stores it.
	 */
	bool name_to_cache(chunk_t chunk)
	{
		if (name_cache->lookup_name(chunk))
		{
			// The ID is already present inside the Name Cache, so update its position and return True.
			// As a consequence, the 'cacheable' flag inside the PIT will be set to 1.
			return true;
		}
		else
		{
			// The ID is NOT present inside the Name Cache, so insert it and return False.
			// As a consequence, the 'cacheable' flag inside the PIT will be set to 0.
			//name_cache->store_name(chunk);
			//cout << simTime() << "\t** NAME CACHE **\t" << "Caching name: " << chunk << endl;
			name_cache->store_name_ttl(chunk);
			//cout << simTime() << "\t** NAME CACHE **\t" << "Actual size: " << name_cache->get_cache_size() << endl;
			return false;
		}
	}

	void extend_sim()	// Correct the Tc (TTL) value.
	{
		//name_cache->flush();
		//name_cache->tc_name_node = newTcName;
		name_cache->extend_sim();
	}

	void finish_name_cache()
	{
		name_cache->finish_name_cache();
	}

    private:
	//lru_cache* name_cache;
	//uint32_t ncSize;		// Size of the Name Cache in terms of number of content IDs.
	ttl_name_cache* name_cache;
	double tcName;			// Tc of the name cache
};
#endif

