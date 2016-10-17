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

#include <iostream>
#include "lru_cache.h"
#include "two_lru_policy.h"

#include "error_handling.h"

Register_Class(lru_cache);

void lru_cache::finish()
{
	cache.clear();

	/*string decision_policy = getAncestorPar("DS");
	if (decision_policy.compare("two_lru")==0)
	{
		//Two_Lru* twoLruDecisor = dynamic_cast<Two_Lru *> (base_cache::get_decisor());
		Two_Lru* twoLruDecisor = (Two_Lru *) (base_cache::get_decisor());
		if(twoLruDecisor)
		{
			double tcNameCache = (double)(twoLruDecisor->name_cache->nodeTc/twoLruDecisor->name_cache->tcSamples);
			cout << "NODE # " << getParentModule()->getIndex() << " NAME CACHE Tc: " << tcNameCache << endl;
		}
	}*/

	// In case of 2-LRU, retrieve and print the Tc of the name cache
		string decision_policy = getAncestorPar("DS");
		if (decision_policy.compare("two_lru")==0)
		{
			//cout << "*** LRU CACHE got the right decision policy! ***" << endl;
			base_cache *contStore = (base_cache *)getParentModule()->getModuleByPath("content_store");
			if(contStore)
			{
				//cout << "*** LRU CACHE got the right pointer to the content store! ***" << endl;
				Two_Lru* twoLruDecisor = (Two_Lru *) (contStore->get_decisor());
				if(twoLruDecisor)
				{
					double tcNameCache = (double)((twoLruDecisor->tc_name_cache)/(twoLruDecisor->tc_name_samples));
					cout << "NODE # " << getParentModule()->getIndex() << " NAME CACHE LRU Tc: " << tcNameCache << endl;
				}

		    }
			else
			{
				std::stringstream ermsg;
				ermsg<<"ERROR - LRU CACHE: cannot retrieve the pointer to the content store. Please check";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
		}

	base_cache::finish();
	cout << "NODE # " << getParentModule()->getIndex() << " Evaluated Tc: " << (double)(nodeTc/tcSamples) << endl;
}

double lru_cache::get_tc_node()
{
	return (double)(nodeTc/tcSamples);
}

/*
 * 	LRU storage handling. The new object is inserted at the head of the cache.
 *
 * 	Parameters:
 * 		- elem: content object to be cached.
 */
void lru_cache::data_store(chunk_t elem)
{
    if (data_lookup(elem))		// The object is already stored inside the cache. Update its position and exit.
    	return;

    lru_pos *p = (lru_pos *)malloc (sizeof(lru_pos)); 	// Data structure for the position for the new element.

    p->k = elem;
    p->hit_time = simTime();
    p->newer = 0;
    p->older = 0;

    if (actual_size == 0)		// The cache is empty. Since this is the first object, lru = mru.
    {
        actual_size++;
        lru = mru = p;
        cache[elem] = p;

        // Monitoring the Tc of the first inserted content.
    	//if(simTime() > 10*3600)
		//{
        if(stability)
        {
			map<chunk_t, double>::iterator it = monitored_contents.find(elem);
			if(it == monitored_contents.end())
			{
				monitored_contents[elem] = SIMTIME_DBL(simTime());
				//cout << "NODE # " << getParentModule()->getIndex() << " :caching content # " << elem << endl;
			}
			else
			{
				//cout << "The entry is present despite the previous miss event!\n";
					exit(1);
			}
		}

        return;
    } 


    //	The cache is NOT empty. The new element is the newest, and it should be added in the front of the list
    p->older = mru; 	// The old MRU is swapped in second position.
    mru->newer = p; 	// The newer element of the old MRU is updated with the new inserted object.
    mru = p; 			// The actual MRU is updated.

    if (actual_size==get_size())	// If the cache is full, the LRU element should be dropped.
    {
        chunk_t k = lru->k;
        lru_pos *tmp = lru;
        lru = tmp->newer;		// The newer element of the old LRU is now the actual LRU.

        lru->older = 0;
        tmp->older = 0;
        tmp->newer = 0;

        free(tmp);
        cache.erase(k); 		// Drop the old LRU.

        // Logging the Tc for the erased content.
        //if(k < 200 && SIMTIME_DBL(simTime())>118.0)
        //{
        	//if(k < 20 || k > 100)		// Only the 10 most popular contents are monitored so far.
        	//if(simTime() > 10*3600)
        	//{
        if(stability)
        {
				map<chunk_t, double>::iterator it = monitored_contents.find(k);
				if(it == monitored_contents.end())
				{
					//cout << simTime() << " NODE # " << getParentModule()->getIndex() << " :Cannot log Tc: entry for content # " << k << " is not present!\n";
					//	exit(1);
				}
				else
				{
					double Tc  = SIMTIME_DBL(simTime()) - monitored_contents[k];
					monitored_contents.erase(k);
					//int nodeId = getParentModule()->getIndex();
					//cout << simTime() << "\tNODE\t" << nodeId << "\tContent\t" << k << "\tTc\t" << Tc << endl;
					nodeTc += Tc;
					tcSamples++;
				}
        }
			//}
        //}
    }
    else		// The cache is NOT full, so just update its size.
    	actual_size++;

    cache[elem] = p; 		// Store the new object with its position inside the map.

	// Inserting a new entry to monitor the Tc. A miss event means that the content has been previously
	// evicted, so there should be no correspondent entry inside the map.
    //if(elem < 200 && SIMTIME_DBL(simTime())>118.0)
    //{
    	//if(elem < 20 || elem > 100)		// Only the 10 most popular contents are monitored so far.
    	//if(simTime() > 10*3600)
		//{
    if(stability)
    {
			map<chunk_t, double>::iterator it = monitored_contents.find(elem);
			if(it == monitored_contents.end())
			{
				monitored_contents[elem] = SIMTIME_DBL(simTime());
				//cout << "NODE # " << getParentModule()->getIndex() << " :caching content # " << elem << endl;
			}
			else
			{
				//cout << "The entry is present despite the previous miss event!\n";
					exit(1);
			}
    }
		//}
    //}
}

lru_pos* lru_cache::get_mru(){
	return mru;
}

lru_pos* lru_cache::get_lru(){
	#ifdef SEVERE_DEBUG
	if (lru != NULL){
		// To see if a seg fault arises due to the access to a forbidden area
		// To use with valgrind software
		chunk_t test = lru->k;
	} //else the cache is empty
	#endif

	return lru;
}

const lru_pos* lru_cache::get_eviction_candidate(){
	if ( full() ) 
		return get_lru();
	else return NULL;
}

bool lru_cache::fake_lookup(chunk_t elem){

	unordered_map<chunk_t,lru_pos *>::iterator it = cache.find(elem);

	if (it==cache.end())	// The element is not found.
    	return false;
    else
    	return true;
}

/*
 * 	LRU lookup. In case of a hit, the position of the element should be updated.
 */
bool lru_cache::data_lookup(chunk_t elem)
{
    unordered_map<chunk_t,lru_pos *>::iterator it = cache.find(elem);

    if (it==cache.end())	// The content object is not present inside the cache.
    	return false;

    // Otherwise update its position.
    lru_pos* pos_elem = it->second;

    if (pos_elem->older && pos_elem->newer)		// The element is in the middle of the list.
    {
    	pos_elem->newer->older = pos_elem->older;
        pos_elem->older->newer = pos_elem->newer;
    }
    else if (!pos_elem->newer)		// The element is already the MRU. Do nothing.
    {
        return true;
    }
    else		// The element is the LRU. Remove it from the bottom of the list.
    {
        lru = pos_elem->newer;
        lru->older = 0;
    }

    //	Place the element in front of the list.
    pos_elem->older = mru;
    pos_elem->newer = 0;
    mru->newer = pos_elem;

    //	Update the MRU.
    mru = pos_elem;
    mru->hit_time = simTime();

	// Updating the Tc timer after a hit.
    //if(elem < 200 && SIMTIME_DBL(simTime())>118.0)
    //{
    	//if(elem < 20 || elem > 100)		// Only the 10 most popular contents are monitored so far.
    	//if(simTime() > 10*3600)
		//{
    if(stability)
    {
			map<chunk_t, double>::iterator itHit = monitored_contents.find(elem);
			if(itHit == monitored_contents.end())
			{
				//cout << "Trying to update a non existing entry!\n";
				//exit(1);
			}
			monitored_contents[elem] = SIMTIME_DBL(simTime());
			//cout << "NODE # " << getParentModule()->getIndex() << " :hit on content # " << elem << endl;
    }
		//}
    //}

    return true;
}

void lru_cache::dump()
{
    lru_pos *it = mru;
    int p = 1;
    while (it){
	cout<<p++<<" ]"<< __id(it->k)<<"/"<<__chunk(it->k)<<endl;
	it = it->older;
    }
}

void lru_cache::flush()
{
	cache.clear();
	actual_size=0;
	monitored_contents.clear();
}

bool lru_cache::full()
{
    return (actual_size==get_size());
}
