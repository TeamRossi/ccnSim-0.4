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
#include "client_Window.h"

#include "error_handling.h"
#include <random>

Register_Class (client_Window);


void client_Window::initialize()
{
	int num_clients = getAncestorPar("num_clients");
	active = false;
	if (find(content_distribution::clients, content_distribution::clients + num_clients ,getNodeIndex()) != content_distribution::clients + num_clients)
	{
		active = true;
		//cout << "Initializing IRM client...\n";
		lambda = getAncestorPar("lambda");
		check_time	= getAncestorPar("check_time");
		arrival = new cMessage("arrival", ARRIVAL );
		//scheduleAt( simTime() + exponential(1./lambda), arrival);
		scheduleAt( simTime() + 1./lambda, arrival);
		//scheduleAt( simTime() + uniform(0,1./lambda), arrival);
		// ** NB ** Disable check for retransmission.
		timer = new cMessage("timer", TIMER);
		scheduleAt( simTime() + check_time, timer );

		defWinSize = par("defWinSize");
		maxWinSize = par("maxWinSize");
		currWinSize = defWinSize;

		// Being an open-loop flow control, we set a deterministic timer in order to reset the size of the window,
		// depending on the maximum size achievable for the window itself.
		winTout = 0.0;
		for(int i=1; i<=maxWinSize; i++)
			winTout += i*(1./lambda);
		winTimeout = new cMessage("winTimeout", WinTO);
		scheduleAt( simTime() + winTout - 1.0/lambda, winTimeout);

		client::initialize();

		/*
		 * Possible option to set a separate timer for each content that will be simulated using the TTL-based caches.
		 * Their original lambas will be used..in this case it won't be necessary anymore to rely on
		 * the zipf vector to establish which content to take
		 */
		/*for(int x=0; x<numOfApps; x++)
		{
		cMessage *timer = new cMessage("timerMsg-unique-string",  x);
		scheduleAt(simTime()+someValue, timer);
		}*/
	}
}

void client_Window::finish()
{
	client::finish();
}

void client_Window::handleMessage(cMessage *in)
{
    if (in->isSelfMessage())	// A self-generated message can be either an 'ARRIVAL' or a 'TIMER'.
    {
		switch(in->getKind())
		{
		case ARRIVAL:
			request_file_window(0);   // Default class num is '0' for client IRM.

			// ** NB ** Requests will be scheduled at the reception of Data packets, and only when inFlightPkts = 0.

			//scheduleAt( simTime() + exponential(1./lambda), arrival );
			//scheduleAt( simTime() + exponential(1./lambda), arrival );
			//scheduleAt( simTime() + 1./lambda, arrival);
			//scheduleAt( simTime() + uniform(0,1./lambda), arrival);
			break;
		case TIMER:
			handle_timers(in);
			scheduleAt( simTime() + check_time, timer );
			break;
		case WinTO:
			// Reset the window size to its default value
			if (getNodeIndex() == 7)
			{
				cout << SIMTIME_DBL(simTime()) << "\t *** Window TO - Reset to Default Value ***" << endl;
			}

			currWinSize = defWinSize;
			scheduleAt( simTime() + winTout - 1.0, winTimeout);
			break;
		default:
			 std::stringstream ermsg;
			 ermsg<<"ERROR - Client IRM: received wrong self message identifier. Please check";
			 severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		 }
		return;
    }

    switch (in->getKind())		// In case of an external message, it can only be a DATA packet.
 	{
 		case CCN_D:
 		{
 			#ifdef SEVERE_DEBUG
 			if (!active){
 				std::stringstream ermsg;
 				ermsg<<"Client attached to node "<<getNodeIndex()<<
 					"  received a DATA despite being NOT ACTIVE";
 				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
 			}
 			#endif

 			ccn_data *data_message = (ccn_data *) in;
 			handle_incoming_chunk (data_message);
 			delete  data_message;


 			// Decrease the inFlightPkts and check if an increment of the window size (along with a new batch request) is allowed.
 			/*if (getNodeIndex() == 7)
 			{
 				cout << SIMTIME_DBL(simTime()) << "\t # In-Flight Packets = " << inFlightPkts << endl;
 			}*/
 			inFlightPkts--;
 			if(inFlightPkts == 0) // All the Data packets of the previous Window have been received.
 			{
 				scheduleAt( simTime() + currWinSize/lambda - (0.1/lambda)*currWinSize, arrival);
 				if(currWinSize < maxWinSize)
 					currWinSize++;
 				/*if (getNodeIndex() == 7)
 				{
 					cout << SIMTIME_DBL(simTime()) << "\t Scheduling Requests for WIN SIZE = " << currWinSize << endl;
 				}*/
 			}

 			break;
 		}

 		#ifdef SEVERE_DEBUG
 		default:
 			std::stringstream ermsg;
 			ermsg<<"Clients can only receive DATA, while this is a message"<<
 				" of TYPE "<<in->getKind();
 			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
 		#endif
    }
}

void client_Window::request_file_window(unsigned long cNum)
{
	if (getNodeIndex() == 7)
	{
		cout << SIMTIME_DBL(simTime()) << "\t Requests with WIN SIZE = " << currWinSize << endl;
	}

	vector<name_t> reqContents;
	reqContents.push_back(0);
	bool found = true;

	for (int i=1; i<=currWinSize; i++)
	{
		name_t name;
		while(found)
		{
			//name = content_distribution::zipf.value(dblrand());
			name = content_distribution::zipf[0]->sample();  // With Rejection-inversion sampling
			if(find(reqContents.begin(), reqContents.end(), name) == reqContents.end())
				found = false;
		}

		found = true;

		reqContents.push_back(name);

		// ****  NB - COMMENT with Rejection-Inversion sampling
		/*// Check if the catalog aggregation is active. In this case, request for less popular contents
		// after a specified percentile are generated by randomly selecting an ID within that range.
		if (content_distribution::zipf.get_perc_aggr() < 1.0)
		{
			if (name == content_distribution::zipf.get_ID_aggr()+1) // In case of catalog aggregation, the vector
																	// containing the Zipf cdf is truncated, so every
																	// content after the specified threshold will provide
																	// the same ID.
			{
				// Extract a random content from the part of the catalog that has been aggregated, that is [name, cat_card].
				name = std::floor((rand() / ((double)RAND_MAX+1))*(content_distribution::zipf.get_catalog_card() - name + 1) + name);
			}
		}*/


		struct download new_download = download (0,simTime() );
		current_downloads.insert(pair<name_t, download >(name, new_download ) );
		send_interest(name, 0 ,-1);
	}
	// Set the number of in-flights packets
	inFlightPkts = currWinSize;
}


/*
 *		Generate Interest packets according to an IRM process. Inter-request times are exponentially distributed
 *		with mean = 1/lambda.
 *
 *		Parameters:
 *		- cNum = class number ('0' for IRM clients).
 */
void client_Window::request_file(unsigned long cNum)
{
	//name_t name = content_distribution::zipf.value(dblrand());
	name_t name = content_distribution::zipf[0]->sample();  // With Rejection-inversion sampling

	/*// ****  NB - COMMENT with Rejection-Inversion sampling
	// Check if the catalog aggregation is active. In this case, request for less popular contents
	// after a specified percentile are generated by randomly selecting an ID within that range.
	if (content_distribution::zipf.get_perc_aggr() < 1.0)
	{
		if (name == content_distribution::zipf.get_ID_aggr()+1) // In case of catalog aggregation, the vector
																// containing the Zipf cdf is truncated, so every
																// content after the specified threshold will provide
																// the same ID.
		{
			// Extract a random content from the part of the catalog that has been aggregated, that is [name, cat_card].
			name = std::floor((rand() / ((double)RAND_MAX+1))*(content_distribution::zipf.get_catalog_card() - name + 1) + name);
	    }
	}*/

	struct download new_download = download (0,simTime() );
	#ifdef SEVERE_DEBUG
	new_download.serial_number = interests_sent;

	if (!active){
		std::stringstream ermsg;
		ermsg<<"Client attached to node "<< getNodeIndex() <<" is requesting file but it "
			<<"shoud not as it is not active";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

	if (getNodeIndex() == 18){
		std::stringstream ermsg;
		ermsg<<"I am node "<< getNodeIndex() <<" and I am requesting a file";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif

	current_downloads.insert(pair<name_t, download >(name, new_download ) );
	send_interest(name, 0 ,-1);
}
