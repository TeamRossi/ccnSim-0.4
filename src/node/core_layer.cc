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
#include "core_layer.h"
#include "ccnsim.h"
#include <algorithm>

#include "content_distribution.h"
#include "strategy_layer.h"
#include "ccn_interest.h"
#include "ccn_data.h"
#include "base_cache.h"
#include "statistics.h"

#include "two_lru_policy.h"
#include "two_ttl_policy.h"

#include "error_handling.h"

Register_Class(core_layer);
int core_layer::repo_interest = 0;

void  core_layer::initialize()
{
	//cout << "CORE LAYER INIT" << endl;

	RTT = par("RTT");

	interest_aggregation = par("interest_aggregation");
	transparent_to_hops = par("transparent_to_hops");

	repo_load = 0;
    nodes = getAncestorPar("n");
    my_btw = getAncestorPar("betweenness");
    int num_repos = getAncestorPar("num_repos");

	#ifdef SEVERE_DEBUG
		is_it_initialized = false;
		it_has_a_repo_attached = false;
	#endif

    int i = 0;
    my_bitmask = 0;
    for (i = 0; i < num_repos; i++)
	{
		if (content_distribution::repositories[i] == getIndex() )
		{
			#ifdef SEVERE_DEBUG
				it_has_a_repo_attached = true;
			#endif

			repo_price = content_distribution::repo_prices[i]; 
			break;
		} else
			repo_price = 0;
	}
    my_bitmask = (1<<i);	// Recall that the width of the repository bitset is only num_repos.

    // Initialize pointers to Content Store and Strategy Layer.
    ContentStore = (base_cache *) gate("cache_port$o")->getNextGate()->getOwner();
    strategy = (strategy_layer *) gate("strategy_port$o")->getNextGate()->getOwner();

	clear_stat();

	#ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);
	is_it_initialized = true;

	if (gateSize("face$o") > (int) sizeof(interface_t)*8 )
	{
		std::stringstream msg;
		msg<<"Node "<<getIndex()<<" has "<<gateSize("face$o")<<" output ports. But the maximum "
			<<"number of interfaces manageable by ccnsim is "<<sizeof(interface_t)*8 <<
			" beacause the type of "
			<<"interface_t is of size "<<sizeof(interface_t)<<" bytes. You can change the definition of "
			<<"interface_t (in ccnsim.h) to solve this issue and recompile";
		severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}
	#endif

	// *** Link Load Evaluation ***
	stable = false;
	catCard = (long)content_distribution::zipf[0]->get_catalog_card();
	llEval = par("llEval");
	if(llEval)
	{
		// *** DISABLED
		//load_check = new cMessage("load_check", LOAD_CHECK);

		
		//batchSize = 10;    // packets (disabled if we want synchronized measure for all links)
		maxInterval = par("maxInterval"); // seconds
		datarate = par("datarate");

		cModule* pSubModule = getParentModule()->getParentModule()->getSubmodule("statistics");
		statistics* pClass2Module = dynamic_cast<statistics*>(pSubModule);
		simDuration = pClass2Module->par("steady");

		//simDuration = getAncestorPar("steady");

		// *** DISABLED
		//scheduleAt(simTime() + maxInterval, load_check);

		int numOutInterf = gateSize("face$o") - 1;     // Interface # 0 is always connected to the client module.

		// With Percentiles
		/*percID = new unsigned int[numPercentiles+1];
		percID[0] = 0;
		for(int i=1; i <= numPercentiles; i++)
		{
			percID[i] = content_distribution::zipf.value(percentiles[i-1]);
			cout << " *** ID Percentile # " << i << " = " << percID[i] << endl;
		}*/

		// With percentiles
		//numPackets = new double*[numOutInterf];
		numBits = new double*[numOutInterf];
		//intvlNumPackets = new unsigned long*[numOutInterf];
		//intvlNumBits = new unsigned long*[numOutInterf];

		for(int i=0; i < numOutInterf; i++)
		{
			//numPackets[i] = new double[catCard];
			//fill_n(numPackets[i], catCard, 0);
			numBits[i] = new double[catCard];
			fill_n(numBits[i], catCard, 0);
			//intvlNumPackets[i] = new unsigned long[numPercentiles];
			//fill_n(intvlNumPackets[i], numPercentiles, 0);
			//intvlNumBits[i] = new unsigned long[numPercentiles];
			//fill_n(intvlNumBits[i], numPercentiles, 0);
		}

		// Without percentiles
		/*numPackets = new double[numOutInterf];
		numBits = new double[numOutInterf];
		intvlNumPackets = new unsigned long[numOutInterf];
		intvlNumBits = new unsigned long[numOutInterf];*/
	}
}



/*
 * 	Core layer handling message. The received packet is classified (Interest or Data)
 * 	and the respective handling functions are called.
 */
void core_layer::handleMessage(cMessage *in)
{
	#ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);
	char* last_received;
	#endif

    ccn_data *data_msg;
    ccn_interest *int_msg;

    int type = in->getKind();

    switch(type){
    case CCN_I:				// An Interest packet is received.
		interests++;

		int_msg = (ccn_interest *) in;

		if (!transparent_to_hops)
			int_msg->setHops(int_msg -> getHops() + 1);

		if (int_msg->getHops() == int_msg->getTTL())
		{
	    	#ifdef SEVERE_DEBUG
	    	discarded_interests++;
	    	check_if_correct(__LINE__);
	    	#endif
	    	break;
		}
		int_msg->setCapacity (int_msg->getCapacity() + ContentStore->get_size());

		handle_interest (int_msg);
		delete in;
		break;

    case CCN_D:			// A Data packet is received.
		data++;

		data_msg = (ccn_data* ) in;

		if (!transparent_to_hops)
			data_msg->setHops(data_msg -> getHops() + 1);

		handle_data(data_msg);
		delete in;
		break;

    case LOAD_CHECK:
    	evaluateLinkLoad();
    	scheduleAt(simTime() + maxInterval, in);
    	break;
    }

    //delete in;
    
	#ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);
	#endif
}

//	Print node statistics
void core_layer::finish()
{
	#ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);
//		if (data+repo_load != (int) (ContentStore->get_decision_yes() +	ContentStore->get_decision_no() )){
//			std::stringstream msg; 
//			msg<<"node["<<getIndex()<<"]: "<<
//				"decision_yes=="<<ContentStore->get_decision_yes()<<
//				"; decision_no=="<<ContentStore->get_decision_no()<<
//				"; repo_load=="<<repo_load<<
//				"; data="<<data<<
//				". The sum of decision_yes+decision_no MUST be equal to data+repo_load";
//			severe_error(__FILE__, __LINE__, msg.str().c_str() );
//		}
	#endif

    char name [30];

    sprintf ( name, "interests[%d]", getIndex());	// Total number of received Interest packets.
    recordScalar (name, interests);

    if (repo_load != 0)
    {
		sprintf ( name, "repo_load[%d]", getIndex());
		recordScalar(name,repo_load);
    }

    sprintf ( name, "data[%d]", getIndex());	//	Total number of received Data packets.
    recordScalar (name, data);

    if (repo_interest != 0)
    {
    	sprintf ( name, "repo_int[%d]", getIndex());	// Total number of Interest packets sent to the attached repository (if present).
    	recordScalar(name, repo_interest);
    	repo_interest = 0;
    }
}




/*
 * Interest handling. The order will be:
 *    a) Check inside Content Store.
 *    b) Check inside the attached Repo (if present).
 *    c) Check inside the PIT and, eventually, create an entry and forward the Interest.
 */
void core_layer::handle_interest(ccn_interest *int_msg)
{
	#ifdef SEVERE_DEBUG
		client* cli = __get_attached_client( int_msg->getArrivalGate()->getIndex() );
		if (cli && !cli->is_active() ) {
			std::stringstream msg; 
			msg<<"I am node "<< getIndex()<<" and I received an interest from interface "<<
				int_msg->getArrivalGate()->getIndex()<<". This is an error since there is "<<
				"a deactivated client attached there";
			debug_message(__FILE__, __LINE__, msg.str().c_str() );
		}
	#endif

	chunk_t chunk = int_msg->getChunk();
    double int_btw = int_msg->getBtw();

    bool cacheable = true;  // This value indicates whether the retrieved content will be cached.
    						// Usually it is always true, and it can be changed only with 2-LRU meta-caching.

    // Check if the meta-caching is 2-LRU. In this case, we need to lookup for the content ID inside the Name Cache.
    string decision_policy = ContentStore->par("DS");

    if (decision_policy.compare("two_lru")==0)		// 2-LRU
    {
    	Two_Lru* tLruPointer = dynamic_cast<Two_Lru *> (ContentStore->get_decisor());
    	if (!(tLruPointer->name_to_cache(chunk)))	// The ID is not present inside the Name Cache, so the
    												// cacheable flag inside the PIT will be set to '0'.
    			cacheable = false;
    }

    if (decision_policy.compare("two_ttl")==0)		// 2-TTL
    {
    	Two_TTL* tTTLPointer = dynamic_cast<Two_TTL *> (ContentStore->get_decisor());
    	if (!(tTTLPointer->name_to_cache(chunk)))	// The ID is not present inside the Name Cache, so the
    		// cacheable flag inside the PIT will be set to '0'.
    		cacheable = false;
    }

    //cout << "** Receiver INTEREST for content: " << int_msg->get_name() << " **" << endl;

    if (ContentStore->lookup(chunk))	// a) Lookup inside the local Content Store.
    {
    	// *** Logging HIT EVENT with timestamp
    	//

    	//cout << SIMTIME_DBL(simTime()) << "\tNODE\t" << getIndex() << "\t_HIT_\t" << __id(chunk) << endl;

    	// The received Interest is satisfied locally.
        ccn_data* data_msg = compose_data(chunk);

        data_msg->setHops(0);
        data_msg->setBtw(int_btw);
        data_msg->setTarget(getIndex());
        data_msg->setFound(true);

        data_msg->setCapacity(int_msg->getCapacity());
        data_msg->setTSI(int_msg->getHops());
        data_msg->setTSB(1);

        send_data(data_msg,"face$o", int_msg->getArrivalGate()->getIndex(), __LINE__);

        
        // *** Link Load Evaluation ***
		if(llEval && stable)
		{
			if(!(ContentStore->__check_client(int_msg->getArrivalGate()->getIndex())))
			{
				int outIndex = int_msg->getArrivalGate()->getIndex() - 1; // Because we downsized the vectors for the load evaluation
																		  // by excluding the face towards the client.
				// With percentiles
				//for(int i=1; i <= numPercentiles+1; i++)
				//{
					// Complete (put attention on the first interval)
				//	if(__id(chunk) > percID[i-1] && __id(chunk)<= percID[i])
				//	{
						// cout << "NODE # " << getIndex() << " - TX - Content # " << __id(chunk) << " from Interface N " << outIndex << " between percentiles " << percID[i-1] << " and " << percID[i] << endl;
						// Only sent DATA packets are considered (supposed having a size of 1536 Bytes)
						// With percentiles
						//numPackets[outIndex][i-1]++;
						//numBits += ((cPacket*)msg)->getBitLength();
						//numBits[outIndex][i-1] += 1536*8;
						// Without percentiles
						//numPackets[outIndex]++;
						numBits[outIndex][__id(chunk)-1] += 1536*8;

						// LOG Link Load in time
						if(getIndex() == 0)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 1)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T1 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}
						else if(getIndex() == 1)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 3)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T2 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}
						else if(getIndex() == 3)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 7)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T3 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}
						// With percentiles
						//intvlNumPackets[outIndex][i-1]++;
						//intvlNumBits += ((cPacket*)msg)->getBitLength();
						//intvlNumBits[outIndex][i-1] += 1536*8;
						//break;

						// Without percentiles
						//intvlNumPackets[outIndex]++;
						//intvlNumBits[outIndex] += 1536*8;
				//	}
				//}
			}
		}

        #ifdef SEVERE_DEBUG
        interests_satisfied_by_cache++;
		check_if_correct(__LINE__);
        #endif
    }
    else if ( my_bitmask & __repo(int_msg->get_name()))  // b) Lookup inside the attached repo if I am supposed
    													 //	   to be the source for the requested content.
    {
    	// *** Logging MISS EVENT with timestamp
    	//cout << SIMTIME_DBL(simTime()) << "\tNODE\t" << getIndex() << "\t_MISS_\t" << __id(chunk) << endl;

    	// We are mimicking an Interest sent to the repository.
        ccn_data* data_msg = compose_data(chunk);
	
		data_msg->setPrice(repo_price); 	// I fix in the data msg the cost of the object
											// that is the price of the repository
		repo_interest++;
		repo_load++;

        data_msg->setHops(1);
        data_msg->setTarget(getIndex());
		data_msg->setBtw(std::max(my_btw,int_btw));

		data_msg->setCapacity(int_msg->getCapacity());
		data_msg->setTSI(int_msg->getHops() + 1);
		data_msg->setTSB(1);
		data_msg->setFound(true);

		// Since the PIT entry has not been created yet, the cacheability control
		// is done locally, i.e., the generated content is cached only if cacheable = true.
		// So far, this control makes sense only if a 2-LRU meta-caching is simulated.
        if (cacheable)
        	ContentStore->store(data_msg);

		send_data(data_msg,"face$o",int_msg->getArrivalGate()->getIndex(),__LINE__);

        // *** Link Load Evaluation ***
		if(llEval && stable)
		{
			if(!(ContentStore->__check_client(int_msg->getArrivalGate()->getIndex())))
			{
				int outIndex = int_msg->getArrivalGate()->getIndex() - 1; // Because we downsized the vectors for the load evaluation
																		  // by excluding the face towards the client.
				// With percentiles
				//for(int i=1; i <= numPercentiles+1; i++)
				//{
					// Complete (put attention on the first interval)
					//if(__id(chunk) > percID[i-1] && __id(chunk)<= percID[i])
					//{
						//cout << "NODE # " << getIndex() << " - TX - Content # " << __id(chunk) << " from Interface N " << outIndex << " between percentiles " << percID[i-1] << " and " << percID[i] << endl;
						// Only sent DATA packets are considered (supposed having a size of 1536 Bytes)
						// With percentiles
//						numPackets[outIndex][i-1]++;
//						//numBits += ((cPacket*)msg)->getBitLength();
//						numBits[outIndex][i-1] += 1536*8;
//
//						intvlNumPackets[outIndex][i-1]++;
//						//intvlNumBits += ((cPacket*)msg)->getBitLength();
//						intvlNumBits[outIndex][i-1] += 1536*8;
//						break;

						// Without percentiles
						//numPackets[outIndex]++;
						//numBits += ((cPacket*)msg)->getBitLength();
						numBits[outIndex][__id(chunk)-1] += 1536*8;

						// LOG Link Load in time
						if(getIndex() == 0)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 1)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T1 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}
						else if(getIndex() == 1)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 3)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T2 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}
						else if(getIndex() == 3)
						{
							double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 7)
							{
								cout << SIMTIME_DBL(simTime()) << "\t LL-T3 - 1" << "\tContent # " << __id(chunk)-1 << endl;
							}
						}

						//intvlNumPackets[outIndex]++;
						//intvlNumBits[outIndex] += 1536*8;
					//}
				//}
			}
		}


		#ifdef SEVERE_DEBUG
		check_if_correct(__LINE__);
		#endif
	}
    else 	// c) PIT lookup.
    {
		#ifdef SEVERE_DEBUG
		unsatisfied_interests++;
		check_if_correct(__LINE__);
		#endif

		// *** Logging MISS EVENT with timestamp
		//cout << SIMTIME_DBL(simTime()) << "\tNODE\t" << getIndex() << "\t_MISS_\t" << __id(chunk) << endl;

		unordered_map < chunk_t , pit_entry >::iterator pitIt = PIT.find(chunk);

		bool i_will_forward_interest = false;

		//<aa> Insert a new PIT entry for this object, if not present. If present and invalid, reset the
		// old entry. If present and valid, do nothing </aa>
        if (	
			// There is no PIT entry for the received Interest, which, as a consequence, should be forwarded.
			pitIt==PIT.end()

			// There is a PIT entry but it is invalid (the PIT entry has been invalidated by client through a retransmission
			// because a timer expired and the object has not been found)
			|| (pitIt != PIT.end() && int_msg->getNfound() ) 

			// Too much time has been passed since the PIT entry was added
			|| simTime() - PIT[chunk].time > 2*RTT
        )
        {
			i_will_forward_interest = true;
			if (pitIt!=PIT.end())				// Invalidate and re-create a new PIT entry.
				PIT.erase(chunk);

			PIT[chunk].time = simTime();

	    	if(!cacheable)						// Set the cacheable flag inside the PIT entry.
	    		PIT[chunk].cacheable.reset();
	    	else
	    		PIT[chunk].cacheable.set();
		}

		if (int_msg->getTarget() == getIndex() )
		{	// I am the target of this interest but I have no more the object
			// Therefore, this interest cannot be aggregated with the others
			int_msg->setAggregate(false);
		}

		if ( !interest_aggregation || int_msg->getAggregate()==false )
			i_will_forward_interest = true;

		if (i_will_forward_interest)
		{  	bool * decision = strategy->get_decision(int_msg);
	    	handle_decision(decision,int_msg);
	    	delete [] decision;//free memory for the decision array
		}

		#ifdef SEVERE_DEBUG
		interface_t old_PIT_string = PIT[chunk].interfaces;
		check_if_correct(__LINE__);

		client*  c = __get_attached_client( int_msg->getArrivalGate()->getIndex() );
		if (c && !c->is_active() ){
			std::stringstream ermsg; 
			ermsg<<"Trying to add to the PIT an interface where a deactivated client is attached";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		#endif

		// Add the incoming interface to the PIT entry.
		add_to_pit( chunk, int_msg->getArrivalGate()->getIndex() );

		#ifdef SEVERE_DEBUG
		check_if_correct(__LINE__);
		#endif
    }
    
    #ifdef SEVERE_DEBUG
    check_if_correct(__LINE__);
    #endif
}


/*
 * Data handling.
 * First of all, the presence of a PIT entry is checked. If so, the Data is stored inside the local cache
 * and it is forwarded towards the incoming interfaces associated to the PIT entry.
 * Otherwise, the Data packet is discarded.
 */
void core_layer::handle_data(ccn_data *data_msg)
{
    int i = 0;
    interface_t interfaces = 0;
    chunk_t chunk = data_msg -> getChunk();

    unordered_map < chunk_t , pit_entry >::iterator pitIt = PIT.find(chunk);

	#ifdef SEVERE_DEBUG
		int copies_sent = 0;
	#endif

    if ( pitIt != PIT.end() )		// A PIT entry is found.
	{

    	if (pitIt->second.cacheable.test(0))  // Cache the content only if the cacheable bit is set.
    		ContentStore->store(data_msg);
		else
			ContentStore->after_discarding_data();

    	interfaces = (pitIt->second).interfaces;	// Get incoming interfaces.
		i = 0;
		while (interfaces)
		{
			if ( interfaces & 1 )
			{
				send_data(data_msg->dup(), "face$o", i,__LINE__ );

		        // *** Link Load Evaluation ***
				if(llEval && stable)
				{
					if(!(ContentStore->__check_client(i)))
					{
						int outIndex = i - 1; // Because we downsized the vectors for the load evaluation
																				  // by excluding the face towards the client.
					//	for(int i=1; i <= numPercentiles+1; i++)
					//	{
							// Complete (put attention on the first interval)
					//		if(chunk > percID[i-1] && chunk <= percID[i])
					//		{
								//cout << "NODE # " << getIndex() << " - TX - Content # " << __id(chunk) << " from Interface N " << outIndex << " between percentiles " << percID[i-1] << " and " << percID[i] << endl;
								// Only sent DATA packets are considered (supposed having a size of 1536 Bytes)
//								numPackets[outIndex][i-1]++;
//								//numBits += ((cPacket*)msg)->getBitLength();
//								numBits[outIndex][i-1] += 1536*8;
//
//								intvlNumPackets[outIndex][i-1]++;
//								//intvlNumBits += ((cPacket*)msg)->getBitLength();
//								intvlNumBits[outIndex][i-1] += 1536*8;
//								break;

								// Without percentiles
								//numPackets[outIndex]++;
								//numBits += ((cPacket*)msg)->getBitLength();
								numBits[outIndex][__id(chunk)-1] += 1536*8;

								// LOG Link Load in time
								if(getIndex() == 0)
								{
									double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
									if(nextNode == 1)
									{
										cout << SIMTIME_DBL(simTime()) << "\t LL-T1 - 1" << "\tContent # " << __id(chunk)-1 << endl;
									}
								}
								else if(getIndex() == 1)
								{
									double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
									if(nextNode == 3)
									{
										cout << SIMTIME_DBL(simTime()) << "\t LL-T2 - 1" << "\tContent # " << __id(chunk)-1 << endl;
									}
								}
								else if(getIndex() == 3)
								{
									double nextNode = getParentModule()->gate("face$o",outIndex+1)->getNextGate()->getOwnerModule()->getIndex();
									if(nextNode == 7)
									{
										cout << SIMTIME_DBL(simTime()) << "\t LL-T3 - 1" << "\tContent # " << __id(chunk)-1 << endl;
									}
								}

								//intvlNumPackets[outIndex]++;
								//intvlNumBits[outIndex] += 1536*8;

					//		}
					//	}
					}
				}


				#ifdef SEVERE_DEBUG
					copies_sent++;
				#endif
			}
			i++;
			interfaces >>= 1;
		}
    }

    // Otherwise the Data is unrequested
	#ifdef SEVERE_DEBUG
		else unsolicited_data++;
	#endif

    PIT.erase(chunk); //erase pending PIT entry.

    #ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);
	#endif
}


void core_layer::handle_decision(bool* decision,ccn_interest *interest){

	#ifdef SEVERE_DEBUG
	bool interest_has_been_forwarded = false;
	#endif

    if (my_btw > interest->getBtw())
		interest->setBtw(my_btw);

    for (int i = 0; i < __get_outer_interfaces(); i++)
	{
		#ifdef SEVERE_DEBUG
			if (decision[i] == true && __check_client(i) )
			{
				std::stringstream msg; 
				msg<<"I am node "<< getIndex()<<" and the interface supposed to give"<<
					" access to chunk "<< interest->getChunk() <<" is "<<i
					<<". This is impossible "<<
					" since that interface is to reach a client and you cannot access"
					<< " a content from a client ";
				severe_error(__FILE__, __LINE__, msg.str().c_str() );
			}
		#endif

		if (decision[i] == true && !__check_client(i))
		{
			sendDelayed(interest->dup(),interest->getDelay(),"face$o",i);
			#ifdef SEVERE_DEBUG
			interest_has_been_forwarded = true;
			#endif
		}
	}

	#ifdef SEVERE_DEBUG
		if (! interest_has_been_forwarded)
		{
			int affirmative_decision_from_arrival_gate = 0;
			int affirmative_decision_from_client = 0;
			int last_affermative_decision = -1;

			for (int i = 0; i < __get_outer_interfaces(); i++)
			{
				if (decision[i] == true)
				{
					if ( __check_client(i) ){
						affirmative_decision_from_client++;
						last_affermative_decision = i;
					}
					if ( interest->getArrivalGate()->getIndex() == i ){
						affirmative_decision_from_arrival_gate++;
						last_affermative_decision = i;
					}
				}
			}
			std::stringstream msg; 
			msg<<"I am node "<< getIndex()<<" and interest for chunk "<<
				interest->getChunk()<<" has not been forwarded. "<<
				". One of the possible repositories of this chunk is "<< 
				interest->get_repos()[0] <<" and the target of the interest is "<<
				interest->getTarget() <<
				". affirmative_decision_for_client = "<<
				affirmative_decision_from_client<<
				". affirmative_decision_for_arrival_gate = "<<
				affirmative_decision_from_arrival_gate<<
				". I would have sent the interest to interface "<<
				last_affermative_decision;
			severe_error(__FILE__, __LINE__, msg.str().c_str() );
		}
	#endif
}

// Check if the local node is the owner of the requested content.
bool core_layer::check_ownership(vector<int> repositories){
    bool check = false;
    if (find (repositories.begin(),repositories.end(),getIndex()) != repositories.end())
	check = true;
    return check;
}



/*
 * 	Create a Data packet in response to the received Interest.
 */
ccn_data* core_layer::compose_data(uint64_t response_data){
    ccn_data* data = new ccn_data("data",CCN_D);
    data -> setChunk (response_data);
    data -> setHops(0);
    data->setTimestamp(simTime());
    return data;
}


/*
 * Clear local statistics
 */
void core_layer::clear_stat(){
    repo_interest = 0;
    interests = 0;
    data = 0;
    
    repo_load = 0;
	ContentStore->set_decision_yes(0);
	ContentStore->set_decision_no(0);

    
   	#ifdef SEVERE_DEBUG
	unsolicited_data = 0;
	discarded_interests = 0;
	unsatisfied_interests = 0;
	interests_satisfied_by_cache = 0;
	check_if_correct(__LINE__);
	#endif
}

#ifdef SEVERE_DEBUG
void core_layer::check_if_correct(int line)
{
	if (repo_load != interests - discarded_interests - unsatisfied_interests
		-interests_satisfied_by_cache)
	{
			std::stringstream msg; 
			msg<<"node["<<getIndex()<<"]: "<<
				"repo_load="<<repo_load<<"; interests="<<interests<<
				"; discarded_interests="<<discarded_interests<<
				"; unsatisfied_interests="<<unsatisfied_interests<<
				"; interests_satisfied_by_cache="<<interests_satisfied_by_cache;
		    severe_error(__FILE__, line, msg.str().c_str() );
	}

	if (!it_has_a_repo_attached && repo_load>0 )
	{
			std::stringstream msg; 
			msg<<"node["<<getIndex()<<"] has no repo attached. "<<
				"repo_load=="<<repo_load<<
				"; repo_interest=="<<repo_interest;
			severe_error(__FILE__, line, msg.str().c_str() );
	}

	if (	ContentStore->get_decision_yes() + ContentStore->get_decision_no() +  
						(unsigned) unsolicited_data
						!=  (unsigned) data + repo_load
	){
					std::stringstream ermsg; 
					ermsg<<"caches["<<getIndex()<<"]->decision_yes="<<ContentStore->get_decision_yes()<<
						"; caches[i]->decision_no="<< ContentStore->get_decision_no()<<
						"; cores[i]->data="<< data<<
						"; cores[i]->repo_load="<< repo_load<<
						"; cores[i]->unsolicited_data="<< unsolicited_data<<
						". The sum of "<< "decision_yes + decision_no + unsolicited_data must be data";
					severe_error(__FILE__,line,ermsg.str().c_str() );
	}
} //end of check_if_correct(..)
#endif

double core_layer::get_repo_price()
{
	#ifdef SEVERE_DEBUG
	if (!is_it_initialized)
	{
			std::stringstream msg; 
			msg<<"I am node "<< getIndex()<<". Someone called this method before I was"
				" initialized";
			severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}
	#endif

	return repo_price;
}

/*
 * 	Add an interface to a PIT entry.
 */
void core_layer::add_to_pit(chunk_t chunk, int gateindex)
{
	#ifdef SEVERE_DEBUG
	check_if_correct(__LINE__);

	if (gateindex > gateSize("face$o")-1 )
	{
		std::stringstream msg;
		msg<<"You are inserting a pit entry related to interface "<<gateindex<<
			". But the number of ports is "<<gateSize("face$o");
		severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}

	if (gateindex > (int) sizeof(interface_t)*8-1 )
	{
		std::stringstream msg;
		msg<<"You are inserting a pit entry related to interface "<<gateindex<<
			". But the maximum interface "
			<<"number manageable by ccnsim is "<<sizeof(interface_t)*8-1 <<" beacause the type of "
			<<"interface_t is of size "<<sizeof(interface_t)<<". You can change the definition of "
			<<"interface_t (in ccnsim.h) to solve this issue and recompile";
		severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}
	#endif	

	__sface( PIT[chunk].interfaces , gateindex );

	#ifdef SEVERE_DEBUG

	unsigned long long bit_op_result = (interface_t)1 << gateindex;
	if ( bit_op_result > pow(2,gateSize("face$o")-1) )
	{
				printf("ATTTENZIONE bit_op_result %llX\n", bit_op_result);
				std::stringstream ermsg; 
				ermsg<<"I am node "<<getIndex()<<", bit_op_result="<<bit_op_result <<
					" while the number of ports is "<<
					gateSize("face$o")<<" and the max number that I should observe is "<<
					pow(2,gateSize("face$o") )-1;
				ermsg<<". (1<<34)="<< (1<<34);
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

	check_if_correct(__LINE__);
	#endif
}

/*
 * 	Forward a Data packet.
 */
int	core_layer::send_data(ccn_data* msg, const char *gatename, int gateindex, int line_of_the_call)
{
	if (gateindex > gateSize("face$o")-1 )
	{
		std::stringstream msg;
		msg<<"I am node "<<getIndex() <<". Line "<<line_of_the_call<<
			" commands you to send a packet to interface "<<gateindex<<
			". But the number of ports is "<<gateSize("face$o");
		severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}

	#ifdef SEVERE_DEBUG
	if ( gateindex > (int) sizeof(interface_t)*8-1 )
	{
		std::stringstream msg;
		msg<<"You are trying to send a packet through the interface gateindex. But the maximum interface "
			<<"number manageable by ccnsim is "<<sizeof(interface_t)*8-1 <<" beacause the type of "
			<<"interface_t is of size "<<sizeof(interface_t)<<". You can change the definition of "
			<<"interface_t (in ccnsim.h) to solve this issue and recompile";
		severe_error(__FILE__, __LINE__, msg.str().c_str() );
	}

	client* c = __get_attached_client(gateindex);
	if (c)
	{	//There is a client attached to that port
		if ( !c->is_waiting_for( msg->get_name() ) )
		{
			std::stringstream msg; 
			msg<<"I am node "<< getIndex()<<". I am sending a data to the attached client that is not "<<
				" waiting for it. This is not necessarily an error, as this data could have been "
				<<" requested by the client and the client could have retrieved it before and now"
				<<" it may be fine and not wanting the data anymore. If it is the case, "<<
				"ignore this message ";
			debug_message(__FILE__, __LINE__, msg.str().c_str() );
		}

		if ( !c->is_active() )
		{
			std::stringstream msg; 
			msg<<"I am node "<< getIndex()<<". I am sending a data to the attached client "<<
				", that is not active, "<<
				" through port "<<gateindex<<". This was commanded in line "<< line_of_the_call;
			severe_error(__FILE__, __LINE__, msg.str().c_str() );
		}
	}
	#endif
	return send (msg, gatename, gateindex);
}

int core_layer::getOutInt(int dest)
{
	return strategy->get_out_interface(dest);
}

void core_layer::evaluateLinkLoad()
{
/*	double currentBitPerSec;
	double currentChLoad;
	//double cumSumChLoad = 0.0;
	int nextNode;
	if (SIMTIME_DBL(simTime()) + maxInterval >= simDuration)  // Print average statistics
	{
		for(int i=0; i < gateSize("face$o") - 1; i++)
		{
//			for(int j=0; j < numPercentiles; j++)
//			{
//				//avgBitPerSec = numBits[i]/SIMTIME_DBL(simTime());
//				//currentBitPerSec = numBits[i]/simDuration;
//				currentBitPerSec = numBits[i][j]/SIMTIME_DBL(simTime());
//				currentChLoad = currentBitPerSec/datarate;       // So far the datarate is fixed and equal for each link.
//				cumSumChLoad += currentChLoad;
//				nextNode = getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
//				cout << SIMTIME_DBL(simTime()) << "\tAvg LL Nodes\t" << getIndex() << " OUT \t-->\t" << nextNode << " IN - Percentile # " << percentiles[j]*100 <<  "th:\t" << cumSumChLoad << endl;
//			}
//			cumSumChLoad = 0.0;

			// Without percentiles
				//avgBitPerSec = numBits[i]/SIMTIME_DBL(simTime());
				//currentBitPerSec = numBits[i]/simDuration;
				currentBitPerSec = numBits[i]/SIMTIME_DBL(simTime());
				currentChLoad = currentBitPerSec/datarate;       // So far the datarate is fixed and equal for each link.
				nextNode = getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
				if(numBits[i] != 0 )
					cout << SIMTIME_DBL(simTime()) << "\tAvg LL Nodes\t" << getIndex() << " OUT \t-->\t" << nextNode << " = " << currentChLoad << endl;
				else
					cout << SIMTIME_DBL(simTime()) << "\tAvg LL Nodes\t" << getIndex() << " OUT \t-->\t" << nextNode << " = 0" << endl;
		}
	}
	else
	{
		for(int i=0; i < gateSize("face$o") - 1; i++)
		{
			// With percentiles
//			for(int j=0; j < numPercentiles; j++)
//			{
//				currentBitPerSec = intvlNumBits[i][j]/maxInterval;
//				currentChLoad = currentBitPerSec/datarate;       // So far the datarate is fixed and equal for each link.
//				cumSumChLoad += currentChLoad;
//				nextNode = getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
//				cout << SIMTIME_DBL(simTime()) << "\tLink Load Nodes\t" << getIndex() << " OUT \t-->\t" << nextNode << " IN - Percentile # " << percentiles[j]*100 <<  "th:\t" << cumSumChLoad << endl;
//				intvlNumBits[i][j] = 0;
//				intvlNumPackets[i][j] = 0;
//			}
//			cumSumChLoad = 0.0;

			// Without percentiles
				currentBitPerSec = intvlNumBits[i]/maxInterval;
				currentChLoad = currentBitPerSec/datarate;       // So far the datarate is fixed and equal for each link.
				nextNode = getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
				cout << SIMTIME_DBL(simTime()) << "\tLink Load Nodes\t" << getIndex() << " OUT \t-->\t" << nextNode << " = " << currentChLoad << endl;
				intvlNumBits[i] = 0;
				intvlNumPackets[i] = 0;

		}
	}*/
}


strategy_layer* core_layer::get_strategy() const
{
	return strategy;
}
