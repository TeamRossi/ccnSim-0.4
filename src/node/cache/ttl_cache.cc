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
#include "ttl_cache.h"
#include "statistics.h"

#include "error_handling.h"

Register_Class(ttl_cache);

void ttl_cache::initialize()
{
	cout << "-- INITIALIZATION TTL CACHE NODE # " << getParentModule()->getIndex() << ":"<< endl;

	base_cache::initialize();

	// *** Set parameters needed to measure to Actual Cache Occupancy of the main cache
    cModule* pSubModule = getParentModule()->getParentModule()->getSubmodule("statistics");
    statistics* pStatisticsModule = dynamic_cast<statistics*>(pSubModule);

    int down = pStatisticsModule->par("downsize");

    // ** IMPORTANT ** Downscaled MC-TTL simulation can be performed only if target_cache >= 10;
    //				   Smaller values make its measurement unstable, thus causing convergence problems.
    //				   It follows that the maximum downscaling factor is "cache_size/10".
    ASSERT2(down <= (cache_size/10), "Downscaling factor too high! The maximum value, in order to avoid measurement instability on the target cache (and consequent convergence problems), should be 'Cache_DIM/10'\n");

    target_cache = cache_size * (1./down);
    //cout << "*** TARGET CACHE: " << target_cache << endl;
    cout << "\t\t TARGET CACHE: " << target_cache << endl;
    avg_as_prev = 0.0;
    time_extend = SIMTIME_DBL(simTime());
    cycle_avg_meas_time = SIMTIME_DBL(simTime()) - time_extend;

   	//cout << "*** AVG MEAS TIME: " << cycle_avg_meas_time << endl;

    // TTL cache check initialization
    //ttl_check_timer = 0.2*tc_node; // Timer set to 20% of TC
    ttl_check_timer = 1.0;

    // Check if the meta-caching is 2-LRU. In this case, we need to schedule a double check: one for the main cache,
    // and the other for the name cache
    string decision_policy = getAncestorPar("DS");

    if (decision_policy.compare("two_ttl")==0)
    {
    	//cout << "*** TTL CACHE got the right decision policy! ***" << endl;
    	base_cache *contStore = (base_cache *)getParentModule()->getModuleByPath("content_store");
    	if(contStore)
    	{
    		//cout << "*** TTL CACHE got the right pointer to the content store! ***" << endl;
    		twoTTLDecisor = dynamic_cast<Two_TTL *> (contStore->get_decisor());
    		if(twoTTLDecisor)
    		{
    			ttl_check_msg = new cMessage("two_ttl_check", TWO_TTL_CHECK);
    			scheduleAt(simTime() + ttl_check_timer, ttl_check_msg);
    			// *** Set the Target Name Cache
    			twoTTLDecisor->set_target_name_cache(target_cache);
    		}
    		else
    		{
    			std::stringstream ermsg;
    			ermsg<<"ERROR - TTL CACHE: cannot retrieve the pointer to the decisor. Please check";
    			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
    		}
    	}
    	else
    	{
    		std::stringstream ermsg;
    		ermsg<<"ERROR - TTL CACHE: cannot retrieve the pointer to the content store. Please check";
    		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
    	}
    }
    // In case of decision policies that are different from two_lru, we just schedule a check for the main cache
    else
    {
    	ttl_check_msg = new cMessage("ttl_check", TTL_CHECK);
    	scheduleAt( simTime() + ttl_check_timer, ttl_check_msg );
    }
    //cout << "-- INITIALIZATION TTL CACHE -- DONE!!" << endl;
}

void ttl_cache::handleMessage(cMessage *in)
{
    if (in->isSelfMessage())	// A self-generated message can be either an 'ARRIVAL' or a 'TIMER'.
    {
		switch(in->getKind())
		{
		case TTL_CHECK:
			for (unordered_map<chunk_t,simtime_t>::iterator it = cache.begin();it != cache.end();)
			{
				if ( simTime() > it->second)  // The TTL of the selected content is expired
				{
					it = cache.erase(it);		 // Erase the content and update the actual size of the cache
					if(actual_size > 0)
						actual_size--;
				}
				else{
					it++;
				}
			}
			scheduleAt( simTime() + ttl_check_timer, ttl_check_msg );  // Schedule the next check
			//cout << simTime() << "\tTTL CHECK\tEXIT" << endl;
			break;



		case TWO_TTL_CHECK:
			// Check the name cache
			twoTTLDecisor->check_name_cache();
			// Check the main cache
			for (unordered_map<chunk_t,simtime_t>::iterator it = cache.begin();it != cache.end();)
			{
				if ( simTime() > it->second)  // The TTL of the selected content is expired
				{
					it = cache.erase(it);		 // Erase the content and update the actual size of the cache
					if(actual_size > 0)
						actual_size--;
				}
				else{
					it++;
				}
			}
			scheduleAt( simTime() + ttl_check_timer, ttl_check_msg );  // Schedule the next check
			//cout << simTime() << "\tTTL CHECK\tEXIT" << endl;
			break;
		default:
			std::stringstream ermsg;
			ermsg<<"ERROR - TTL CACHE: received wrong self message identifier. Please check";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
    }
}
void ttl_cache::finish()
{
	cache.clear();
	base_cache::finish();
	//cout << "NODE # " << getIndex() << " MAIN CACHE ACTUAL SIZE: " << actual_size << endl;
	cout << "NODE # " << getIndex() << " MAIN CACHE ONLINE AVG ACTUAL SIZE: " << avg_as_curr << endl;
	cout << "NODE # " << getIndex() << " MAIN MAX CACHE SIZE: " << max_as << endl;
	cout << "NODE # " << getIndex() << " MAIN CACHE Tc: " << tc_node << endl;

	string decision_policy = getAncestorPar("DS");

	if (decision_policy.compare("two_ttl")==0)
	{
		twoTTLDecisor->finish_name_cache();
	}
}

/*
 * 	TTL insertion handling. The new object is inserted at the head of the cache.
 *
 * 	Parameters:
 * 		- elem: content object to be cached.
 */
void ttl_cache::data_store(chunk_t elem)
{
	if (fake_lookup(elem))		// The object is already stored inside the cache.
    	return;

	cache[elem] = simTime() + tc_node; 		// Store the new object;
	//cout << "CACHE ENTRY SIZE: " << sizeof(cache[elem]) << " Bytes" << endl;
	actual_size++;
	if(actual_size > max_as)
		max_as = actual_size;
}

bool ttl_cache::fake_lookup(chunk_t elem){

	unordered_map<chunk_t,simtime_t>::iterator it = cache.find(elem);

	if (it==cache.end())	// The element is not found.
    	return false;
    else
    	return true;
}

/*
 * 	TTL lookup.
 * 	If the requested content is in the cache, the following check is performed:
 * 	-) if simTime() > evict_time  --> MISS: it means that the content has expired and it was not removed from the periodic
 * 											check of the TTL cache. So it is removed from the list.
 * 	-) otherwise --> HIT: the evict_time of the content is updated.
 */
bool ttl_cache::data_lookup(chunk_t elem)
{
    unordered_map<chunk_t,simtime_t>::iterator it = cache.find(elem);

    if (it==cache.end())	// The content object is not present inside the cache.
    {
    	//cout << simTime() << "\tMISS\tActual Cache Size:\t" << actual_size << endl;

		num_event_online_avg++;
		//if (num_event_online_avg == max_num_event_online_avg)
		if (dblrand() < 0.1)
		{
			cycle_curr_time = SIMTIME_DBL(simTime()) - time_extend;

			//avg_as_curr = (double)((avg_as_prev*avg_meas_time + actual_size*(SIMTIME_DBL(simTime())-avg_meas_time))*(1./SIMTIME_DBL(simTime())));
			avg_as_curr = (double)((avg_as_prev*cycle_avg_meas_time + actual_size*(cycle_curr_time-cycle_avg_meas_time))*(1./cycle_curr_time));

			//cout << simTime() << "\tNODE: " << getIndex() << "\n\tACTUAL: " << actual_size << "\n\tMEAN PREV: " << avg_as_prev << "\n\tMEAN CURR: " << avg_as_curr << "\n\tTIME PREV: " << cycle_avg_meas_time << "\n\tTIME CURR: " << cycle_curr_time  << endl;

			//if(abs(avg_as_curr - avg_as_prev)/avg_as_prev < 0.001)
			//	conv_online_avg = true;

			avg_as_prev = avg_as_curr;
			cycle_avg_meas_time = SIMTIME_DBL(simTime()) - time_extend;
			num_event_online_avg = 0;
		}
    	return false;
    }

    simtime_t evict_time = it->second;

    if(simTime() > evict_time)   // MIISS
    {
    	cache.erase(elem);
    	if(actual_size > 0)
    		actual_size--;
        return false;
    }
    else
    {
    	//cout << simTime() << "\tHIT\tActual Cache Size:\t" << actual_size << endl;
    	//cout << simTime() << "\tHIT\tENTER" << endl;
    	//cout << simTime() << "\tHIT\tEXIT" << endl;

        it->second = SIMTIME_DBL(simTime()) + tc_node;
    }
    return true;
}

void ttl_cache::flush()
{
	cache.clear();
	actual_size=0;
}

bool ttl_cache::full()
{
    return false;
}

void ttl_cache::extend_sim()
{
	if(change_tc)
		tc_node = tc_node + tc_node*(target_cache*(1./avg_as_curr) - 1);
	//avg_as_prev = target_cache;            		// Target size is used as initial value.
	//avg_meas_time = SIMTIME_DBL(simTime());
	time_extend = SIMTIME_DBL(simTime());
	cycle_avg_meas_time =  SIMTIME_DBL(simTime()) - time_extend;

	avg_as_prev = 0.0;
	num_event_online_avg = 0;
	flush();
	change_tc = true;
}
