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
#include "ttl_name_cache.h"
#include "statistics.h"

#include "error_handling.h"

Register_Class(ttl_name_cache);

void ttl_name_cache::initialize_name_cache(double tcNode)
{
	//cout << "-- INITIALIZATION TTL NAME CACHE --" << endl;

	tc_name_node = tcNode;

	avg_as_prev = 0.0;
	time_extend = SIMTIME_DBL(simTime());
	cycle_avg_meas_time = SIMTIME_DBL(simTime()) - time_extend;

	//cout << "*** AVG MEAS TIME NAME CACHE: " << cycle_avg_meas_time << endl;

    //cout << "-- INITIALIZATION TTL NAME CACHE -- DONE!!" << endl;
}

void ttl_name_cache::check_cache()
{
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

}


void ttl_name_cache::finish_name_cache()
{
	cache.clear();
	//cout << "NODE # " << getIndex() << " NAME CACHE ACTUAL SIZE: " << actual_size << endl;
	//cout << "NODE # " << getIndex() << " Tc: " << tc_node << endl;

	//cout << "NODE # " << getIndex() << " NAME CACHE ACTUAL SIZE: " << actual_size << endl;
	cout << "NODE # " << getIndex() << " NAME CACHE ONLINE AVG ACTUAL SIZE: " << avg_as_curr << endl;
	cout << "NODE # " << getIndex() << " NAME CACHE Tc: " << tc_name_node << endl;
}

/*
 * 	TTL insertion handling. The new object is inserted at the head of the cache.
 *
 * 	Parameters:
 * 		- elem: content object to be cached.
 */
void ttl_name_cache::data_store(chunk_t elem)
{

	if (fake_lookup(elem))		// The object is already stored inside the cache.
    	return;

    cache[elem] = simTime() + tc_name_node; 		// Store the new object;
    actual_size++;
}

bool ttl_name_cache::fake_lookup(chunk_t elem){

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
bool ttl_name_cache::data_lookup(chunk_t elem)
{
    unordered_map<chunk_t,simtime_t>::iterator it = cache.find(elem);

    if (it==cache.end())	// The content object is not present inside the cache.
    {
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
        it->second = SIMTIME_DBL(simTime()) + tc_name_node;
    return true;
}

void ttl_name_cache::flush()
{
	cache.clear();
	actual_size=0;
}

bool ttl_name_cache::full()
{
    return false;
}

void ttl_name_cache::extend_sim()
{
	//if(change_tc)
	if(abs(target_name_cache - avg_as_curr)/target_name_cache > 0.1)
		tc_name_node = tc_name_node + tc_name_node*(target_name_cache*(1./avg_as_curr) - 1);
	time_extend = SIMTIME_DBL(simTime());
	cycle_avg_meas_time =  SIMTIME_DBL(simTime()) - time_extend;

	avg_as_prev = 0.0;
	flush();
}
