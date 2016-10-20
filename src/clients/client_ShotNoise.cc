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
#include "client_ShotNoise.h"

#include "error_handling.h"
#include <random>

Register_Class (client_ShotNoise);

ShotNoiseContentDistribution* client_ShotNoise::snPointer;

void client_ShotNoise::initialize(int stage)
{
	if(stage == 1)		// Multi-stage initialization (we need the ShotNoiseContentDistribution pointer to be initialized)
	{
		int num_clients = getAncestorPar("num_clients");
		active = false;
		if (find(content_distribution::clients, content_distribution::clients + num_clients ,getNodeIndex()) != content_distribution::clients + num_clients)
		{
			// Initialize the pointer to ShotNoiseContentDistribution in order to take useful info,
			// like number of classes with the respective request rates.
			cModule* pSubModule = getParentModule()->getSubmodule("content_distribution");
			if (pSubModule)
			{
				snPointer = dynamic_cast<ShotNoiseContentDistribution*>(pSubModule);
				if (snPointer)
				{
					cout << "Number of Classes:\t" << snPointer->numOfClasses << "\n";

					onOffClass.resize(snPointer->numOfClasses, true);

					scheduledReq.resize(snPointer->numOfClasses, 0);
					validatedReq.resize(snPointer->numOfClasses, 0);

					// Take info and initialize the respective structures.
					for (int i=0; i<snPointer->numOfClasses; i++)
					{
						// The identifier of each msg will correspond to the class number.
						cMessage* tempArrival = new cMessage("tempArrival", (short)(i+1));
						arrivals.push_back(tempArrival);
						scheduleAt( simTime() + exponential(1./snPointer->classInfo->operator [](i).lambdaClient), arrivals[i]);

						// Check if the class will be modeled as ON-OFF.
						if (snPointer->classInfo->operator [](i).ton > snPointer->steadySimTime)
							onOffClass[i] = false;	// The ON period of the class is longer the the whole simulation.
					}

					active = true;
					lambda = getAncestorPar("lambda");		// Unused (the lambda is calculated for each class according to the configuration file).
					check_time	= getAncestorPar("check_time");
					timer = new cMessage("timer", TIMER);
					scheduleAt( simTime() + check_time, timer );
				}
				else
				{
					// ERROR MSG: client_ShotNoise is strictly associated to the ShotNoiseContentDistribution.
					std::stringstream ermsg;
					ermsg<<"A 'Shot Noise' client type requires to instantiate a 'Shot Noise' content distribution"<<
							" type. Please check.";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			}
			client::initialize();
		}
    }
}

int client_ShotNoise::numInitStages() const
{
	return 2;
}

void client_ShotNoise::finish()
{
	client::finish();
}


void client_ShotNoise::handleMessage(cMessage *in)
{
    if (in->isSelfMessage())	// A self-generated message can be either an 'ARRIVAL' or a 'TIMER'.
    {
    	short msgID = in->getKind();

    	if(msgID == TIMER)
    	{
			handle_timers(in);
			scheduleAt( simTime() + check_time, timer );
			return;
    	}
    	else       // It can be a request for class 'msgID'.
    	{
    		bool check = false;
    		for(short i=0; i<snPointer->numOfClasses; i++)
    		{
    			if( (i+1) == msgID)
    			{
    				request_file(i+1);  	// Request contents from the 'msgID'-th popularity class.

    				// Re-schedule a request according to the lambda of the 'msgID' class.
    				scheduleAt( simTime() + exponential(1./snPointer->classInfo->operator [](i).lambdaClient), arrivals[i]);
    				check = true;
    				scheduledReq[i]++;
    			}
    		}

    		if(!check)  	// msgID does not correspond to any of the class identifiers.
    		{
    			std::stringstream ermsg;
    			ermsg<<"ERROR - The message identifier does not correspond to any of the class identifier"<<
    					" type. Please check.";
    			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
    		}
    		return;
    	}
    }

    switch (in->getKind())	// In case of an external message, it can only be a DATA packet.
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
 *		Generate Interest packets according to the Shot Noise Model (SNM). Interest packets are generated
 *		and forwarded iif the extracted content is within an ON period. Inter-request times within ON periods
 *		are exponentially distributed with mean_class_i = 1/lambda_class_i.
 *
 *		Parameters:
 *		- cNum = popularity class from which a request should be generated.
 */
void client_ShotNoise::request_file(int cNum)
{
	// Extract a 'local' content ID from the range associated to class 'cNum'.
	//name_t nameLocal = snPointer->zipfClasses.operator [](cNum-1)->value(dblrand());
	name_t nameLocal = snPointer->zipfClasses.operator [](cNum-1)->sample();

	// Obtaining the 'global' content ID by adding the lower bound ID of class 'cNum'.
	name_t nameGlobal = snPointer->classInfo->operator [](cNum-1).mostPopular - 1  + nameLocal;

    // Request Validation (i.e., the request will be generated only if the extracted content is ON).
	if (onOffClass[cNum-1])	// A request validation is needed only for those classes with Ton < stedySimTime.
	{
		if(validateRequest(cNum, nameGlobal))
		{
			validatedReq[cNum-1]++;
			struct download new_download = download (0,simTime() );

			#ifdef SEVERE_DEBUG
			new_download.serial_number = interests_sent;
			if (!active){
				std::stringstream ermsg;
				ermsg<<"Client attached to node "<< getNodeIndex() <<" is requesting file but it "
						<<"shoud not as it is not active";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
			#endif

			current_downloads.insert(pair<name_t, download >(nameGlobal, new_download ) );
			send_interest(nameGlobal, 0 ,-1);
		}
		else
		{
			// Do nothing. The request is not generated.
		}
	}
	else  // Always generate a request for classes that are not ON-OFF modeled.
	{
		validatedReq[cNum-1]++;

		struct download new_download = download (0,simTime() );

		#ifdef SEVERE_DEBUG
		new_download.serial_number = interests_sent;
		if (!active){
			std::stringstream ermsg;
			ermsg<<"Client attached to node "<< getNodeIndex() <<" is requesting file but it "
					<<"shoud not as it is not active";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		#endif

		current_downloads.insert(pair<name_t, download >(nameGlobal, new_download ) );
		send_interest(nameGlobal, 0 ,-1);
	}
}

/*
 * 		Validate the request for the extracted content. Both the 'State' (i.e., ON/OFF) and the
 * 		'Transition Time' of the specified content are checked end eventually updated.
 *
 * 		Parameters:
 * 		- cNum: class number;
 * 		- ID: global content ID.
 */
bool client_ShotNoise::validateRequest(int cNum, name_t ID)
{
	bool sendOk = true;
	simtime_t contentTime = snPointer->get_state_time(ID-1);

	if (snPointer->check_state_flag(ID-1))  	// The content was ON.
	{
		if (simTime() <= contentTime)  			// The content is still ON.
		{
			// Do nothing. The content was ON and it is still ON; as a consequence, 'sendOK' remains true.
		}
		else	// Sim_time > ON_time associated to the content, which, as a consequence,
			    // can be either ON or OFF at the current moment. Therefore, we need to update
				// the ON/OFF process (state and transition time) associated to the content.
		{
			while (contentTime < simTime())
			{
				// We add first the OFF time because the content was in a ON state.
				contentTime += snPointer->classInfo->operator [](cNum-1).toff;
				snPointer->unset_state_flag(ID-1);
				sendOk = false;
				if (contentTime < simTime())	// We need to extract another ON time.
				{
					contentTime += (simtime_t)exponential(snPointer->classInfo->operator [](cNum-1).ton);
					snPointer->set_state_flag(ID-1);
					sendOk = true;
				}
			}
			snPointer->set_state_time(ID-1, contentTime);		// Update the transition time of the content.
		}
	}
	else		// The content was OFF.
	{
		if (simTime() <= contentTime)	// The content is still OFF.
		{
			sendOk = false;		// Do not generate the request.
		}
		else	// Sim_time > OFF_time associated to the content, which, as a consequence,
		    	// can be either ON or OFF at the current moment. Therefore, we need to update
				// the ON/OFF process (state and transition time) associated to the content.
		{
			while (contentTime < simTime())
			{
				// We add first the ON time because the content was in a OFF state.
				contentTime += (simtime_t)exponential(snPointer->classInfo->operator [](cNum-1).ton);
				snPointer->set_state_flag(ID-1);
				sendOk = true;
				if(contentTime < simTime())		// We need to extract another OFF time.
				{
					contentTime += snPointer->classInfo->operator [](cNum-1).toff;
					snPointer->unset_state_flag(ID-1);
					sendOk = false;
				}
			}
			snPointer->set_state_time(ID-1, contentTime);			// Update the transition time of the content.
		}
	}
	return sendOk;
}
