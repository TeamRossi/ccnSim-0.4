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
#include "content_distribution.h"

#include "statistics.h"

#include "ccn_interest.h"
#include "ccn_data.h"

#include "ccnsim.h"
#include "client.h"

#include "error_handling.h"
#include <random>

Register_Class (client);


void client::initialize()
{
	//cout << "Start Initializing CLIENT BASE ... \n";

	RTT = par("RTT");

	//Allocating file statistics
	//**mt** DISABLED
	//client_stats = new client_stat_entry[__file_bulk+1];

	// Initialize average statistics.
	avg_distance = 0;
	avg_time = 0;
	tot_downloads = 0;
	tot_chunks = 0;

	#ifdef SEVERE_DEBUG
	interests_sent = 0;
	#endif
}

int client::getNodeIndex()
{
    return gate("client_port$o")->getNextGate()->getOwnerModule()->getIndex();
}

void client::finish()
{
    //	Output average local statistics.
    if (active)
    {
    	char name [30];
		sprintf ( name, "hdistance[%d]", getNodeIndex());
		recordScalar (name, avg_distance);

		sprintf ( name, "downloads[%d]",getNodeIndex());
		recordScalar (name, tot_downloads );

		sprintf ( name, "avg_time[%d]",getNodeIndex());
		recordScalar (name, avg_time);

		#ifdef SEVERE_DEBUG
		sprintf ( name, "interests_sent[%d]",getNodeIndex());
		recordScalar (name, interests_sent );

		if (interests_sent != tot_downloads)
		{
			std::stringstream ermsg; 
			ermsg<<"interests_sent="<<interests_sent<<"; tot_downloads="<<tot_downloads<<
				". If **.size==1 in omnetpp and all links has 0 delay, this "<<
				" is an error. Otherwise, it is not. In the latter case, ignore "<<
				"this message";
			debug_message(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		#endif

		//Output per file statistics
		//sprintf ( name, "hdistance[%d]", getNodeIndex());
		//cOutVector distance_vector(name);

		//for (name_t f = 1; f <= __file_bulk; f++)
		//    distance_vector.recordWithTimestamp(f, client_stats[f].avg_distance);

		//**mt** DISABLED
		//delete [] client_stats;
	}
}

/*
 * 	Verifies if retransmissions are needed.
 */
void client::handle_timers(cMessage *timer)
{
	for (multimap<name_t, download >::iterator i = current_downloads.begin();i != current_downloads.end();i++)
	{
		if ( simTime() - i->second.last > RTT )
		{
			#ifdef SEVERE_DEBUG
			    chunk_t chunk = 0; 	// Allocate chunk data structure. 
									// This value wiil be overwritten soon
				name_t object_name = i->first;
				chunk_t object_id = __sid(chunk, object_name);
				std::stringstream ermsg; 
				ermsg<<"Client attached to node "<< getNodeIndex() <<" was not able to retrieve object "
					<<object_id<< " before the timeout expired. Serial number of the interest="<< 
					i->second.serial_number <<". This is not necessarily a bug. If you expect "<<
					"such an event and you think it is not a bug, disable this error message";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			#endif
			
		    //	Resend the request for the given chunk.
		    cout<<getIndex()<<"]**********Client timer hitting ("<<simTime()-i->second.last<<")************"<<endl;
		    cout<<i->first<<"(while waiting for chunk n. "<<i->second.chunk << ",of a file of "<< __size(i->first) <<" chunks at "<<simTime()<<")"<<endl;
		    resend_interest(i->first,i->second.chunk,-1);
	    }
	}
}


void client::resend_interest(name_t name,cnumber_t number, int toward)
{
    chunk_t chunk = 0;
    ccn_interest* interest = new ccn_interest("interest",CCN_I);
    __sid(chunk, name);
    __schunk(chunk, number);

    interest->setChunk(chunk);
    interest->setHops(-1);
    interest->setTarget(toward);
    interest->setNfound(true);
    send(interest, "client_port$o");

    #ifdef SEVERE_DEBUG
	std::stringstream ermsg; 
	ermsg<<"An error occurred. This is not necessarily a bug. If you "<<
		"think that a retransmission is plausible in your scenario, "<<
		"please disable this error and run again";
	severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
    #endif
}

void client::send_interest(name_t name,cnumber_t number, int toward)
{
    chunk_t chunk = 0;
    ccn_interest* interest = new ccn_interest("interest",CCN_I);

    __sid(chunk, name);
    __schunk(chunk, number);

    interest->setChunk(chunk);
    interest->setHops(-1);
    interest->setTarget(toward);

	#ifdef SEVERE_DEBUG
	interest->setSerialNumber(interests_sent);
	interest->setOrigin( getNodeIndex() );
	interests_sent++;
	#endif

    send(interest, "client_port$o");
}



void client::handle_incoming_chunk (ccn_data *data_message)
{
    cnumber_t chunk_num = data_message -> get_chunk_num();
    name_t name = data_message -> get_name();
    filesize_t size = data_message -> get_size();

	#ifdef SEVERE_DEBUG
		if ( !is_waiting_for(name) )
		{
			std::stringstream ermsg; 
			ermsg<<"Client attached to node "<< getNodeIndex() <<" is receiving object "
				<<name<<" but it is not waiting for it" <<endl;
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		

		if (client_stats == 0)
		{
			std::stringstream ermsg;
			ermsg<<"The pointer client_stats points to 0. This is an error";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	#endif

    //----------Statistics-----------------
    // Update average statistics.
    avg_distance = (tot_chunks*avg_distance+data_message->getHops())/(tot_chunks+1);
    tot_downloads+=1./size;

    // Statistics for each file. Only those regarding the specified part of the catalog
    // (i.e., the most '__file_bulk' popular contents are gathered.
    //**mt** DISABLED
    /*if (name <= __file_bulk){

        client_stats[name].avg_distance =
				( client_stats[name].tot_chunks*client_stats[name].avg_distance+data_message->getHops() )/
				( client_stats[name].tot_chunks+1 );
        client_stats[name].tot_chunks++;
        client_stats[name].tot_downloads+=1./size;
    }*/


    //-----------Handling downloads------
    pair< multimap<name_t, download>::iterator, multimap<name_t, download>::iterator > ii;
    multimap<name_t, download>::iterator it; 

    ii = current_downloads.equal_range(name);
    it = ii.first;

    while (it != ii.second)
	{
        if ( it->second.chunk == chunk_num )
		{
            it->second.chunk++;
            if (it->second.chunk< __size(name) )
			{ 
		    	it->second.last = simTime();
		    	// If the file is not completed yet, send the next Interest.
		    	send_interest(name, it->second.chunk, data_message->getTarget());
            }
            else
            {
	        	// Delete the entry related to the completed file from the list.
				simtime_t completion_time = simTime()-it->second.start;
				avg_time = (tot_chunks * avg_time + completion_time ) * 1./( tot_chunks+1 );
		    	if (current_downloads.count(name)==1)
				{
		    	    current_downloads.erase(name);
		    	    break;
		    	}
		    	else
		    	{
		    	    current_downloads.erase(it++);
		    	    continue;
		    	}
			}
        }
        ++it;
    }
    tot_chunks++;
}

void client::clear_stat(){
	#ifdef SEVERE_DEBUG
		if (client_stats == 0)
		{
			std::stringstream ermsg;
			ermsg<<"The pointer client_stats points to 0. This is an error";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	#endif


    avg_distance = 0;
    avg_time = 0;
    tot_downloads = 0;
    tot_chunks = 0;

    #ifdef SEVERE_DEBUG
    interests_sent = 0;
    #endif

    //**mt** DISABLED
    /*delete [] client_stats;
    client_stats = new client_stat_entry[__file_bulk+1];*/

	#ifdef SEVERE_DEBUG
		if (client_stats == 0)
		{
			std::stringstream ermsg;
			ermsg<<"The pointer client_stats points to 0. This is an error";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	#endif
}

double client::get_avg_distance()
{
	return avg_distance;
}
double client::get_tot_downloads()
{
	return tot_downloads;
}
simtime_t client::get_avg_time()
{
	return avg_time;
}
bool client::is_active()
{
	return active;
}

#ifdef SEVERE_DEBUG
unsigned int client::get_interests_sent(){
	return interests_sent;
}

bool client::is_waiting_for(name_t name)
{
	multimap<name_t, download>::iterator it = current_downloads.find(name);
	return it != current_downloads.end();
}
#endif

/*
 * 	Get the number of scheduled requests for popularity class 'cNum' (only for Shot Noise).
 */
double client::getScheduledReq(int cNum)
{
	return scheduledReq[cNum];
}

/*
 * 	Get the number of scheduled requests for popularity class 'cNum' (only for Shot Noise).
 */
double client::getValidatedReq(int cNum)
{
	return validatedReq[cNum];
}

double client::getLambda()
{
	return lambda;
}
