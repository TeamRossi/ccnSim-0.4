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
#ifndef TWO_LRU_POLICY_H_
#define TWO_LRU_POLICY_H_

#include "decision_policy.h"
#include "base_cache.h"
#include "lru_cache.h"

#include "error_handling.h"

/*
 * 2-LRU policy: although being a meta-caching algorithm, it requires nodes to allocate a second cache, namely
 * 				 Name Cache (always with LRU replacement), in order to keep track of the IDs of the received Interest packets.
 * 				 In case of a HIT inside the Name Cache, the retrieved Data packet will be cached in the normal
 * 				 cache (i.e., the one that contains real contents); otherwise, it will be just forwarded back.
 */

class Two_Lru: public DecisionPolicy
{
    public:
	Two_Lru(uint32_t cSize):ncSize(cSize){
		base_cache* bcPointer = new lru_cache();	// Create a new LRU cache that will act as a Name Cache.
		name_cache = dynamic_cast<lru_cache *> (bcPointer);
		name_cache->set_size(ncSize);}				// Set the size of the Name Cache.

	/*
	 * Cache decision of the 2-LRU. Since the flag that indicates the decision (cache or not) is present inside
	 * the PIT entry, and it has already been set by the core_layer, this function returns always true.
	 */
	virtual bool data_to_cache(ccn_data *){return true;}

	/*
	 *  Check the presence of the content ID inside the Name Cache, and eventually stores it.
	 */
	// *** WITHOUT TC MEASUREMENT ***
	/*bool name_to_cache(chunk_t chunk)
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
			name_cache->store_name(chunk);
			return false;
		}
	}*/

	// *** WITH TC MEASUREMENT ***
	bool name_to_cache(chunk_t chunk)
	{
		if (name_cache->lookup_name(chunk))
		{
			// The ID is already present inside the Name Cache, so update its position and return True.
			// As a consequence, the 'cacheable' flag inside the PIT will be set to 1.

			// HIT - Update the timestamp of the relative content
			if(nc_stable)
			{
				map<chunk_t, double>::iterator itHit = monitored_contents.find(chunk);
				if(itHit == monitored_contents.end())
				{
					//cout << "Trying to update a non existing entry!\n";
					//exit(1);
				}
				monitored_contents[chunk] = SIMTIME_DBL(simTime());
				//cout << "NODE # " << getParentModule()->getIndex() << " :hit on content # " << elem << endl;
			}

			return true;
		}
		else
		{
			// The ID is NOT present inside the Name Cache, so insert it and return False.
			// As a consequence, the 'cacheable' flag inside the PIT will be set to 0.

			// MISS - Insert the new element and log the Tc of the LRU one.
			if(nc_stable)
			{
				if(current_size == ncSize)  // Log the Tc of the LRU elem that will be discarded
				{
					chunk_t k = (name_cache->get_lru())->k;
					map<chunk_t, double>::iterator it = monitored_contents.find(k);
					if(it == monitored_contents.end())
					{
						//cout << simTime() << " :Cannot log Tc: entry for content # " << k << " is not present!\n";
						//exit(1);
					}
					else
					{
						double Tc  = SIMTIME_DBL(simTime()) - monitored_contents[k];
						monitored_contents.erase(k);
						//int nodeId = getParentModule()->getIndex();
						//cout << simTime() << "\tNODE\t" << nodeId << "\tContent\t" << k << "\tTc\t" << Tc << endl;
						tc_name_cache += Tc;
						tc_name_samples++;
					}
				}

				// Insert the new element inside the map
				map<chunk_t, double>::iterator it = monitored_contents.find(chunk);
				if(it == monitored_contents.end())
				{
					monitored_contents[chunk] = SIMTIME_DBL(simTime());
					//cout << "NODE # " << getParentModule()->getIndex() << " :caching content # " << elem << endl;
				}
				else
				{
					cout << "The entry is present despite the previous miss event!\n";
					exit(1);
				}
			}

			if(current_size < ncSize)
				current_size++;

			name_cache->store_name(chunk);
			return false;
		}
	}


	lru_cache* name_cache;

	double tc_name_cache = 0;
	double tc_name_samples = 0;
	map < chunk_t, double> monitored_contents;

	bool nc_stable = false;


    private:
	//lru_cache* name_cache;
	uint32_t ncSize;		// Size of the Name Cache in terms of number of content IDs.
	uint32_t current_size = 0;
};
#endif

