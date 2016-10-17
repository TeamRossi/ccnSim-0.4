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
#include "client_IRM.h"

#include "error_handling.h"
#include <random>

Register_Class (client_IRM);


void client_IRM::initialize()
{
	int num_clients = getAncestorPar("num_clients");
	active = false;
	if (find(content_distribution::clients, content_distribution::clients + num_clients ,getNodeIndex()) != content_distribution::clients + num_clients)
	{
		active = true;
		lambda = getAncestorPar("lambda");
		check_time	= getAncestorPar("check_time");

		timer = new cMessage("timer", TIMER);
		scheduleAt( simTime() + check_time, timer );

		//arrival = new cMessage("arrival", ARRIVAL );
		//scheduleAt( simTime() + uniform(0,1./lambda), arrival);
		//scheduleAt( simTime() + exponential(1./lambda), arrival);
		//scheduleAt( simTime() + 1./lambda, arrival);

		if(!onlyModel)		// The scheduling of clients requests should be done.
		{
			cModule* pSubModule = getParentModule()->getSubmodule("statistics");
			statistics* pStatisticsModule = dynamic_cast<statistics*>(pSubModule);

			normConstant = content_distribution::zipf->get_normalization_constant();  // It can be computed here.
			long M = (long)content_distribution::zipf->get_catalog_card();
			down = pStatisticsModule->par("downsize");
			newCard = round(M*(1./(double)down));    // Equal to the number of bin (or meta-contents)
			alphaVal = content_distribution::zipf->get_alpha();

			unsigned int converted_content;
			double current_lambda;


			if(down > 1)		// TTL-based scenario: schedule newCard content requests
			{
				// Extract a content from each bin (Zipf like) and schedule a request according to its original lambda
				for(unsigned int k=1; k <= newCard; k++)
				{
					unsigned int meta_content = content_distribution::zipf->sample();    // The meta-content is used only to retrieve the current lambda.
																						// The id of the requested content will be always equal to the bin number.
					converted_content = (unsigned int)(meta_content + (k-1)*down);
					current_lambda = (1.0/(double)pow(converted_content,alphaVal))*normConstant*lambda;

					cMessage *arrival_ttl = new cMessage("arrival_ttl", 4001 + k);
					scheduleAt( simTime() + uniform(0,(double)(1./(double)current_lambda)), arrival_ttl );

					//cout << "* BIN:\t" << k << "\t* META CONTENT:\t" << meta_content << "\t* CONVERTED CONTENT:\t" << converted_content << endl;
				}
			}
			else if(down == 1)		// ED-sim scenario: schedule one request with the overall lambda mean. The Zipf's like selection of the content will be executed later.
			{
				arrival = new cMessage("arrival", ARRIVAL );
				scheduleAt( simTime() + uniform(0,1./lambda), arrival);
			}
			else
			{
				std::stringstream ermsg;
				ermsg<<"Please insert a downsizing factor Delta >= 1.";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
		}
		client::initialize();
	}
}

void client_IRM::finish()
{
	client::finish();
}

void client_IRM::handleMessage(cMessage *in)
{
    if (in->isSelfMessage())	// A self-generated message can be either an 'ARRIVAL' or a 'TIMER'.
    {
		switch(in->getKind())
		{
		case ARRIVAL:
			request_file(newCard+1);   // Default class num is '0' for client IRM.
			scheduleAt( simTime() + exponential(1./lambda), arrival );
			//scheduleAt( simTime() + 1./lambda, arrival);
			//scheduleAt( simTime() + uniform(0,1./lambda), arrival);
			break;
		case TIMER:
			handle_timers(in);
			scheduleAt( simTime() + check_time, timer );
			break;
		default:
			// *** NB -- WITH RejInvSampling
			// Extract the content ID (which corresponds also the the bin number)
			unsigned int content = (unsigned int) (in->getKind() - 4001);

			// Extract another content from the same bin in order to change the lambda
			unsigned int meta_content = content_distribution::zipf->sample();      // The meta-content is used only to retrieve the current lambda.
																				  // The id of the requested content will be always equal to the bin number.
			unsigned int converted_content = (unsigned int)(meta_content + (content-1)*down);
			double current_lambda = (1.0/(double)pow(converted_content,alphaVal))*normConstant*lambda;

			if(content > 0 && content <= newCard)
			{
				request_file(content);
				scheduleAt( simTime() + exponential(1./current_lambda), in);
				break;
			}
			else
			{
				std::stringstream ermsg;
				ermsg<<"ERROR - Client IRM: received wrong self message identifier. Please check";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
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


/*
 *		Generate Interest packets according to an IRM process. Inter-request times are exponentially distributed
 *		with mean = 1/lambda.
 *
 *		Parameters:
 *		- cNum = class number ('0' for IRM clients).
 */
void client_IRM::request_file(unsigned int nameC)
{
	name_t name;

	if(nameC == newCard+1)  // ED-sim
		name = content_distribution::zipf->sample();	// Zipf with rejection-inversion sampling
	else					// TTL_based
		name = (name_t) nameC;

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
