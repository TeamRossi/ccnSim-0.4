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
#include "fifo_cache.h"
#include <iostream>

#include "error_handling.h"


Register_Class(fifo_cache);

void fifo_cache::data_store(chunk_t chunk)
{
   unordered_map<chunk_t, int>::iterator it = cache.find(chunk);
   if(it == cache.end())
	   cache[chunk] = 1;
   else
   {
	   cache[chunk] += 1;
	   //cout << "NC - Content: " << chunk << "\t #replicas: " << cache[chunk] << endl;
   }
   //cache[chunk] = true;

   deq.push_back(chunk);

   if(stability)
   {
	   map<chunk_t, double>::iterator it = monitored_contents.find(chunk);
	   if(it == monitored_contents.end())
	   {
		   monitored_contents[chunk] = SIMTIME_DBL(simTime());
		   //cout << "NODE # " << getParentModule()->getIndex() << " :caching content # " << elem << endl;
	   }
	   else
	   {
		   //cout << "The entry is present despite the previous miss event!\n";
		   exit(1);
	   }
   }


   if ( deq.size() > get_size() )
   {
	   //Eviction of the last element
       chunk_t toErase = deq.front();
       deq.pop_front();

       cache[toErase] -= 1;

       if(cache[toErase] == 0)// Erase the content from the cache only when all its replicas have been evicted
       {
    	   cache.erase(toErase);

    	   if(stability)
    	   {
    		   map<chunk_t, double>::iterator it = monitored_contents.find(toErase);
    		   if(it == monitored_contents.end())
    		   {
    			   //cout << simTime() << " NODE # " << getParentModule()->getIndex() << " :Cannot log Tc: entry for content # " << k << " is not present!\n";
    			   //	exit(1);
    		   }
    		   else
    		   {
    			   double Tc  = SIMTIME_DBL(simTime()) - monitored_contents[toErase];
    			   monitored_contents.erase(toErase);
    			   //int nodeId = getParentModule()->getIndex();
    			   //cout << simTime() << "\tNODE\t" << nodeId << "\tContent\t" << k << "\tTc\t" << Tc << endl;
    			   nodeTc += Tc;
    			   tcSamples++;
    		   }
    	   }
       }
   }

}


bool fifo_cache::data_lookup(chunk_t chunk)
{
	unordered_map<chunk_t, int>::iterator it = cache.find(chunk);
	if (it==cache.end())	// The content object is not present inside the cache.
		return false;
	/*else					// ** NB Should we update the Tc? Or not?
							//    If the goal is to measure the sojourn time of a content inside the cache
							//    (regardless of the particular instance of the cached copy) no.
	{
		if(stability)
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
	}*/
	return true;
}


bool fifo_cache::full(){
    return (cache.size() == get_size());
}

double fifo_cache::get_tc_node()
{
	return (double)(nodeTc/tcSamples);
}

void fifo_cache::finish()
{
	cache.clear();

	base_cache::finish();
	cout << "NODE # " << getParentModule()->getIndex() << " Evaluated Tc: " << (double)(nodeTc/tcSamples) << endl;
}

bool fifo_cache::fake_lookup(chunk_t elem){

	unordered_map<chunk_t,int>::iterator it = cache.find(elem);

	if (it==cache.end())	// The element is not found.
    	return false;
    else
    	return true;
}

void fifo_cache::flush()
{
	cache.clear();
	actual_size=0;
	monitored_contents.clear();
}


void fifo_cache::dump()
{
	int p = 1;
	while(deq.size()!=0)
	{
		cout<<p++<<" ]" << deq.front() << endl;
		deq.pop_front();
	}
}

chunk_t fifo_cache::get_toErase()
{
	return deq.front();
}

bool fifo_cache::check_if_eraseElement(chunk_t k)
{
	if(cache[k] == 1)
		return true;
	else
		return false;
}
