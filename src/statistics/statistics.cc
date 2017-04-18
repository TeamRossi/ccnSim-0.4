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
#include <cmath>
#include "statistics.h"
#include "core_layer.h"
#include "base_cache.h"
#include "strategy_layer.h"
#include "content_distribution.h"
#include "ShotNoiseContentDistribution.h"
#include "lru_cache.h"
#include <unistd.h>
#include "ccn_data.h"
#include "always_policy.h"
#include "fix_policy.h"
#include "two_lru_policy.h"
#include "two_ttl_policy.h"
//#include "nrr.h"
#include "client_IRM.h"
#include "ttl_cache.h"
#include "ttl_name_cache.h"

//<aa>
#include "error_handling.h"
//</aa>


Register_Class(statistics);


using namespace std;
#if OMNETPP_VERSION >= 0x0500
    using namespace omnetpp;
#endif

extern "C" double compute_Tc_single_Approx(double cSizeTarg, double alphaVal, long catCard, float** reqRates, int colIndex, const char* decString, double fixProb);
extern "C" double compute_Tc_single_Approx_More_Repo(double cSizeTarg, double alphaVal, long catCard, float** reqRates, int colIndex, const char* decString, double fixProb);

void statistics::initialize(int stage)
{
	if(stage == 1)
	{
		scheduleEnd = false;
		tStartGeneral = chrono::high_resolution_clock::now();
		cout << "Initialize STATISTICS...\tTime:\t" << SimTime() << "\n";

		// Network information
		num_nodes 	= getAncestorPar("n");
		num_clients = getAncestorPar("num_clients");
		num_repos = getAncestorPar("num_repos");
   
		// Statistics parameters
		ts = par("ts"); 		// Sampling time
		window = par("window");

		// Only model solver of entire simulation
		onlyModel = par("onlyModel");

		// If the Shot Noise Model is simulated, the steady state time is evaluated
		// according to the parameters extracted from the configuration file, and to the total
		// number of requests that the user wants to simulate. The initialization stage of Statistics module
		// is after the one of ShotNoiseContentDistribution class.
		cModule* pSubModule = getParentModule()->getSubmodule("content_distribution");
		if (pSubModule)
		{
			ShotNoiseContentDistribution* pClass2Module = dynamic_cast<ShotNoiseContentDistribution*>(pSubModule);
			if (pClass2Module)		// The Shot Noise Model is simulated.
			{
				time_steady = pClass2Module->steadySimTime;
			}
			else					// The IRM model is simulated. Read the steady time from the .ini file.
				time_steady = par("steady");
		}

		cout << "STEADY_TIME: " << time_steady << endl;

		string startMode = par("start_mode");       // Hot vs Cold start.
		string fillMode = par("fill_mode");			// Naive vs Model.

		// Downsizing factor. In case of ED-SIM: downsize = 1.
		long unsigned int downsize_temp = par("downsize");
		downsize = (long long int)downsize_temp;
		unsigned long long M = content_distribution::zipf[0]->get_catalog_card();
		unsigned long newCard = round(M*(1./(double)downsize));

		cout << "*** ORIGINAL CATALOG (from Statistics module): " << M << endl;
		cout << "*** DOWNSCALING VALUE (from Statistics module): " << downsize << endl;
		cout << "*** DOWNSCALED CATALOG (from Statistics module): " << newCard << endl;

		partial_n = par("partial_n");	// Number of nodes whose state will be checked.
		variance_threshold = par("variance_threshold");

		// Coefficient of Variation (CV) and Consistency thresholds.
        cvThr = par("cvThr");
        consThr = par("consThr");

		if (partial_n < 0 || partial_n > 1)
		{
			std::stringstream ermsg;
			ermsg<<"The parameter \"partial_n\" indicates the proportion of nodes to be considered!"
					"It must be in the range [0,1]. Please correct.";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}

		partial_n = round(partial_n*(double)num_nodes);


		cTopology topo;

		//	Extracting client modules according to the simulated type (IRM or ShotNoise).
		string clType = getAncestorPar("client_type");
		string clBase = "modules.clients.";
		clBase+=clType;
		clients = new client* [num_clients];
		clients_id.resize(num_clients);
		vector<string> clients_vec(1,clBase);
		topo.extractByNedTypeName(clients_vec);
		int k = 0;
		for (int i = 0;i<topo.getNumNodes();i++)
		{
		    int c = ((client *)topo.getNode(i)->getModule())->getNodeIndex();
		    if (find (content_distribution::clients, content_distribution::clients + num_clients, c)
			!= content_distribution::clients + num_clients)
		    {
		    	clients[k] = (client *)topo.getNode(i)->getModule();
		    	clients_id[k] = c;
		    	k++;
		    }
		}
    
		total_replicas = *(content_distribution::total_replicas_p );

		if ( content_distribution::repo_popularity_p != NULL )
		{
			for (unsigned repo_idx =0; repo_idx < content_distribution::repo_popularity_p->size(); repo_idx++)
			{
				char name[30];

				sprintf ( name, "repo_popularity[%u]",repo_idx);
				double repo_popularity = (*content_distribution::repo_popularity_p)[repo_idx];
				recordScalar(name,repo_popularity);
			}


			#ifdef SEVERE_DEBUG
				double repo_popularity_sum = 0;
				for (unsigned repo_idx =0; repo_idx < content_distribution::repo_popularity_p->size();
					 repo_idx++)
				{
					repo_popularity_sum += (*content_distribution::repo_popularity_p)[repo_idx];
				}

				if ( ! double_equality(repo_popularity_sum, 1) )
				{
					std::stringstream ermsg;
					ermsg<<"pop_indication_sum="<<repo_popularity_sum<<
						". It must be 1";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			#endif
		} // else info about repo popularity are not calculated


		// Extracting cache and core modules.
		caches = new base_cache*[num_nodes];
		cores = new core_layer*[num_nodes];
		vector<string> nodes_vec(1,"modules.node.node");
		topo.extractByNedTypeName(nodes_vec);

		for (int i = 0;i<topo.getNumNodes();i++)
		{
			//caches[i] = (base_cache *) (topo.getNode(i)->getModule()->getModuleByPath("content_store"));
			caches[i] = (base_cache *) (topo.getNode(i)->getModule()->getSubmodule("content_store"));
			//cores [i] = (core_layer *) (topo.getNode(i)->getModule()->getModuleByPath("core_layer"));
			cores [i] = (core_layer *) (topo.getNode(i)->getModule()->getSubmodule("core_layer"));
		}

		//	Store samples for stabilization
		samples.resize(num_nodes);
		events.resize(num_nodes);
		stable_nodes.resize(num_nodes);
		stable_with_traffic_nodes.resize(num_nodes);
		for (int n=0; n < num_nodes; n++)
		{
			events[n] = 0;
			stable_nodes[n] = false;
			stable_with_traffic_nodes[n] = false;
		}


		stable_check = new cMessage("stable_check",STABLE_CHECK);
		end = new cMessage("end",END);

		cout<<endl;

		// Full_check, Cold vs Hot, have a meaning only with ED-SIM (i.e., when RS != TTL)
		//string forwStr = caches[0]->getParentModule()->par("RS");
		
		string forwStr = caches[0]->getParentModule()->par("RS");
		cout << "REPLACEMENT STRATEGY:\t" << forwStr << endl;
		if(forwStr.compare("ttl_cache") != 0)
		{
			full_check = new cMessage("full_check", FULL_CHECK);

			if(startMode.compare("cold") == 0)	// COLD start: simulation starts with empty caches.
			{
				tStartCold = chrono::high_resolution_clock::now();
				tEndFill = chrono::high_resolution_clock::now();
				scheduleAt(simTime() + ts, full_check);		// Start checking for full state
				//scheduleAt(simTime() + ts, stable_check);
			}
			else if(startMode.compare("hot") == 0)	// HOT start: simulation starts with full caches.
			{
				/*
				 * Check the filling mode:
				 *  - naive: all the caches will be filled with the |cache_size| most popular contents
				 *  - model: each cache will be filled with |cache_size| objects according to the p_in
				 *  		 calculated by the model.
				 */
				if(fillMode.compare("model") == 0)
				{
					tStartHot = chrono::high_resolution_clock::now();

					if(num_repos == 1)
						cacheFillModel_Scalable_Approx("init");		// No Neigh Matrix; Approx. (only for tree or single repo)
					else if(num_repos > 1)
						cacheFillModel_Scalable_Approx_NRR("init");	// Neigh Matrix; Approx (possible NRR and multiple Repo)
					else
					{
						std::stringstream ermsg;
						ermsg<<"Number of Repos = 0! Please correct.";
						severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
					}


					tEndHot = chrono::high_resolution_clock::now();
					auto duration = chrono::duration_cast<chrono::milliseconds>( tEndHot - tStartHot ).count();
					cout << "Execution time of the MODEL [ms]: " << duration << endl;

					// Schedule simulation end according to the steady time.
					// *** PUT EXIT for perf eval
					if(onlyModel) 	// Model execution without cache filling
					{
						cout << "Model Execution - Sim STOPPED for performance evaluation." << endl;
						exit(1);
					}

					scheduleAt(simTime() + time_steady, end);
				}
				else if(fillMode.compare("naive") == 0)
				{
					cacheFillNaive();
					// Schedule simulation end according to the steady time.
					scheduleAt(simTime() + time_steady, end);
				}
				else
				{
					cout << "Error: specify a valid cache fill mode ('model' or 'naive').\n";
					exit(1);
				}
			}
			else
			{
				cout << "Error: specify a valid start mode ('hot' or 'cold').\n";
				exit(1);
			}
		}
		else
		{
			scheduleAt(simTime() + ts, stable_check);
		}
	}
}


int statistics::numInitStages() const
{
	return 2;
}

/*
 * 	Function to handle timers aimed at checking the state of the caches, i.e.,
 * 	when they become full, and when their hit rate remains stable within a certain threshold.
 */
void statistics::handleMessage(cMessage *in)
{
	int full = 0;
    int stables = 0;
    int numActiveNodes = 0;

    switch (in->getKind()){
    case FULL_CHECK:
    	for (int i = 0; i < num_nodes;i++)
    	{
    		if(cores[i]->interests!=0)
    			full += (int)caches[i]->full();
    		else
    			full += 1; 						// Inactive nodes are considered full for convergence purposes
    	}

        //if (full >= partial_n || simTime()>=10*8000)
        if (full >= partial_n || simTime()>=10*3600)
        {
        	tEndFill = chrono::high_resolution_clock::now();
        	auto duration = chrono::duration_cast<chrono::milliseconds>( tEndFill - tStartCold ).count();
        	cout << "Execution time of the FILL [ms]: " << duration << endl;

        	cout<<"Caches filled at time "<< simTime() <<endl;
        	scheduleAt(simTime() + ts, stable_check);	// Schedule the 'stable' check.
        	delete full_check;

        	double phitNode;
    		double phitTot = 0;
    		for (int i=0; i < num_nodes; i++)
    		{
    			if(caches[i]->hit != 0)
    			{
    				phitNode = caches[i]->hit * 1./ ( caches[i]->hit + caches[i]->miss );
    				phitTot += phitNode;
    				cout << "Node # " << i << " pHit: " << phitNode << endl;
    				phitNode = 0;
    			}
    			else
    			{
    				phitTot += 0;
    			}
    		}
    		cout << "SIMULATION - Total MEAN HIT PROB AFTER CACHE FILL: " << phitTot * 1./(double)num_nodes << endl;
    		clear_stat();		// Clear the statistics gathered during the transient time.
        }
        else
        	scheduleAt(simTime() + ts, in);		// Reschedule a 'full' check.
        break;

    case STABLE_CHECK:
    	for (int i = 0;i<num_nodes;i++)
    	{
    		if(!stable_nodes[i])
    		{
    			stable_nodes[i] = stable(i);
    			stables += (int) stable_nodes[i];

    		}
    		else
    			stables += 1;
    	}

    	if ( stables >= partial_n )
    	{
    		// Due to the fact that inactive nodes are considered as "stable",
    		// a further control is needed in order to check if a representative
    		// parte of the stable nodes (in this case half of them) effectively received traffic.
    		// This is especially needed for downscaled TTL-based simulations,
    		// and in particular for those cases where more simulation cycles are needed
    		// in order to correct the input Tc values. In case the next condition is not verified,
    		// the simulation continues with the transient state.

    		int stable_with_traffic = 0;
    		for (int i=0; i < num_nodes; i++)
    		{
    			if(stable_nodes[i]  &&  stable_with_traffic_nodes[i])
    			{
    				stable_with_traffic++;
    			}
    			else
    			{
    				stable_nodes[i] = false;
    				stables--;
    			}
    		}
    		cout << "**##**  ACTIVE NODES BTW STABLES:\t" << stable_with_traffic << endl;

    		if(stable_with_traffic >= floor(partial_n/2))
    		{
				for (int i = 0;i<num_nodes;i++)
				{
					caches[i]->stability = true;

					// In case of 2-LRU, signal the name cache for stability
					DecisionPolicy* decisor = caches[i]->get_decisor();
					Two_Lru* twoLruDecisor = dynamic_cast<Two_Lru *> (decisor);
					if(twoLruDecisor)			// 2-LRU-LRU
						twoLruDecisor->nc_stable = true;
				}

				for(int i=0; i<num_clients; i++)
					clients[i]->stability = true;

				cout << "*** FULL STABLE ***" << endl;

				// *****  NB  *** Insert for the link failure scenario
				//if(scheduleEnd)
				scheduleAt(simTime() + time_steady, end);	// Schedule the end of the simulation according to the steady
																// time after stabilization.
				double phitNode;
				double phitTot = 0;
				int numActiveNodes = 0;
				for (int i=0; i < num_nodes; i++)
				{
					if(cores[i]->interests != 0)
					{
						phitNode = caches[i]->hit * 1./ ( caches[i]->hit + caches[i]->miss );
						phitTot += phitNode;
						cout << "Node # " << i << " pHit: " << phitNode << endl;
						phitNode = 0;
						numActiveNodes++;

						cores[i]->stable = true;

					}
					else
					{
						phitTot += 0;
						//numActiveNodes++;

						cores[i]->stable = true;
					}
				}
				cout << "SIMULATION - Total MEAN HIT PROB AFTER STABILIZATION: " << phitTot * 1./(double)numActiveNodes << endl;

				if(!scheduleEnd)
					cout << "Number of Active Nodes 1: " << numActiveNodes << endl;
				else
					cout << "Number of Active Nodes 2: " << numActiveNodes << endl;
				cout << "SIMULATION - Stabilization reached at: " << simTime() << endl;

				tEndStable = chrono::high_resolution_clock::now();

				// ** Stability time measured since the beginning
				auto duration = chrono::duration_cast<chrono::milliseconds>( tEndStable - tStartGeneral ).count();
				cout << "Execution time of the STABILIZATION [ms]: " << duration << endl;

				stability_has_been_reached();

				for (int n=0; n < num_nodes; n++)
				{
					events[n] = 0;
				}
    		}
    		else  // Continue the simulation with the transient state (reschedule a 'stable' check)
    			scheduleAt(simTime() + ts, in);
    	}
    	else {
    		//cout << "NOT STABLE!" << endl;
    		scheduleAt(simTime() + ts, in);		// Reschedule a 'stable' check.
    	}

    	break;
    case END:
    	tEndGeneral = chrono::high_resolution_clock::now();
    	auto duration = chrono::duration_cast<chrono::milliseconds>( tEndGeneral - tStartGeneral ).count();
    	cout << "Execution time of the SIMULATION [ms]: " << duration << endl;
    	//delete in;

    	// Dynamic evalutaion of Tc in TTL-based scenario
    	string forwStr = caches[0]->getParentModule()->par("RS");
		if(forwStr.compare("ttl_cache") == 0)
		{
			if(!dynamic_tc)
			{
				delete in;
				endSimulation();
			}
			else  // *** DYNAMIC TC ***
			{
				// Compare the SUM of the actual cache sizes of each node
				double Sum_avg_as_cur = 0.0;

    	    	numActiveNodes = 0;


    	    	for (int i=0; i < num_nodes; i++)
    	    	{
    	    		if(cores[i]->interests != 0)
    	    		{
    	    			// For the consistency check on Ck we count only those nodes that have been stated as STABLE
    	    			// (considering partial_n) and that are active
    	    			if(stable_nodes[i])
    	    			{
    	    				cout << "** Consistency Check on NODE # " << i << endl;
    	    				numActiveNodes++;
    	    				Sum_avg_as_cur += abs(dynamic_cast<ttl_cache*>(caches[i])->avg_as_curr - dynamic_cast<ttl_cache*>(caches[i])->target_cache); //SUM of ABS
    	    			}

   	    				if(abs(dynamic_cast<ttl_cache*>(caches[i])->target_cache - dynamic_cast<ttl_cache*>(caches[i])->avg_as_curr)/dynamic_cast<ttl_cache*>(caches[i])->target_cache < 0.1)
   	    				{
   	    					dynamic_cast<ttl_cache*>(caches[i])->change_tc = false;   // Stop tc changing
   	    				}

    	    		}
    	    		else
    	    			dynamic_cast<ttl_cache*>(caches[i])->change_tc = false;   // Do not change Tc for inactive nodes.
    	    	}



    	    	// Only active nodes count
    	    	double Sum_target_cache = dynamic_cast<ttl_cache*>(caches[0])->target_cache * numActiveNodes;

    	    	cout << "CYCLE " << sim_cycles << " -\tNUM of ACTIVE NODES among the PARTIAL_N NODES: " << numActiveNodes << endl;

    	    	if(Sum_avg_as_cur/Sum_target_cache < consThr || sim_cycles > 20)
    	    	{
    	    		cout << " *** SIMULATION ENDED AT CYCLE:\t" << sim_cycles << endl;
    	    		delete in;
    	    		endSimulation();
    	    	}
    			else
    			{
    				cout << " *** CONDITION NOT VERIFIED! CONTINUE SIMULATION ***" << endl;
    				sim_cycles++;

    				for (int i=0; i < num_nodes; i++)
    				{
    					cout << "NODE:\t" << i << "\n\tOLD Tc = " << caches[i]->tc_node << "\n";
    					dynamic_cast<ttl_cache*>(caches[i])->extend_sim();			// Set the new Tc
    					cout << "\tNEW Tc = " << caches[i]->tc_node << "\n";
    					cout << "\tOnline Avg Cache Size = " << dynamic_cast<ttl_cache*>(caches[i])->avg_as_curr << "\n";
    				}


    				string decision_policy = caches[0]->par("DS");

    				if (decision_policy.compare("two_ttl")==0)   // Change the Tc of the name cache accordingly
    				{
    					for (int i=0; i < num_nodes; i++)
    					{
    						Two_TTL* tTTLPointer = dynamic_cast<Two_TTL *> (caches[i]->get_decisor());
    						tTTLPointer->extend_sim();
    					}
    				}

   					// Clear statistics of the current cycle
   					clear_stat();

   					// Reset the stable nodes
   					for(int i=0; i<num_nodes; i++)
   					{
   						stable_nodes[i] = false;
   						stable_with_traffic_nodes[i] = false;
   					}

   					scheduleAt(simTime() + ts, stable_check);  // Schedule another stability check.
   				}
   	    	}
		}
		else		// NOT TTL-based scenario. Delete msg and end simulation.
		{

			// *** Link Load Evaluation ***
			int nextNode;
			double currentBitPerSec;
			double currentChLoad;
			if(cores[0]->llEval)
			{
				for(int j=0; j<num_nodes; j++)
				{
					if(j==0)
					{
						for(int i=0; i < cores[j]->gateSize("face$o") - 1; i++)
						{
							nextNode = cores[j]->getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 1)
							{
								cout << "CAT CARD from CORE = " << cores[0]->catCard << endl;
								for(long k=0; k < cores[0]->catCard; k++)
								{
									currentBitPerSec = cores[j]->numBits[i][k]/(SIMTIME_DBL(simTime())-stabilization_time);
									currentChLoad = currentBitPerSec/cores[j]->datarate;
									cout << "TIER 1 - Content # " << k+1 << "\tLOAD - " << currentChLoad << endl;
								}
							}
						}

					}
					else if(j==1)
					{
						for(int i=0; i < cores[j]->gateSize("face$o") - 1; i++)
						{
							nextNode = cores[j]->getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 3)
							{
								for(long k=0; k<cores[0]->catCard; k++)
								{
									currentBitPerSec = cores[j]->numBits[i][k]/(SIMTIME_DBL(simTime())-stabilization_time);
									currentChLoad = currentBitPerSec/cores[j]->datarate;
									cout << "TIER 2 - Content # " << k+1 << "\tLOAD - " << currentChLoad << endl;
								}
							}
						}
					}
					else if(j==3)
					{
						for(int i=0; i < cores[j]->gateSize("face$o") - 1; i++)
						{
							nextNode = cores[j]->getParentModule()->gate("face$o",i+1)->getNextGate()->getOwnerModule()->getIndex();
							if(nextNode == 7)
							{
								for(long k=0; k<cores[0]->catCard; k++)
								{
									currentBitPerSec = cores[j]->numBits[i][k]/(SIMTIME_DBL(simTime())-stabilization_time);
									currentChLoad = currentBitPerSec/cores[j]->datarate;
									cout << "TIER 3 - Content # " << k+1 << "\tLOAD - " << currentChLoad << endl;
								}
							}
						}
					}
				}
			}

			delete in;
			endSimulation();
		}
   }
}

/*
 * 	Check the hit rate stability of a node.
 *
 * 	Parameters:
 * 		- n: node ID
 */
bool statistics::stable(int n)
{
    bool stable = false;
    double var = 0.0;
    double mean = 0.0;
    double rate = caches[n]->hit * 1./ ( caches[n]->hit + caches[n]->miss );

    //	Only hit rates matter.
    if (caches[n]->hit != 0 )
    {
    	if((caches[n]->decision_yes + caches[n]->hit) != events[n])  // If something is changed wrt the previous sample
    	{
    		samples[n].push_back(rate);		// Collect a sample.

    		if (samples[n].size()==window+1)
    			samples[n].erase(samples[n].begin()); // we try to maintain constant size while inserting new elements.

    		events[n] = caches[n]->decision_yes + caches[n]->hit;
    	}
    	// else: do not collect the sample
    }
    else
    {
    	if(caches[n]->miss == 0)      	// Inactive node  (we should state it as stable)
    		samples[n].push_back(0);
    	else							// Node which has experienced only miss events so far -> do not collect the sample
    									// we will start only by the first hit.
    		events[n] = caches[n]->decision_yes + caches[n]->hit;
    }

    if ( samples[n].size() == window )	// Calculate the variance each window samples.
	{
    	var = variance(samples[n]);
    	mean = average(samples[n]);
    	cout << "NODE # " << n << " Variance: " << var << " SIM TIME: " << simTime() << endl;
        double cv;
    	if (mean > 0.1)
    		cv = cvThr;
    	else
    		cv = 0.1;
    	if ( sqrt(var) <= cv * mean)
        {
            stabilization_time = SIMTIME_DBL(simTime());
            stable = true;
            cout << "NODE # " << n << " is STABLE at # " << simTime() << " with Samples:" << endl;
            for (unsigned int i=0; i < samples[n].size(); i++)
            	cout << samples[n][i] << " - ";
            cout << endl;

            // Set stable flag inside "base_cache"
            caches[n]->stability = true;

            // Check if it is an active node
            if(cores[n]->interests)
            	stable_with_traffic_nodes[n]=true;

        }
        samples[n].clear();		// Clear the collected samples.
    }
    return stable;
}

// Print statistics.
void statistics::finish()
{
	char name[30];

    uint32_t global_hit = 0;
    uint32_t global_miss = 0;
    uint32_t global_interests = 0;
    uint32_t global_data      = 0;
    double global_hit_ratio = 0;

    uint32_t global_repo_load = 0;
	long total_cost = 0;

    double global_avg_distance = 0;
    simtime_t global_avg_time = 0;
    uint32_t global_tot_downloads = 0;

    #ifdef SEVERE_DEBUG
    unsigned int global_interests_sent = 0;
    #endif

    int active_nodes = 0;
    for (int i = 0; i<num_nodes; i++)
	{
    	//TODO: do not always compute cost. Do it only when you want to evaluate the cost in your network
		total_cost += cores[i]->repo_load * cores[i]->get_repo_price();

		if (cores[i]->interests)	// Check if the considered node has received Interest packets.
		{
			active_nodes++;
			global_hit  += caches[i]->hit;
			global_miss += caches[i]->miss;
			global_data += cores[i]->data;
			global_interests += cores[i]->interests;
			global_repo_load += cores[i]->repo_load;
			global_hit_ratio += caches[i]->hit * 1./(caches[i]->hit+caches[i]->miss);

			#ifdef SEVERE_DEBUG
				if (	caches[i]->decision_yes + caches[i]->decision_no +
						(unsigned) cores[i]->unsolicited_data
						!=  (unsigned) cores[i]->data + cores[i]->repo_load
				){
					std::stringstream ermsg;
					ermsg<<"caches["<<i<<"]->decision_yes="<<caches[i]->decision_yes<<
						"; caches[i]->decision_no="<<caches[i]->decision_no<<
						"; cores[i]->data="<<cores[i]->data<<
						"; cores[i]->repo_load="<<cores[i]->repo_load<<
						"; cores[i]->unsolicited_data="<<cores[i]->unsolicited_data<<
						". The sum of "<< "decision_yes and decision_no must be data";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			#endif
		}
    }

	cout << "Num Active Nodes: " << active_nodes << endl;
	cout << "Num Total Nodes: " << num_nodes << endl;

    // Print and store global statistics

    // The global_hit is the mean hit rate among all the caches.
    sprintf (name, "p_hit");
    //recordScalar(name,global_hit * 1./(global_hit+global_miss));
    recordScalar(name,global_hit_ratio * 1./active_nodes);
    //cout<<"p_hit/cache: "<<global_hit *1./(global_hit+global_miss)<<endl;
    cout<<"p_hit/cache: "<<global_hit_ratio * 1./active_nodes<<endl;

    // Mean number of received Interest packets per node.
    sprintf ( name, "interests");
    recordScalar(name,global_interests * 1./num_nodes);

    // Mean number of received Data packets per node.
    sprintf ( name, "data" );
    recordScalar(name,global_data * 1./num_nodes);

    vector<double> global_scheduledReq;
    vector<double> global_validatedReq;
    ShotNoiseContentDistribution* snmPointer;

    // Statistics for popularity classes (only for Shot Noise model)
    cModule* pSubModule = getParentModule()->getSubmodule("content_distribution");
    if (pSubModule)
    {
    	snmPointer = dynamic_cast<ShotNoiseContentDistribution*>(pSubModule);
    	if (snmPointer)
    	{
    		global_scheduledReq.resize(snmPointer->numOfClasses,0);
    		global_validatedReq.resize(snmPointer->numOfClasses,0);
    	}
    }


    for (int i = 0;i<num_clients;i++)
    {
		global_avg_distance += clients[i]->get_avg_distance();
		global_tot_downloads += clients[i]->get_tot_downloads();
		global_avg_time  += clients[i]->get_avg_time();
		//<aa>
		#ifdef SEVERE_DEBUG
		global_interests_sent += clients[i]->get_interests_sent();
		#endif
		//</aa>


		if (snmPointer)
		{
			for(int j=0; j<snmPointer->numOfClasses; j++)
			{
				global_scheduledReq[j] += clients[i]->getScheduledReq(j);
				global_validatedReq[j] += clients[i]->getValidatedReq(j);
			}
		}
	}

    // Mean hit distance.
    sprintf ( name, "hdistance");
    recordScalar(name,global_avg_distance * 1./num_clients);
    cout<<"Distance/client: "<<global_avg_distance * 1./num_clients<<endl;

    // Mean download time.
    sprintf ( name, "avg_time");
    recordScalar(name,global_avg_time * 1./num_clients);
    cout<<"Time/client: "<<global_avg_time * 1./num_clients<<endl;

    // SNM statistics
    if(snmPointer)
    {
    	for(int j=0; j<snmPointer->numOfClasses; j++)
    	{
    		sprintf(name, "Scheduled_requests_Class_%d", j+1);		// Absolute number of scheduled requests for that class.
    		recordScalar(name, global_scheduledReq[j]);

    		sprintf(name, "Scheduled_requests_perc_Class_%d", j+1);		// Percentage of scheduled requests.
    		recordScalar(name, global_scheduledReq[j] * 1./snmPointer->totalRequests);

    		sprintf(name, "Validated_requests_Class_%d", j+1);		// Absolute number of validated requests for that class.
    		recordScalar(name, global_validatedReq[j]);

    		sprintf(name, "Validated_requests_relative_perc_Class_%d", j+1);		// Relative percentage of validated requests.
    		recordScalar(name, global_validatedReq[j] * 1./global_scheduledReq[j]);

       		sprintf(name, "Validated_requests_abslute_perc_Class_%d", j+1);		// Absolute percentage of validated requests.
        	recordScalar(name, global_validatedReq[j] * 1./snmPointer->totalRequests);

    		sprintf(name, "Suppressed_requests_Class_%d", j+1);
    		recordScalar(name, global_scheduledReq[j]-global_validatedReq[j]);
    	}
    }

	// Total number of completed downloads (sum over all clients).
    sprintf ( name, "downloads");
    recordScalar(name,global_tot_downloads);

    sprintf ( name, "total_cost");
    recordScalar(name,total_cost);
    cout<<"total_cost: "<<total_cost<<endl;


    sprintf ( name, "total_replicas");
    recordScalar(name,total_replicas);
    cout<<"total_replicas: "<<total_replicas<<endl;

    // It is the fraction of traffic that is satisfied by some cache inside
    // the network, and thus does not exit the network
    sprintf (name, "inner_hit");
    recordScalar(name , (double) (global_tot_downloads - global_repo_load) / global_tot_downloads) ;

    #ifdef SEVERE_DEBUG
	if (global_tot_downloads == 0)
	{
	       	std::stringstream ermsg;
		ermsg<<"global_tot_downloads == 0";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

		sprintf ( name, "interests_sent");
		recordScalar(name,global_interests_sent);
		cout<<"interests_sent: "<<global_interests_sent<<endl;

		if (global_interests_sent != global_tot_downloads){
			std::stringstream ermsg;
			ermsg<<"interests_sent="<<global_interests_sent<<"; tot_downloads="<<
				global_tot_downloads<<
				". If **.size==1 in omnetpp.ini and all links have 0 delay, this "<<
				" is an error. Otherwise, it is not";
			debug_message(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	#endif


    //TODO per content statistics
    //double hit_rate;
    // for (uint32_t f = 1; f <=content_distribution::perfile_bulk; f++){
    //     hit_rate = 0;
    //     if(hit_per_file[f]!=0)
    //         hit_rate = hit_per_file[f] / ( hit_per_file[f] +miss_per_file[f] );
    //     hit_per_fileV.recordWithTimestamp(f, hit_rate);
    //}

	delete [] caches;
	delete [] cores;
	delete [] clients;
}

void statistics::clear_stat()
{
	for (int i = 0;i<num_clients;i++)
	if (clients[i]->is_active() )
	    clients[i]->clear_stat();

    for (int i = 0;i<num_nodes;i++)
        cores[i]->clear_stat();

    for (int i = 0;i<num_nodes;i++)
	    caches[i]->clear_stat();
}

void statistics::stability_has_been_reached(){
	char name[30];
	sprintf (name, "stabilization_time");
	recordScalar(name,stabilization_time);
	cout<<"stabilization_time: "<< stabilization_time <<endl;

	clear_stat();
}

void statistics::registerIcnChannel(cChannel* icn_channel){
	#ifdef SEVERE_DEBUG
	if ( std::find(icn_channels.begin(), icn_channels.end(), icn_channel)
			!=icn_channels.end()
	){
        std::stringstream ermsg;
		ermsg<<"Trying to add to statistics object an icn channel already added"<<endl;
	    severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	icn_channels.push_back(icn_channel);
}

void statistics::checkStability()
{
	//stable_check = new cMessage("stable_check",STABLE_CHECK);

	//take(stable_check);
	clear_stat();
	tEndFill = chrono::high_resolution_clock::now();
	scheduleEnd = true;
	scheduleAt(simTime() + ts, stable_check);
}



// *** In the following: FUNCTIONS related to ANALYTICAL MODEL SOLVING and other UTILITIES (like cache filling) ***
double statistics::calculate_phit_neigh(int node_ID, int cont_ID, float **ratePrev, float **Pin, float **Phit, double *tcVect, double alphaExp, double lambdaVal, long catCard, bool* clientVect, vector<vector<map<int,int> > > &neighMat)
{
    double num;
    double lambda_ex_cont_ID;       // Exogenouas rate for content ID.

    double p_hit_tot = 0.0;
    double probNorm = 0.0;

    if (ratePrev[node_ID][cont_ID] != 0)    // Only if there is incoming traffic the hit probability can be greater than 0.
    {
		if(clientVect[node_ID])
		{
			num = (double)(1.0/(double)pow(cont_ID+1,alphaExp));
			lambda_ex_cont_ID = (double)(num*content_distribution::zipf[0]->get_normalization_constant())*(lambdaVal);
			probNorm += lambda_ex_cont_ID;      // It will be added to the sum of the incoming miss streams.
		}

		if (neighMat[node_ID][cont_ID].size() > 0)
		{
			int neigh;

			//for(uint32_t i=0; i < neighMat[node_ID][cont_ID].size(); i++)
			for (std::map<int,int>::iterator it = neighMat[node_ID][cont_ID].begin(); it!=neighMat[node_ID][cont_ID].end(); ++it)
			{
				neigh = it->first;
				if(ratePrev[neigh][cont_ID]!=0)
					probNorm += (ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))*(1./it->second); // we take into account ri,j
			}

			if(probNorm == 0)
			{
				p_hit_tot = 0;
				return p_hit_tot;
			}

			//cout << "PROB NORM: " << probNorm << endl;


			//for(uint32_t i=0; i < neighMat[node_ID][cont_ID].size(); i++)
			for (std::map<int,int>::iterator it = neighMat[node_ID][cont_ID].begin(); it!=neighMat[node_ID][cont_ID].end(); ++it)
			{
				neigh = it->first;
				if (ratePrev[neigh][cont_ID]!=0)
				{
					double Aij  = 0;
					double partial_sum = 0;
					//for (uint32_t j=0; j < neighMat[node_ID][cont_ID].size(); j++)  // If there are at least 2 neighbors (different from a client)
					for (std::map<int,int>::iterator it2 = neighMat[node_ID][cont_ID].begin(); it2!=neighMat[node_ID][cont_ID].end(); ++it2)
					{
						if (it2!=it)	// The 'j' neigh must be different than the one selected to calculate the conditional prob.
						{
							neigh = it2->first;
							if(ratePrev[neigh][cont_ID]!=0)
								partial_sum += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*tcVect[node_ID])*(1./it2->second);
								//partial_sum += (ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])* max((double)0, tcVect[node_ID]-tcVect[neigh]);
						}
					}
					if (clientVect[node_ID])
					{
						partial_sum += (lambda_ex_cont_ID)*tcVect[node_ID];
					}

					neigh = it->first;

					if(meta_cache == LCE)
					{
						Aij += (((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID]) * max((double)0, tcVect[node_ID]-tcVect[neigh]))*(1./it->second) + partial_sum);
						// *** a-NET style ***
						//Aij += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID]) * tcVect[node_ID] + partial_sum);

						// Sum conditional probabilities.
						p_hit_tot += (1 - exp(-Aij))*((ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))*(1./it->second)/probNorm);
					}
					else if(meta_cache == fixP)
					{
						// "partial_sum" only in the exponent with Tc2-Tc1
						Aij += (1-q)*(1-exp(-ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh])) +
								exp(-ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh]) *
								(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID]) * max(double(0), tcVect[node_ID] - tcVect[neigh]) + partial_sum)));
						// "partial_sum" in each exponent.
						//Aij += (1-q)*(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh] + partial_sum))) +
						//		exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh] + partial_sum)) *
						//		(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID]) * max(double(0), tcVect[node_ID] - tcVect[neigh]) + partial_sum)));


						p_hit_tot += ((q * Aij) / (1 - (1-q)*Aij))*((ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))/probNorm);
					}
					else
					{
						cout << "Meta Caching Algorithm NOT Implemented!" << endl;
						exit(0);
					}
				}
			}
		}
		if (clientVect[node_ID])  // we have to calculate the conditional probability with respect to the client
		{
			double Aij = 0;
			double partial_sum = 0;
			int neigh;
			//for (uint32_t j=0; j < neighMat[node_ID][cont_ID].size(); j++)
			if (neighMat[node_ID][cont_ID].size() > 0)
			{
				for (std::map<int,int>::iterator it = neighMat[node_ID][cont_ID].begin(); it!=neighMat[node_ID][cont_ID].end(); ++it)
				{
					neigh = it->first;

					if(ratePrev[neigh][cont_ID]!=0)
						partial_sum += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*tcVect[node_ID])*(1./it->second);
						//partial_sum += (ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*max((double)0, tcVect[node_ID]-tcVect[neigh]);
				}
			}

			if(meta_cache == LCE)
			{
				Aij += ((lambda_ex_cont_ID)*tcVect[node_ID] + partial_sum);
				p_hit_tot += (1 - exp(-Aij))*(lambda_ex_cont_ID/probNorm);
			}
			else if (meta_cache == fixP)
			{
				Aij += q * (1 - exp(-(lambda_ex_cont_ID*tcVect[node_ID] + partial_sum)));
				p_hit_tot += (Aij / (exp(-(lambda_ex_cont_ID*tcVect[node_ID] + partial_sum)) + Aij)) * (lambda_ex_cont_ID/probNorm);
			}
			else
			{
				cout << "Meta Caching Algorithm NOT Implemented!" << endl;
				exit(0);
			}
		}
    }

	return p_hit_tot;
}



double statistics::MeanSquareDistance(uint32_t catCard, double **prevRate, double **currRate, int colIndex)
{
	double sumCurr = 0;
	double sumPrev = 0;
	//double sumVal = 0;
	double msdVal;
	for (uint32_t m=0; m < catCard; m++)
	{
		//sumVal += pow((currRate[colIndex][m] - prevRate[colIndex][m]),2);
		sumCurr += currRate[colIndex][m];
		sumPrev += prevRate[colIndex][m];
	}

	//msdVal = sumVal/(double)catCard;
	msdVal = abs(sumCurr-sumPrev);

	return msdVal;
}


void statistics::cacheFillNaive()
{
	cout << "Filling Caches with Naive method...\n" << endl;

	int N = num_nodes;									   			// Number of nodes.
	uint32_t M = content_distribution::zipf[0]->get_catalog_card();		// Number of contents.
	uint32_t cSize_targ = (double)caches[0]->get_size();

	double alphaVal = content_distribution::zipf[0]->get_alpha();
	double normConstant = content_distribution::zipf[0]->get_normalization_constant();  // Zipf's normalization constant.

	double *pZipf = new double[M];
	double num;

	uint32_t cont_id, name;

	double q = 1.0;

	DecisionPolicy* decisor;
	Fix* dp1;

	decisor = caches[0]->get_decisor();
	dp1 = dynamic_cast<Fix*> (decisor);;

	if(dp1)
	{
		q = dp1->p;
	}


	ccn_data* data = new ccn_data("data",CCN_D);

	for(int n=0; n < N; n++)
	{
		decisor = caches[n]->get_decisor();
		dp1 = dynamic_cast<Fix*> (decisor);;

		if(dp1)
		{
			dp1->setProb(1.0);
		}

		for (uint32_t m=0; m < M; m++)
		{
			num = (1.0/pow(m+1,alphaVal));
			pZipf[m] = (double)(num*normConstant);
		}

		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<uint32_t> uni(1,M); // guaranteed unbiased

		for (uint32_t k=0; k < cSize_targ; k++)
		{
			bool found = false;
			while(!found)
			{
				//name = std::floor((rand() / ((double)RAND_MAX+1))*(M + 1));
			    name = uni(rng);
				//if (pZipf[name] > dblrand())
				if (pZipf[name] != 0)
				{
					pZipf[name] = 0;
					found = true;
				}
			}

			cont_id = name;
			//cont_id = k+1;
			chunk_t chunk = 0;
			__sid(chunk, cont_id);
			__schunk(chunk, 0);
			data -> setChunk (chunk);
			data -> setHops(0);
			data->setTimestamp(simTime());
			caches[n]->store(data);
		}
		cout << "Cache Node # " << n << " :\n";
		caches[n]->dump();
		if(dp1)
		{
			dp1->setProb(q);
		}

	}
	cout << "MEAN hit ratio after CACHE hot start naive: unknown\n" << endl;
	delete data;
}


// *** CACHE MODEL SCALABLE ***
void statistics::cacheFillModel_Scalable_Approx(const char* phase)
{
	chrono::high_resolution_clock::time_point tStartAfterFailure;
	chrono::high_resolution_clock::time_point tEndAfterFailure;
	if(strcmp(phase,"init")!=0)
		tStartAfterFailure = chrono::high_resolution_clock::now();
	long M = (long)content_distribution::zipf[0]->get_catalog_card();		// Catalog cardinality.
	int N = num_nodes;												// Number of nodes.
	double Lambda = clients[0]->getLambda();						// Aggregate lambda of exogenous requests.
	uint32_t cSize_targ = (double)caches[0]->get_size();			// Target Cache size.

	cout << "Model CACHE SIZE = " << cSize_targ << endl;

	double alphaVal = content_distribution::zipf[0]->get_alpha();
	double normConstant = content_distribution::zipf[0]->get_normalization_constant();  // Zipf's normalization constant.

	string forwStr = caches[0]->getParentModule()->par("FS");
	cout << "*** Model Forwarding Strategy : " << forwStr << " ***" << endl;

	// Retrieve meta-caching algorithm
	DecisionPolicy* decisor = caches[0]->get_decisor();
	Always* dp1 = dynamic_cast<Always*> (decisor);
	if(dp1)
	{
		meta_cache = LCE;
		dpString = "LCE";
		cout << "*** LCE *** Decision Policy!" << endl;
	}
	else
	{
		Fix* dp2 = dynamic_cast<Fix*> (decisor);
		if(dp2)
		{
			meta_cache = fixP;
			q = dp2->p;
			dpString = "fixP";
			cout << "*** fixP *** Decision Policy with q = " << q << endl;
		}
		else
		{
			cout << "Decision policy not supported!" << endl;
			exit(0);
		}
	}

	cout << "***** CACHE FILLING WITH MODEL *****" << endl;

	// Definition and initialization of useful data structures.
	float **prev_rate;		// Request rate for each content at each node at the 'Previous Step'.
	float **curr_rate;		// Request rate for each content at each node at the 'Current Step'.
	float **p_in;			// Pin probability for each content at each node.
	float **p_hit;			// Phit probability for each content at each node.

	// All the previous structures will be matrices of size [M][N].
	prev_rate = new float*[N];
	curr_rate = new float*[N];
	p_in = new float*[N];
	p_hit = new float*[N];

	for (int i=0; i < N; i++)
	{
		prev_rate[i] = new float[M];
		curr_rate[i] = new float[M];
		p_in[i] = new float[M];
		p_hit[i] = new float[M];
	}

	double *tc_vect = new double[N]; 		// Vector containing the 'characteristic times' of each node.


	/*double **p_in_temp;				// Temp vector to find the max p_in per each node at the end of each iteration.
	p_in_temp = new double*[N];
	for(int n=0; n < N; n++)
			p_in_temp[n] = new double[M];
	 */

	double *pHitNode = new double[N];
	fill_n(pHitNode,N,0.0);
	double prev_pHitTot = 0.0;
	double curr_pHitTot = 0.0;


	double *sumCurrRate = new double[N];	// It will contain the total incoming rate for each node.
	fill_n(sumCurrRate,N,0.0);

	//vector<int> inactiveNodes;
	vector<int> activeNodes;

	// *** DISABLED FOR PERF EVALUATION ***
	int **steadyCache; 			// Matrix containing the IDs of the content the will be cached inside the active nodes for the hot start.
	if(!onlyModel)
	{
		steadyCache = new int*[N];
		for (int n=0; n < N; n++)
		{
			steadyCache[n] = new int[cSize_targ];
			fill_n(steadyCache[n], cSize_targ, M+1);
		}
	}

	// *** INITIALIZATION ***
	// At step '0', the incoming rates at each node are supposed to be the same,
	// i.e., prev_rate(i,n) = zipf(i) * Lambda.
	// As a consequence, the characteristic times of all the nodes will be initially the same, and so the Pin.

	int step = 0;
	double num = 0;
	bool climax;

	cout << "Iteration # " << step << " - INITIALIZATION" << endl;

	for (long m=0; m < M; m++)
	{
		num = (1.0/pow(m+1,alphaVal));
		prev_rate[0][m] = (float)(num*normConstant)*Lambda;
		sumCurrRate[0] += prev_rate[0][m];
	}

	for (int n=1; n < N; n++)
	{
		std::copy(&prev_rate[0][0], &prev_rate[0][M], prev_rate[n]);
		sumCurrRate[n] = sumCurrRate[0];
	}


	// The Tc will be initially the same for all the nodes, so we pass just the first column of the prev_rate.
	// In the following steps it will be Tc_val(n) = compute_Tc(...,prev_rate, n-1).

	double tc_val = compute_Tc_single_Approx(cSize_targ, alphaVal, M, prev_rate, 0, dpString, q);

	cout << "Computed Tc during initialization:\t" << tc_val << endl;

	if(meta_cache == LCE)
	{
		for (int n=0; n < N; n++)
		{
			tc_vect[n] = tc_val;
			for (long m=0; m < M; m++)
			{
				p_in[n][m] = 1 - exp(-prev_rate[n][m]*tc_vect[n]);
				p_hit[n][m] = p_in[n][m];
				pHitNode[n] += (prev_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}
			prev_pHitTot += pHitNode[n];
		}
	}

	else if (meta_cache == fixP)
	{
		for (int n=0; n < N; n++)
		{
			tc_vect[n] = tc_val;
			for (long m=0; m < M; m++)
			{
				p_in[n][m] = (q * (1.0 - exp(-prev_rate[n][m]*tc_vect[n])))/(exp(-prev_rate[n][m]*tc_vect[n]) + q * (1.0 - exp(-prev_rate[n][m]*tc_vect[n])));
				p_hit[n][m] = p_in[n][m];
				pHitNode[n] += (prev_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}
			prev_pHitTot += pHitNode[n];
		}
	}
	else
	{
		cout << "Meta Caching Algorithm NOT Implemented!" << endl;
		exit(0);
	}

	prev_pHitTot /= N;
	cout << "pHit Tot Init - " << prev_pHitTot << endl;


	// *** ITERATIVE PROCEDURE ***

	// Cycle over more steps
	int slots = 30;

	//double **MSD; 			// matrix[slots][N]; it will contain computed Mean Square Distance at each step of the iteration.
							// Useful as a stop criterion for the iterative procedure.

	vector<map<int,int> >  neighMatrix;
	neighMatrix.resize(N);

	/*vector<vector<map<int,double> > > rateNeighMatrix;
	rateNeighMatrix.resize(N);
	for(int n=0; n < N; n++)
		rateNeighMatrix[n].resize(M);*/


	bool* clientVector = new bool[N];					// Vector indicating if the current node has an attached client.
	for (int n=0; n < N; n++)
		clientVector[n] = false;


	vector<int> repoVector;


    int l;

    	// Retrieve the Repository storing all the contents

    	repo_t repo = __repo(1);    // The repo is the one associated to the position of the '1' inside the string
    	l = 0;
    	while (repo)
    	{
    		if (repo & 1)
			{
 				//repoVector[m] = content_distribution::repositories[l];  // So far, the replication of seed copies is supposed
    																    // to be 1 (i.e., the seed copy of each content is stored only in one repo)
    			//repoVector[m].push_back(content_distribution::repositories[l]);	// In case of more repos stories the seed copies of a content.
    			repoVector.push_back(content_distribution::repositories[l]);	// In case of more repos stories the seed copies of a content.
 				//if (m<20)
 				//	cout << "The Repo of content # " << m << " is: " << repoVector[m] << endl;
    		}
    		repo >>= 1;
    		l++;
    	}

	int outInt;
	int target;


	std::vector<int>::iterator itRepoVect;

	for (int n=0; n < N; n++)  // NODES
	{
	    for(int i=0; i < num_clients; i++)
	    {
	    	if(clients_id[i] == n)
	    	{
	    		clientVector[n] = true;
	    		break;
	    	}
	    }

	    itRepoVect = find (repoVector.begin(), repoVector.end(), n);
		if (itRepoVect != repoVector.end() )
			continue;
		else
		{
			for(itRepoVect = repoVector.begin(); itRepoVect != repoVector.end(); ++itRepoVect)
			{
				outInt = cores[n]->getOutInt(*itRepoVect);
				target = caches[n]->getParentModule()->gate("face$o",outInt)->getNextGate()->getOwnerModule()->getIndex();
				neighMatrix[target].insert( pair<int,int>(n,1));
			}
		}
	}


	// Iterations
	for (int k=0; k < slots; k++)
	{
		step++;
		cout << "Iteration # " << step << endl;

		// Calculate the 'current' request rate for each content at each node.
		for (int n=0; n < N; n++)			// NODES
		{
			double sum_curr_rate = 0;
			double sum_prev_rate = 0;

			//cout << "NODE # " << n << endl;
			for (long m=0; m < M; m++)		// CONTENTS
			{
				double neigh_rate = 0;			// Cumulative miss rate from neighbors for the considered content.

			    if (neighMatrix[n].size() > 0 || clientVector[n])	// Sum the miss streams of the neighbors for the considered content.
			    {
			    	//for (uint32_t i=0; i < neighMatrix[n][m].size(); i++)
			    	for (std::map<int,int>::iterator it = neighMatrix[n].begin(); it!=neighMatrix[n].end(); ++it)
			    	{
			    		// The miss stream of the selected neighbor will depend on its hit probability for the selected
			    		// content (i.e., Phit(neigh,m). This can be calculated using the conditional probabilities
			    		// Phit(neigh,m|j) = 1 - exp(-A_neigh_j). It expresses the probability that a request for 'm'
			    		// hits cache 'neigh', provided that it comes from cache 'j'. Only neighboring caches for which
			    		// the neigh represents the next hop will be considered.
			    		//int neigh = neighMatrix[n][m][i];
			    		int neigh = it->first;
			    		int numPot = it->second;    // number of potential targets for the neighbor

			    		if(meta_cache == LCE)
			    		{
			    			if(prev_rate[neigh][m]*tc_vect[neigh] <= 0.01)
			    				p_hit[neigh][m] = prev_rate[neigh][m]*tc_vect[neigh];
			    			else
			    				p_hit[neigh][m] = calculate_phit_neigh_scalable(neigh, m, prev_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
			    		}
			    		else if(meta_cache == fixP)
			    		{
			    			if(q*prev_rate[neigh][m]*tc_vect[neigh] <= 0.01)
			    				p_hit[neigh][m] = q*(prev_rate[neigh][m]*tc_vect[neigh]);
			    			else
			    				p_hit[neigh][m] = calculate_phit_neigh_scalable(neigh, m, prev_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
			    		}
			    		else
						{
							cout << "Meta Caching Algorithm NOT Implemented!" << endl;
							exit(0);
						}

						if (p_hit[neigh][m] > 1)
							cout << "Node: " << n << "\tContent: " << m << "\tNeigh: " << neigh << "\tP_hit: " << p_hit[neigh][m] << endl;

						//neigh_rate += (prev_rate[neigh][m]*(1-p_hit[neigh][m]));
						neigh_rate += (prev_rate[neigh][m]*(1-p_hit[neigh][m]))*(1./numPot);

						//rateNeighMatrix[n][m][neigh] = 0; 		// Reset the result of the previous iteration.
						//rateNeighMatrix[n][m][neigh] += (prev_rate[neigh][m]*(1-p_hit[neigh][m]));
			    	}
			    }


			    if (clientVector[n])	// In case a client is attached to the current node.
		    	{
					num = (1.0/(double)pow(m+1,alphaVal));
					neigh_rate += (num*normConstant)*Lambda;
		    	}

			    curr_rate[n][m] = neigh_rate;

			    sum_curr_rate += curr_rate[n][m];
			    sum_prev_rate += prev_rate[n][m];

			    prev_rate[n][m] = curr_rate[n][m];

			}  // contents

			//cout << "Iteration # " << step << " Node # " << n << " Incoming Rate: " << sum_curr_rate << endl;

			sumCurrRate[n] = sum_curr_rate;
			climax = false;

			// Calculate the new Tc and Pin for the current node.
			if (sum_curr_rate != 0) 	// The current node is hit either by exogenous traffic or by miss streams from
										// other nodes (or by both)
			{
				cout << "NODE # " << n << " Sum Current Rate: " << sumCurrRate[n] << endl;
				tc_vect[n] = compute_Tc_single_Approx(cSize_targ, alphaVal, M, curr_rate, n, dpString, q);

				cout << "Node # " << n << " - Tc " << tc_vect[n] << endl;
				if(meta_cache == LCE)
				{
					for(long m=0; m < M; m++)
					{
						if(!climax)
						{
							if(curr_rate[n][m]*tc_vect[n] <= 0.01)
							{
								p_in[n][m] = curr_rate[n][m]*tc_vect[n];
							}
							else
							{
								p_in[n][m] = 1 - exp(-curr_rate[n][m]*tc_vect[n]);
								climax = true;
							}
						}
						else
						{
							p_in[n][m] = 1 - exp(-curr_rate[n][m]*tc_vect[n]);
							if(curr_rate[n][m]*tc_vect[n] <= 0.01)
							{
								for (long z=m+1; z < M; z++)
									p_in[n][z] = curr_rate[n][z]*tc_vect[n];
							}
							break;

						}
					}
					climax = false;
				}
				else if(meta_cache == fixP)
				{
					for(long m=0; m < M; m++)
					{
						if(!climax)
						{

							if(q*curr_rate[n][m]*tc_vect[n] <= 0.01)
							{
								p_in[n][m] = q * curr_rate[n][m]*tc_vect[n];
							}
							else
							{
								p_in[n][m] = (q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])))/(exp(-curr_rate[n][m]*tc_vect[n]) + q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])));
								climax = true;
							}
						}
						else
						{
							p_in[n][m] = (q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])))/(exp(-curr_rate[n][m]*tc_vect[n]) + q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])));
							if(q*curr_rate[n][m]*tc_vect[n] <= 0.01)
							{
								for (long z=m+1; z < M; z++)
									p_in[n][z] = q * curr_rate[n][z]*tc_vect[n];
							}
							break;
						}
					}
					climax = false;
				}
				else
				{
					cout << "Meta Caching Algorithm NOT Implemented!" << endl;
					exit(0);
				}

				// Calculate the Phit of the node hosting one of the repos (it can happen that it does not forward any
				// traffic, thus it is not a neighbor for any node, and so the logic of the p_hit calculus followed
				// before does not apply.
				if (find (content_distribution::repositories, content_distribution::repositories + num_repos, n)
							!= content_distribution::repositories + num_repos)
				{
					for(long m=0; m < M; m++)
					{
						if(meta_cache == LCE)
						{
							if(curr_rate[n][m]*tc_vect[n] <= 0.01)
								p_hit[n][m] = curr_rate[n][m]*tc_vect[n];
							else
								p_hit[n][m] = calculate_phit_neigh_scalable(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else if(meta_cache == fixP)
						{
							if(q*curr_rate[n][m]*tc_vect[n] <= 0.01)
								p_hit[n][m] = q*(curr_rate[n][m]*tc_vect[n]);
							else
								p_hit[n][m] = calculate_phit_neigh_scalable(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else
						{
							cout << "Meta Caching Algorithm NOT Implemented!" << endl;
							exit(0);
						}
					}
				}
			}
			else		// It means that the current node will not be hit by any traffic.
						// So we zero the miss stream coming from this node by putting p_in=1 for each content.
			{
				tc_vect[n] = numeric_limits<double>::max();   // Like infinite value;
				//cout << "Iteration # " << step << " NODE # " << n << " Tc - " << tc_vect[n] << endl;
				for(long m=0; m < M; m++)
				{
					p_in[n][m] = 0;
					p_hit[n][m] = 0;
				}

			}

		} // nodes

		curr_pHitTot = 0;
		for(int n=0; n < N; n++)
		{
			pHitNode[n] = 0;
			if(sumCurrRate[n]!=0)
			{
				for (long m=0; m < M; m++)
				{
					pHitNode[n] += (curr_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
				}
				curr_pHitTot += pHitNode[n];
			}

		}

		// *** EXIT CONDITION ***
		if( (abs(curr_pHitTot - prev_pHitTot)/curr_pHitTot) < 0.005 )
			break;
		else		// For the NRR implementation, the actual cache fill is moved here in order to update the neighMatrix
					// computation at the next step
		{
			prev_pHitTot = curr_pHitTot;

			/// ***** TRY *****
			for(int n=0; n < N; n++)
			{
				if(sumCurrRate[n]!=0)
				{
					for (long m=0; m < M; m++)
					{
						curr_rate[n][m] = 0;
					}
				}
			}
		}
	}  // Successive step

	int maxPin;

	double pHitTotMean = 0;
	double pHitNodeMean = 0;

	activeNodes.clear();

	for(int n=0; n < N; n++)
	{
		// Allocate contents only for nodes interested by traffic (it depends on the frw strategy)
		if(tc_vect[n] != numeric_limits<double>::max())
		{
			activeNodes.push_back(n);

			// Calculate di p_hit mean of the node
			for(uint32_t m=0; m < M; m++)
			{
				pHitNodeMean += (curr_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}

			// *** DISABLED for perf measurements
			if(!onlyModel)
			{
				// *** DETERMINING CONTENTS TO BE PUT INSIDE CACHES***
				// Choose the contents to be inserted into the cache.
				for(uint32_t k=0; k < cSize_targ; k++)
				{
					maxPin = distance(p_in[n], max_element(p_in[n], p_in[n] + M));  // Position of the highest popular object (i.e., its ID).
					// *** NRR
					if(p_in[n][maxPin] == 0.0)			// There are few contents than cSize_targ that can be inside the cache
						break;
					steadyCache[n][k] = maxPin;

					//cout << "NODE # " << n << " Content # " << maxPin << endl;

					//p_in_temp[n][k] = p_in[n][maxPin];
					//p_hit_temp[n][k] = p_hit[n][maxPin];
					p_in[n][maxPin] = 0;
				}
			}


			pHitTotMean += pHitNodeMean;
			cout << "NODE # " << n << endl;
			cout << "-> Mean Phit: " << pHitNodeMean << endl;
//			cout << "-> Steady Cache: Content - Pin - Phit\n";
//			for (uint32_t k=0; k < cSize_targ; k++)
//				cout << steadyCache[n][k] << " - " << p_in_temp[n][k] << " - " << p_hit_temp[n][k] << endl;
			cout << endl;
			pHitNodeMean = 0;
		}
	}

	if(strcmp(phase,"init")==0)
		cout << "Number of Active Nodes 1: " << activeNodes.size() << endl;
	else
		cout << "Number of Active Nodes 2: " << num_nodes - activeNodes.size() << " Time: " << simTime() << endl;

	pHitTotMean = pHitTotMean/(activeNodes.size());
	//pHitTotMean = pHitTotMean/(N);

	if(strcmp(phase,"init")==0)
		cout << "MODEL - P_HIT MEAN TOTAL AFTER CACHE FILL: " << pHitTotMean << endl;
	else
		cout << "MODEL - P_HIT MEAN TOTAL AFTER ROUTE re-CALCULATION: " << pHitTotMean << endl;

	cout << "*** Tc of nodes ***\n";
	for(int n=0; n < N; n++)
	{
		cout << "Tc-" << n << " --> " << tc_vect[n] << endl;
	}

    // *** DISABLED per perf evaluation
	if(!onlyModel)
	{
		// *** REAL CACHE FILLING ***
		if(strcmp(phase,"init")==0)
		{
			cout << "Filling Caches with Model results...\n" << endl;
			int node_id;
			uint32_t cont_id;

			// In case of FIX probabilistic with need to temporarly change the caching prob id order to fill the cache
			DecisionPolicy* decisor;
			Fix* dp1;

			ccn_data* data = new ccn_data("data",CCN_D);

			for(unsigned int n=0; n < activeNodes.size(); n++)
			{
				node_id = activeNodes[n];

				decisor = caches[node_id]->get_decisor();
				dp1 = dynamic_cast<Fix*> (decisor);;

				if(dp1)
				{
					dp1->setProb(1.0);
				}

				// Empty the cache from the previous step.
				caches[node_id]->flush();

				//for (int k=cSize_targ-1; k >= 0; k--)   // Start inserting contents with the smallest p_in.
				for (unsigned int k=0; k < cSize_targ; k++)
				{
					cont_id = (uint32_t)steadyCache[node_id][k]+1;
					if (cont_id == M+2)			// There are few contents than cSize_targ that can be inside the cache
						break;
					chunk_t chunk = 0;
					__sid(chunk, cont_id);
					__schunk(chunk, 0);
					//ccn_data* data = new ccn_data("data",CCN_D);
					data -> setChunk (chunk);
					data -> setHops(0);
					data->setTimestamp(simTime());
					caches[node_id]->store(data);
				}
				//cout << "Cache Node # " << node_id << " :\n";
				//caches[node_id]->dump();

				if(dp1)
				{
					dp1->setProb(q);
				}

			}

			delete data;
		}
	}



	// DISABLED for perf eval
	// *** NEW LINK-LOAD EVALUATION WITH MEAN AND DISTRIBUTION ***

	/*double linkTraffic = 0;
	int neigh;
	vector<map<int,double> > totalNeighRate;
	totalNeighRate.resize(N);
	cout << "*** Link State ***" << endl;
	cout << "Extreme A -- Link Load [Bw=1Mbps] -- Extreme B" << endl;
	cout << endl;*/

	/*for (int n=0; n < N; n++)
	{
		for (int m=0; m < M; m++)
		{
			map<int,double>::iterator it = rateNeighMatrix[n][m].begin();
			while(it!=rateNeighMatrix[n][m].end())
			{
				neigh = it->first;
				map<int,double>::iterator itInner = totalNeighRate[n].find(neigh);
				if(itInner == totalNeighRate[n].end()) // Insert for the first time
					totalNeighRate[n][neigh] = it->second;
				else										// Neigh already inserted before.
					totalNeighRate[n][neigh] += it->second;
				++it;
			}
		}
	}*/

	/*for (int n=0; n < N; n++)
	{
		for (int m=0; m < M; m++)
		{
			map<int,double>::iterator it = rateNeighMatrix[n][m].begin();
			while(it!=rateNeighMatrix[n][m].end())
			{
				neigh = it->first;

				// *** DISTRIBUTION ***
				linkTraffic = it->second;
				map<int,double>::iterator itReverse = rateNeighMatrix[neigh][m].find(n);
				if(itReverse != rateNeighMatrix[neigh][m].end()) // There is traffic for that content also in the opposite direction
					linkTraffic += itReverse->second;
				if (n==0 && neigh==1)
				{
					cout << "TIER 1 - Content # " << m+1 << "\tLOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
				}
				else if (n==1 && neigh==3)
				{
					cout << "TIER 2 - Content # " << m+1 << "\tLOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
				}
				else if (n==3 && neigh==7)
				{
					cout << "TIER 3 - Content # " << m+1 << "\tLOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
				}
				linkTraffic = 0;
				// ***********************

				map<int,double>::iterator itInner = totalNeighRate[n].find(neigh);
				if(itInner == totalNeighRate[n].end()) // Insert for the first time
					totalNeighRate[n][neigh] = it->second;
				else								  // Neigh already inserted before.
					totalNeighRate[n][neigh] += it->second;
				++it;
			}
		}
	}

	for (int n=0; n < N; n++)
	{
		for(map<int,double>::iterator it=totalNeighRate[n].begin(); it!=totalNeighRate[n].end(); ++it)
		{
			neigh = it->first;
			linkTraffic += it->second;
			map<int,double>::iterator itInner = totalNeighRate[neigh].find(n);  // Sum the traffic in the opposite direction
			if(itInner != totalNeighRate[neigh].end())  // There is also traffic in the opposite direction.
			{
				linkTraffic += itInner->second;
				totalNeighRate[neigh].erase(itInner);
			}
			//cout << "Node " << n << "\t- " << (double)((linkTraffic*(1536*8))/1000000) << " -\tNode " << neigh << endl;
			if (n==0 && neigh==1)
			{
				cout << "TIER 1 - " << "\tMEAN LOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
			}
			else if (n==1 && neigh==3)
			{
				cout << "TIER 2 - " << "\tMEAN LOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
			}
			else if (n==3 && neigh==7)
			{
				cout << "TIER 3 - " << "\tMEAN LOAD - " << (double)((linkTraffic*(1536*8))/1000000) << endl;
			}
			linkTraffic = 0;
		}
	}*/

	// *******************************

	if(strcmp(phase,"init")!=0)
	{
		tEndAfterFailure = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>( tEndAfterFailure - tStartAfterFailure  ).count();
		cout << "Execution time of the model after failure [ms]: " << duration << endl;
	}
	// De-allocating memory
	for (int n = 0; n < N; ++n)
	{
		delete [] prev_rate[n];
		delete [] curr_rate[n];
		delete [] p_in[n];
		delete [] p_hit[n];
	}

	delete [] prev_rate;
	delete [] curr_rate;
	delete [] p_in;
	delete [] p_hit;

}



double statistics::calculate_phit_neigh_scalable(int node_ID, int cont_ID, float **ratePrev, float **Pin, float **Phit, double *tcVect, double alphaExp, double lambdaVal, long catCard, bool* clientVect, vector<map<int,int> > &neighMat)
{
    double num;
    double lambda_ex_cont_ID;       // Exogenouas rate for content ID.

    double p_hit_tot = 0.0;
    double probNorm = 0.0;

    if (ratePrev[node_ID][cont_ID] != 0)    // Only if there is incoming traffic the hit probability can be greater than 0.
    {
		if(clientVect[node_ID])
		{
			num = (double)(1.0/(double)pow(cont_ID+1,alphaExp));
			lambda_ex_cont_ID = (double)(num*content_distribution::zipf[0]->get_normalization_constant())*(lambdaVal);
			probNorm += lambda_ex_cont_ID;      // It will be added to the sum of the incoming miss streams.
		}

		if (neighMat[node_ID].size() > 0)
		{
			int neigh;

			//for(uint32_t i=0; i < neighMat[node_ID][cont_ID].size(); i++)
			for (std::map<int,int>::iterator it = neighMat[node_ID].begin(); it!=neighMat[node_ID].end(); ++it)
			{
				neigh = it->first;
				if(ratePrev[neigh][cont_ID]!=0)
					probNorm += (ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))*(1./it->second); // we take into account ri,j
			}

			if(probNorm == 0)
			{
				p_hit_tot = 0;
				return p_hit_tot;
			}

			//cout << "PROB NORM: " << probNorm << endl;


			//for(uint32_t i=0; i < neighMat[node_ID][cont_ID].size(); i++)
			for (std::map<int,int>::iterator it = neighMat[node_ID].begin(); it!=neighMat[node_ID].end(); ++it)
			{
				neigh = it->first;
				if (ratePrev[neigh][cont_ID]!=0)
				{
					double Aij  = 0;
					double partial_sum = 0;
					//for (uint32_t j=0; j < neighMat[node_ID][cont_ID].size(); j++)  // If there are at least 2 neighbors (different from a client)
					for (std::map<int,int>::iterator it2 = neighMat[node_ID].begin(); it2!=neighMat[node_ID].end(); ++it2)
					{
						if (it2!=it)	// The 'j' neigh must be different than the one selected to calculate the conditional prob.
						{
							neigh = it2->first;
							if(ratePrev[neigh][cont_ID]!=0)
								partial_sum += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*tcVect[node_ID])*(1./it2->second);
								//partial_sum += (ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])* max((double)0, tcVect[node_ID]-tcVect[neigh]);
						}
					}
					if (clientVect[node_ID])
					{
						partial_sum += (lambda_ex_cont_ID)*tcVect[node_ID];
					}

					neigh = it->first;

					if(meta_cache == LCE)
					{
						Aij += (((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID]) * max((double)0, tcVect[node_ID]-tcVect[neigh]))*(1./it->second) + partial_sum);
						// *** a-NET style ***
						//Aij += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID]) * tcVect[node_ID] + partial_sum);

						// Sum conditional probabilities.
						//if(Aij < 0.01)
						//	p_hit_tot += (-Aij)*((ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))*(1./it->second)/probNorm);
						//else
							p_hit_tot += (1 - exp(-Aij))*((ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))*(1./it->second)/probNorm);
					}
					else if(meta_cache == fixP)
					{
						// "partial_sum" only in the exponent with Tc2-Tc1
						Aij += (1-q)*(1-exp(-ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh])) +
								exp(-ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh]) *
								(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID]) * max(double(0), tcVect[node_ID] - tcVect[neigh]) + partial_sum)));
						// "partial_sum" in each exponent.
						//Aij += (1-q)*(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh] + partial_sum))) +
						//		exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID])*tcVect[neigh] + partial_sum)) *
						//		(1-exp(-(ratePrev[neigh][cont_ID]*(1-Pin[neigh][cont_ID]) * max(double(0), tcVect[node_ID] - tcVect[neigh]) + partial_sum)));


						p_hit_tot += ((q * Aij) / (1 - (1-q)*Aij))*((ratePrev[neigh][cont_ID]*(1-Phit[neigh][cont_ID]))/probNorm);
					}
					else
					{
						cout << "Meta Caching Algorithm NOT Implemented!" << endl;
						exit(0);
					}
				}
			}
		}
		if (clientVect[node_ID])  // we have to calculate the conditional probability with respect to the client
		{
			double Aij = 0;
			double partial_sum = 0;
			int neigh;
			//for (uint32_t j=0; j < neighMat[node_ID][cont_ID].size(); j++)
			if (neighMat[node_ID].size() > 0)
			{
				for (std::map<int,int>::iterator it = neighMat[node_ID].begin(); it!=neighMat[node_ID].end(); ++it)
				{
					neigh = it->first;

					if(ratePrev[neigh][cont_ID]!=0)
						partial_sum += ((ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*tcVect[node_ID])*(1./it->second);
						//partial_sum += (ratePrev[neigh][cont_ID])*(1-Pin[neigh][cont_ID])*max((double)0, tcVect[node_ID]-tcVect[neigh]);
				}
			}

			if(meta_cache == LCE)
			{
				Aij += ((lambda_ex_cont_ID)*tcVect[node_ID] + partial_sum);
				//if(Aij < 0.01)
				//	p_hit_tot += (Aij)*(lambda_ex_cont_ID/probNorm);
				//else
					p_hit_tot += (1 - exp(-Aij))*(lambda_ex_cont_ID/probNorm);
			}
			else if (meta_cache == fixP)
			{
				Aij += q * (1 - exp(-(lambda_ex_cont_ID*tcVect[node_ID] + partial_sum)));
				p_hit_tot += (Aij / (exp(-(lambda_ex_cont_ID*tcVect[node_ID] + partial_sum)) + Aij)) * (lambda_ex_cont_ID/probNorm);
			}
			else
			{
				cout << "Meta Caching Algorithm NOT Implemented!" << endl;
				exit(0);
			}
		}
    }

	return p_hit_tot;
}


// *** APPROX NRR ***


void statistics::cacheFillModel_Scalable_Approx_NRR(const char* phase)
{
	chrono::high_resolution_clock::time_point tStartAfterFailure;
	chrono::high_resolution_clock::time_point tEndAfterFailure;
	if(strcmp(phase,"init")!=0)
		tStartAfterFailure = chrono::high_resolution_clock::now();
	long M = (long)content_distribution::zipf[0]->get_catalog_card();		// Catalog cardinality.
	int N = num_nodes;												// Number of nodes.
	double Lambda = clients[0]->getLambda();						// Aggregate lambda of exogenous requests.
	uint32_t cSize_targ = (double)caches[0]->get_size();			// Target Cache size.

	cout << "Model CACHE SIZE = " << cSize_targ << endl;

	double alphaVal = content_distribution::zipf[0]->get_alpha();
	double normConstant = content_distribution::zipf[0]->get_normalization_constant();  // Zipf's normalization constant.

	string forwStr = caches[0]->getParentModule()->par("FS");
	cout << "*** Model Forwarding Strategy : " << forwStr << " ***" << endl;

	// Retrieve meta-caching algorithm
	DecisionPolicy* decisor = caches[0]->get_decisor();
	Always* dp1 = dynamic_cast<Always*> (decisor);
	if(dp1)
	{
		meta_cache = LCE;
		dpString = "LCE";
		cout << "*** LCE *** Decision Policy!" << endl;
	}
	else
	{
		Fix* dp2 = dynamic_cast<Fix*> (decisor);
		if(dp2)
		{
			meta_cache = fixP;
			q = dp2->p;
			dpString = "fixP";
			cout << "*** fixP *** Decision Policy with q = " << q << endl;
		}
		else
		{
			cout << "Decision policy not supported!" << endl;
			exit(0);
		}
	}

	cout << "***** CACHE FILLING WITH MODEL *****" << endl;

	// Definition and initialization of useful data structures.
	float **prev_rate;		// Request rate for each content at each node at the 'Previous Step'.
	float **curr_rate;		// Request rate for each content at each node at the 'Current Step'.
	float **p_in;			// Pin probability for each content at each node.
	float **p_hit;			// Phit probability for each content at each node.

	// All the previous structures will be matrices of size [M][N].
	prev_rate = new float*[N];
	curr_rate = new float*[N];
	p_in = new float*[N];
	p_hit = new float*[N];

	for (int i=0; i < N; i++)
	{
		prev_rate[i] = new float[M];
		curr_rate[i] = new float[M];
		p_in[i] = new float[M];
		p_hit[i] = new float[M];
	}

	double *tc_vect = new double[N]; 		// Vector containing the 'characteristic times' of each node.


	float **p_in_temp;				// Temp vector to find the max p_in per each node at the end of each iteration.
	p_in_temp = new float*[N];
	for(int n=0; n < N; n++)
			p_in_temp[n] = new float[M];

	double *pHitNode = new double[N];
	fill_n(pHitNode,N,0.0);
	double prev_pHitTot = 0.0;
	double curr_pHitTot = 0.0;


	double *sumCurrRate = new double[N];	// It will contain the total incoming rate for each node.
	fill_n(sumCurrRate,N,0.0);

	//vector<int> inactiveNodes;
	vector<int> activeNodes;

	// *** DISABLED FOR PERF EVALUATION ***
	int **steadyCache; 			// Matrix containing the IDs of the content the will be cached inside the active nodes for the hot start.
	steadyCache = new int*[N];
	for (int n=0; n < N; n++)
	{
		steadyCache[n] = new int[cSize_targ];
		fill_n(steadyCache[n], cSize_targ, M+1);
	}

	// *** INITIALIZATION ***
	// At step '0', the incoming rates at each node are supposed to be the same,
	// i.e., prev_rate(i,n) = zipf(i) * Lambda.
	// As a consequence, the characteristic times of all the nodes will be initially the same, and so the Pin.

	int step = 0;
	double num = 0;

	cout << "Iteration # " << step << " - INITIALIZATION" << endl;

	for (long m=0; m < M; m++)
	{
		num = (1.0/pow(m+1,alphaVal));
		prev_rate[0][m] = (float)(num*normConstant)*Lambda;
		sumCurrRate[0] += prev_rate[0][m];
	}

	for (int n=1; n < N; n++)
	{
		std::copy(&prev_rate[0][0], &prev_rate[0][M], prev_rate[n]);
		sumCurrRate[n] = sumCurrRate[0];
	}


	// The Tc will be initially the same for all the nodes, so we pass just the first column of the prev_rate.
	// In the following steps it will be Tc_val(n) = compute_Tc(...,prev_rate, n-1).

	double tc_val = compute_Tc_single_Approx(cSize_targ, alphaVal, M, prev_rate, 0, dpString, q);

	cout << "Computed Tc during initialization:\t" << tc_val << endl;

	if(meta_cache == LCE)
	{
		for (int n=0; n < N; n++)
		{
			tc_vect[n] = tc_val;
			for (long m=0; m < M; m++)
			{
				p_in[n][m] = 1 - exp(-prev_rate[n][m]*tc_vect[n]);
				p_hit[n][m] = p_in[n][m];
				pHitNode[n] += (prev_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}
			prev_pHitTot += pHitNode[n];
		}
	}

	else if (meta_cache == fixP)
	{
		for (int n=0; n < N; n++)
		{
			tc_vect[n] = tc_val;
			for (long m=0; m < M; m++)
			{
				p_in[n][m] = (q * (1.0 - exp(-prev_rate[n][m]*tc_vect[n])))/(exp(-prev_rate[n][m]*tc_vect[n]) + q * (1.0 - exp(-prev_rate[n][m]*tc_vect[n])));
				p_hit[n][m] = p_in[n][m];
				pHitNode[n] += (prev_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}
			prev_pHitTot += pHitNode[n];
		}
	}
	else
	{
		cout << "Meta Caching Algorithm NOT Implemented!" << endl;
		exit(0);
	}

	prev_pHitTot /= N;
	cout << "pHit Tot Init - " << prev_pHitTot << endl;


	// *** ITERATIVE PROCEDURE ***

	// Cycle over more steps
	int slots = 30;

	//double **MSD; 			// matrix[slots][N]; it will contain computed Mean Square Distance at each step of the iteration.
							// Useful as a stop criterion for the iterative procedure.

	vector<vector<map<int,int> > >  neighMatrix;
	neighMatrix.resize(N);
	for(int n=0; n < N; n++)
		neighMatrix[n].resize(M);


	/*vector<vector<map<int,double> > > rateNeighMatrix;
	rateNeighMatrix.resize(N);
	for(int n=0; n < N; n++)
		rateNeighMatrix[n].resize(M);
	 */

	bool* clientVector = new bool[N];					// Vector indicating if the current node has an attached client.
	for (int n=0; n < N; n++)
		clientVector[n] = false;

	//int* repoVector = new int[M];						// Vector indicating the repoID for each content (e.g., repoVect[2] = 30
														// means that content '2' is stored in Repo '30').
														// To do: transform in vector<> in case of multiple seed copies.

	vector<vector<int> > repoVector;
	repoVector.resize(M);



    int l;

    for (long m=0; m < M; m++)		// CONTENTS
    {
    	// Retrieve the Repository storing content 'm'

    	repo_t repo = __repo(m+1);    // The repo is the one associated to the position of the '1' inside the string
    	l = 0;
    	while (repo)
    	{
    		if (repo & 1)
			{
 				//repoVector[m] = content_distribution::repositories[l];  // So far, the replication of seed copies is supposed
    																    // to be 1 (i.e., the seed copy of each content is stored only in one repo)
    			repoVector[m].push_back(content_distribution::repositories[l]);	// In case of more repos stories the seed copies of a content.
 				//if (m<20)
 				//	cout << "The Repo of content # " << m << " is: " << repoVector[m] << endl;
    		}
    		repo >>= 1;
    		l++;
    	}
    }

	int outInt;
	int target;
	int numPotTargets;

	std::vector<int>::iterator itRepoVect;

	for (int n=0; n < N; n++)  // NODES
	{
	    for(int i=0; i < num_clients; i++)
	    {
	    	if(clients_id[i] == n)
	    	{
	    		clientVector[n] = true;
	    		break;
	    	}
	    }

		for (long m=0; m < M; m++)
		{
			//if(n != repoVector[m])  // For the node owning the repo of content 'm' there is no potential target
			//for(itRepoVect = repoVector[m].begin(); itRepoVect != repoVector[m].end(); ++itRepoVect )
			itRepoVect = find (repoVector[m].begin(), repoVector[m].end(), n);
			if (itRepoVect != repoVector[m].end() )
				continue;
			else
			{
				for(itRepoVect = repoVector[m].begin(); itRepoVect != repoVector[m].end(); ++itRepoVect)
				{
					//outInt = cores[n]->getOutInt(repoVector[m]);
					outInt = cores[n]->getOutInt(*itRepoVect);
					target = caches[n]->getParentModule()->gate("face$o",outInt)->getNextGate()->getOwnerModule()->getIndex();
					neighMatrix[target][m].insert( pair<int,int>(n,1));
				}
			}
		}
	}


	// Iterations
	for (int k=0; k < slots; k++)
	{
		step++;
		cout << "Iteration # " << step << endl;

		// *** With NRR, we need to renew the neighMatrix at each step
		if (step > 1 && forwStr.compare("nrr") == 0)
		{
			// Reset the old neighMatrix
			for(int n=0; n < N; n++)
			{
				for (int m=0; m < M; m++)
				{
					neighMatrix[n][m].clear();
				}
			}

			for (int n=0; n < N; n++)  // NODES
			{
				if (tc_vect[n] != numeric_limits<double>::max())    // Only active nodes can have potential targets.
				{
					//if (n==11 || n==3)
					//	caches[n]->dump();
					strategy_layer* strategy_ptr = cores[n]->get_strategy();
					int nOutInt = strategy_ptr->__get_outer_interfaces();
					bool *outInterfaces = new bool [nOutInt];
					fill_n(outInterfaces, nOutInt, false);

					for (int m=0; m < M; m++)
					{
						//if(n != repoVector[m]) // For the node owing the repo of content 'm' there is no potential target.
						itRepoVect = find (repoVector[m].begin(), repoVector[m].end(), n);
						if (itRepoVect != repoVector[m].end() )
							continue;
						//if (itRepoVect != repoVector[m].end() && *itRepoVect != n)
						else
						{
							// Reset the vector representing the out interfaces
							fill_n(outInterfaces, nOutInt, false);
							numPotTargets = 0;

							outInterfaces = strategy_ptr->exploit_model(m);  // Lookup inside network caches

							for (int p=0; p < nOutInt; p++)			// Count the number of potential targets.
							{
								if(outInterfaces[p])
									numPotTargets++;
							}


							for (int p=0; p < nOutInt; p++)  	// Fill the neighMatrix for the selected target.
							{
								if(outInterfaces[p])
								{
									target = caches[n]->getParentModule()->gate("face$o",p)->getNextGate()->getOwnerModule()->getIndex();
									neighMatrix[target][m].insert( pair<int,int>(n,numPotTargets));
								}
							}
						}
					}
				}
			}

			/*for (int i=0; i<=2; i++)
			{
				cout << "# Neigh for Content # " << i << " : " << neighMatrix[3][i].size() << endl;
				for (std::map<int,int>::iterator it = neighMatrix[3][i].begin(); it!=neighMatrix[3][i].end(); ++it)
				{
					int neigh = it->first;
					cout << "Neigh = " << neigh << endl;
				}
			}*/

		}

		// Calculate the 'current' request rate for each content at each node.
		for (int n=0; n < N; n++)			// NODES
		{
			double sum_curr_rate = 0;
			double sum_prev_rate = 0;

			//cout << "NODE # " << n << endl;
			for (long m=0; m < M; m++)		// CONTENTS
			{
				double neigh_rate = 0;			// Cumulative miss rate from neighbors for the considered content.

			    if (neighMatrix[n][m].size() > 0 || clientVector[n])	// Sum the miss streams of the neighbors for the considered content.
			    {
			    	//for (uint32_t i=0; i < neighMatrix[n][m].size(); i++)
			    	for (std::map<int,int>::iterator it = neighMatrix[n][m].begin(); it!=neighMatrix[n][m].end(); ++it)
			    	{

			    		// The miss stream of the selected neighbor will depend on its hit probability for the selected
			    		// content (i.e., Phit(neigh,m). This can be calculated using the conditional probabilities
			    		// Phit(neigh,m|j) = 1 - exp(-A_neigh_j). It expresses the probability that a request for 'm'
			    		// hits cache 'neigh', provided that it comes from cache 'j'. Only neighboring caches for which
			    		// the neigh represents the next hop will be considered.
			    		//int neigh = neighMatrix[n][m][i];
			    		int neigh = it->first;
			    		int numPot = it->second;    // number of potential targets for the neighbor

			    		/*if (step == 1 && n == 4 )
			    		{
			    			cout << "NODE # " << n << ", Content # " << m << " Neigh # " << neigh << " Num Pot Targ # " << numPot << endl;
			    		}*/
			    		if(meta_cache == LCE)
			    		{
			    			if(prev_rate[neigh][m]*tc_vect[neigh] <= 0.01)
			    				p_hit[neigh][m] = prev_rate[neigh][m]*tc_vect[neigh];
			    			else
			    				p_hit[neigh][m] = calculate_phit_neigh(neigh, m, prev_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
			    		}
			    		else if(meta_cache == fixP)
			    		{
			    			if(q*prev_rate[neigh][m]*tc_vect[neigh] <= 0.01)
			    				p_hit[neigh][m] = q*(prev_rate[neigh][m]*tc_vect[neigh]);
			    			else
			    				p_hit[neigh][m] = calculate_phit_neigh(neigh, m, prev_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
			    		}
			    		else
						{
							cout << "Meta Caching Algorithm NOT Implemented!" << endl;
							exit(0);
						}


						if (p_hit[neigh][m] > 1)
							cout << "Node: " << n << "\tContent: " << m << "\tNeigh: " << neigh << "\tP_hit: " << p_hit[neigh][m] << endl;

						//neigh_rate += (prev_rate[neigh][m]*(1-p_hit[neigh][m]));
						neigh_rate += (prev_rate[neigh][m]*(1-p_hit[neigh][m]))*(1./numPot);

						//rateNeighMatrix[n][m][neigh] = 0; 		// Reset the result of the previous iteration.
						//rateNeighMatrix[n][m][neigh] += (prev_rate[neigh][m]*(1-p_hit[neigh][m]));
			    	}
			    }


			    if (clientVector[n])	// In case a client is attached to the current node.
		    	{
					num = (1.0/(double)pow(m+1,alphaVal));
					neigh_rate += (num*normConstant)*Lambda;
		    	}

			    curr_rate[n][m] = neigh_rate;

			    sum_curr_rate += curr_rate[n][m];
			    sum_prev_rate += prev_rate[n][m];

			    prev_rate[n][m] = curr_rate[n][m];

			}  // contents

			//cout << "Iteration # " << step << " Node # " << n << " Incoming Rate: " << sum_curr_rate << endl;

			sumCurrRate[n] = sum_curr_rate;

			// Calculate the new Tc and Pin for the current node.
			if (sum_curr_rate != 0) 	// The current node is hit either by exogenous traffic or by miss streams from
										// other nodes (or by both)
			{
				//cout << "NODE # " << n << " Sum Current Rate: " << sumCurrRate[n] << endl;
				tc_vect[n] = compute_Tc_single_Approx_More_Repo(cSize_targ, alphaVal, M, curr_rate, n, dpString, q);

				//cout << "Node # " << n << " - Tc " << tc_vect[n] << endl;
				if(meta_cache == LCE)
				{
					for(long m=0; m < M; m++)
					{
						if(curr_rate[n][m]*tc_vect[n] <= 0.01)
						{
							p_in[n][m] = curr_rate[n][m]*tc_vect[n];
						}
						else
						{
							p_in[n][m] = 1 - exp(-curr_rate[n][m]*tc_vect[n]);
						}
					}
				}
				else if(meta_cache == fixP)
				{
					for(long m=0; m < M; m++)
					{
						if(q*curr_rate[n][m]*tc_vect[n] <= 0.01 && q*curr_rate[n][m]*tc_vect[n] > 0.0)
						{
							p_in[n][m] = q * curr_rate[n][m]*tc_vect[n];
						}
						else if(q*curr_rate[n][m]*tc_vect[n] == 0.0)
						{
							p_in[n][m] = 0;
							p_hit[n][m] = 0;
						}
						else
						{
							p_in[n][m] = (q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])))/(exp(-curr_rate[n][m]*tc_vect[n]) + q * (1.0 - exp(-curr_rate[n][m]*tc_vect[n])));
						}
					}
				}
				else
				{
					cout << "Meta Caching Algorithm NOT Implemented!" << endl;
					exit(0);
				}

				// Calculate the Phit of the node hosting one of the repos (it can happen that it does not forward any
				// traffic, thus it is not a neighbor for any node, and so the logic of the p_hit calculus followed
				// before does not apply.
				if (find (content_distribution::repositories, content_distribution::repositories + num_repos, n)
							!= content_distribution::repositories + num_repos)
				{
					for(long m=0; m < M; m++)
					{
						if(meta_cache == LCE)
						{
							if(curr_rate[n][m]*tc_vect[n] <= 0.01)
								p_hit[n][m] = curr_rate[n][m]*tc_vect[n];
							else
								p_hit[n][m] = calculate_phit_neigh(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else if(meta_cache == fixP)
						{
							if(q*curr_rate[n][m]*tc_vect[n] <= 0.01 && q*curr_rate[n][m]*tc_vect[n] > 0.0)
								p_hit[n][m] = q*(curr_rate[n][m]*tc_vect[n]);
							else if(q*curr_rate[n][m]*tc_vect[n] == 0.0)
								p_hit[n][m] = 0;
							else
								p_hit[n][m] = calculate_phit_neigh(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else
						{
							cout << "Meta Caching Algorithm NOT Implemented!" << endl;
							exit(0);
						}
					}
				}
				// Do the same thing for the temporary repos
				for(long m=0; m < M; m++)
				{
					itRepoVect = find (repoVector[m].begin(), repoVector[m].end(), n);
					if (itRepoVect != repoVector[m].end() )
					{
						if(meta_cache == LCE)
						{
							if(curr_rate[n][m]*tc_vect[n] <= 0.01)
								p_hit[n][m] = curr_rate[n][m]*tc_vect[n];
							else
								p_hit[n][m] = calculate_phit_neigh(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else if(meta_cache == fixP)
						{
							if(q*curr_rate[n][m]*tc_vect[n] <= 0.01 && q*curr_rate[n][m]*tc_vect[n] > 0.0)
								p_hit[n][m] = q*(curr_rate[n][m]*tc_vect[n]);
							else if(q*curr_rate[n][m]*tc_vect[n] == 0.0)
							{
								p_hit[n][m] = 0;
								p_in[n][m] = 0;
							}
							else
								p_hit[n][m] = calculate_phit_neigh(n, m, curr_rate, p_in, p_hit, tc_vect, alphaVal, Lambda, M, clientVector, neighMatrix);
						}
						else
						{
							cout << "Meta Caching Algorithm NOT Implemented!" << endl;
							exit(0);
						}
					}
				}

			}
			else		// It means that the current node will not be hit by any traffic.
						// So we zero the miss stream coming from this node by putting p_in=1 for each content.
			{
				tc_vect[n] = numeric_limits<double>::max();   // Like infinite value;
				//cout << "Iteration # " << step << " NODE # " << n << " Tc - " << tc_vect[n] << endl;
				for(long m=0; m < M; m++)
				{
					p_in[n][m] = 0;
					p_hit[n][m] = 0;
				}

			}

		} // nodes

		curr_pHitTot = 0;
		for(int n=0; n < N; n++)
		{
			pHitNode[n] = 0;
			if(sumCurrRate[n]!=0)
			{
				for (long m=0; m < M; m++)
				{
					pHitNode[n] += (curr_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
				}
				curr_pHitTot += pHitNode[n];
			}

		}

		// *** EXIT CONDITION ***
		if( (abs(curr_pHitTot - prev_pHitTot)/curr_pHitTot) < 0.005 )
			break;
		else		// For the NRR implementation, the actual cache fill is moved here in order to update the neighMatrix
					// computation at the next step
		{
			prev_pHitTot = curr_pHitTot;

			/// ***** TRY *****
			for(int n=0; n < N; n++)
			{
				if(sumCurrRate[n]!=0)
				{
					for (long m=0; m < M; m++)
					{
						curr_rate[n][m] = 0;
						// Clear and re-fill the repo vector with nodes storing seed copies
						repoVector[m].clear();
				    	repo_t repo = __repo(m+1);    // The repo is the one associated to the position of the '1' inside the string
				    	l = 0;
				    	while (repo)
				    	{
				    		if (repo & 1)
							{
				 				//repoVector[m] = content_distribution::repositories[l];  // So far, the replication of seed copies is supposed
				    																    // to be 1 (i.e., the seed copy of each content is stored only in one repo)
				    			repoVector[m].push_back(content_distribution::repositories[l]);	// In case of more repos stories the seed copies of a content.
				 				//if (m<20)
				 				//	cout << "The Repo of content # " << m << " is: " << repoVector[m] << endl;
				    		}
				    		repo >>= 1;
				    		l++;
				    	}

					}
				}
			}


			// *** NRR
			// **** DISABLED ****
			if(forwStr.compare("nrr") == 0)
			{
				int maxPin;

				activeNodes.clear();

				for(int n=0; n < N; n++)
				{
					// Allocate contents only for nodes interested by traffic (it depends on the frw strategy)
					if(tc_vect[n] != numeric_limits<double>::max())
					{
						// **** NBB *** manca l'incremento degli active nodes!!!!
						activeNodes.push_back(n);

						// *** DISABLED for perf measurements
						// *** DETERMINING CONTENTS TO BE PUT INSIDE CACHES***
						p_in_temp[n] = p_in[n];
						for(uint32_t k=0; k < cSize_targ; k++)
						{
							maxPin = distance(p_in_temp[n], max_element(p_in_temp[n], p_in_temp[n] + M));  // Position of the highest popular object (i.e., its ID).
							if(p_in_temp[n][maxPin] == 0)			// There are few contents than cSize_targ that can be inside the cache
								break;
							steadyCache[n][k] = maxPin;

							// Adding temporary cached copies to the repo vector
							repoVector[maxPin].push_back(n);

							//p_in_temp[n][k] = p_in[n][maxPin];
							//p_hit_temp[n][k] = p_hit[n][maxPin];
							p_in_temp[n][maxPin] = 0;
						}
					}

					caches[n]->flush();
				}

				// *** REAL CACHE FILLING ***
				if(strcmp(phase,"init")==0)
				{
					cout << "Filling Caches with Model results...\n" << endl;
					int node_id;
					uint32_t cont_id;

					// In case of FIX probabilistic with need to temporarly change the caching prob id order to fill the cache
					DecisionPolicy* decisor;
					Fix* dp1;

					ccn_data* data = new ccn_data("data",CCN_D);

					for(unsigned int n=0; n < activeNodes.size(); n++)
					{
						node_id = activeNodes[n];

						decisor = caches[node_id]->get_decisor();
						dp1 = dynamic_cast<Fix*> (decisor);;

						if(dp1)
						{
							dp1->setProb(1.0);
						}

						// Empty the cache from the previous step
						caches[node_id]->flush();

						for (unsigned int k=0; k < cSize_targ; k++)
						{
							cont_id = (uint32_t)steadyCache[node_id][k] + 1;
							if (cont_id == M+2)			// There are few contents than cSize_targ that can be inside the cache
								break;
							chunk_t chunk = 0;
							__sid(chunk, cont_id);
							__schunk(chunk, 0);
							//ccn_data* data = new ccn_data("data",CCN_D);
							data -> setChunk (chunk);
							data -> setHops(0);
							data->setTimestamp(simTime());
							caches[node_id]->store(data);

							// Adding the location of the new cached content as a repo for that content
			    			// in order to avoid considering outgoing flows from that node and for that
							// particular content.
						}
						//cout << "Cache Node # " << node_id << " Step # " << step << " :\n";
						//caches[node_id]->dump();

						if(dp1)
						{
							dp1->setProb(q);
						}


						// Reset the steadyCache vector
						fill_n(steadyCache[n], cSize_targ, M+1);
					}

					delete data;
				}
			}
		}
	}  // Successive step

	int maxPin;

	double pHitTotMean = 0;
	double pHitNodeMean = 0;

	activeNodes.clear();

	for(int n=0; n < N; n++)
	{
		// Allocate contents only for nodes interested by traffic (it depends on the frw strategy)
		if(tc_vect[n] != numeric_limits<double>::max())
		{
			activeNodes.push_back(n);

			// Calculate di p_hit mean of the node
			for(uint32_t m=0; m < M; m++)
			{
				pHitNodeMean += (curr_rate[n][m]/sumCurrRate[n])*p_hit[n][m];
			}

			// *** DISABLED for perf measurements
			if(!onlyModel)
			{
			// *** DETERMINING CONTENTS TO BE PUT INSIDE CACHES***
			// Choose the contents to be inserted into the cache.
				for(uint32_t k=0; k < cSize_targ; k++)
				{
					maxPin = distance(p_in[n], max_element(p_in[n], p_in[n] + M));  // Position of the highest popular object (i.e., its ID).
					// *** NRR
					if(p_in[n][maxPin] == 0.0)			// There are few contents than cSize_targ that can be inside the cache
						break;
					steadyCache[n][k] = maxPin;

					//cout << "NODE # " << n << " Content # " << maxPin << endl;

					//p_in_temp[n][k] = p_in[n][maxPin];
					//p_hit_temp[n][k] = p_hit[n][maxPin];
					p_in[n][maxPin] = 0;
				}
			}


			pHitTotMean += pHitNodeMean;
			cout << "NODE # " << n << endl;
			cout << "-> Mean Phit: " << pHitNodeMean << endl;
//			cout << "-> Steady Cache: Content - Pin - Phit\n";
//			for (uint32_t k=0; k < cSize_targ; k++)
//				cout << steadyCache[n][k] << " - " << p_in_temp[n][k] << " - " << p_hit_temp[n][k] << endl;
			cout << endl;
			pHitNodeMean = 0;
		}
	}

	if(strcmp(phase,"init")==0)
		cout << "Number of Active Nodes 1: " << activeNodes.size() << endl;
	else
		cout << "Number of Active Nodes 2: " << num_nodes - activeNodes.size() << " Time: " << simTime() << endl;

	pHitTotMean = pHitTotMean/(activeNodes.size());
	//pHitTotMean = pHitTotMean/(N);

	if(strcmp(phase,"init")==0)
		cout << "MODEL - P_HIT MEAN TOTAL AFTER CACHE FILL: " << pHitTotMean << endl;
	else
		cout << "MODEL - P_HIT MEAN TOTAL AFTER ROUTE re-CALCULATION: " << pHitTotMean << endl;

	cout << "*** Tc of nodes ***\n";
	for(int n=0; n < N; n++)
	{
		cout << "Tc-" << n << " --> " << tc_vect[n] << endl;
	}

    // *** DISABLED per perf evaluation
	if(!onlyModel)
	{
		// *** REAL CACHE FILLING ***
		if(strcmp(phase,"init")==0)
		{
			cout << "Filling Caches with Model results...\n" << endl;
			int node_id;
			uint32_t cont_id;

			// In case of FIX probabilistic with need to temporarly change the caching prob id order to fill the cache
			DecisionPolicy* decisor;
			Fix* dp1;

			ccn_data* data = new ccn_data("data",CCN_D);

			for(unsigned int n=0; n < activeNodes.size(); n++)
			{
				node_id = activeNodes[n];

				decisor = caches[node_id]->get_decisor();
				dp1 = dynamic_cast<Fix*> (decisor);;

				if(dp1)
				{
					dp1->setProb(1.0);
				}

				// Empty the cache from the previous step.
				caches[node_id]->flush();

				//for (int k=cSize_targ-1; k >= 0; k--)   // Start inserting contents with the smallest p_in.
				for (unsigned int k=0; k < cSize_targ; k++)
				{
					cont_id = (uint32_t)steadyCache[node_id][k]+1;
					if (cont_id == M+2)			// There are few contents than cSize_targ that can be inside the cache
						break;
					chunk_t chunk = 0;
					__sid(chunk, cont_id);
					__schunk(chunk, 0);
					//ccn_data* data = new ccn_data("data",CCN_D);
					data -> setChunk (chunk);
					data -> setHops(0);
					data->setTimestamp(simTime());
					caches[node_id]->store(data);
				}
				//cout << "Cache Node # " << node_id << " :\n";
				//caches[node_id]->dump();

				if(dp1)
				{
					dp1->setProb(q);
				}

			}

			delete data;
		}
	}



	// DISABLED for perf eval
	// *** INCOMING RATE CALCULATION rate for each node on each link
	/*double linkTraffic = 0;
	int neigh;
	vector<map<int,double> > totalNeighRate;
	totalNeighRate.resize(N);
	cout << "*** Link State ***" << endl;
	//cout << "Extreme A -- Link Traffic -- Extreme B" << endl;
	cout << "Extreme A -- Link Load [Bw=1Mbps] -- Extreme B" << endl;
	cout << endl;
	for (int n=0; n < N; n++)
	{
		for (int m=0; m < M; m++)
		{
			map<int,double>::iterator it = rateNeighMatrix[n][m].begin();
			while(it!=rateNeighMatrix[n][m].end())
			{
				neigh = it->first;
				map<int,double>::iterator itInner = totalNeighRate[n].find(neigh);
				if(itInner == totalNeighRate[n].end()) // Insert for the first time
					totalNeighRate[n][neigh] = it->second;
				else										// Neigh already inserted before.
					totalNeighRate[n][neigh] += it->second;
				++it;
			}
		}
	}

	for (int n=0; n < N; n++)
	{
		for(map<int,double>::iterator it=totalNeighRate[n].begin(); it!=totalNeighRate[n].end(); ++it)
		{
			neigh = it->first;
			linkTraffic += it->second;
			map<int,double>::iterator itInner = totalNeighRate[neigh].find(n);  // Sum the traffic in the opposite direction
			if(itInner != totalNeighRate[neigh].end())  // There is also traffic in the opposite direction.
			{
				linkTraffic += itInner->second;
				totalNeighRate[neigh].erase(itInner);
			}
			//cout << "Node " << n << "\t- " << linkTraffic << " [Interest/s] -\tNode " << neigh << endl;
			cout << "Node " << n << "\t- " << (double)((linkTraffic*(1536*8))/1000000) << " -\tNode " << neigh << endl;
			linkTraffic = 0;

			/* Take the DataRate from each Link; To use with cDatarateChannel and not with the current cDelayChannel
			cDelayChannel *chPointF;
			for (int i = 0; i<caches[n]->getParentModule()->gateSize("face$o");i++)
			{
				if (!caches[n]->__check_client(i))
				{
					int index = caches[n]->getParentModule()->gate("face$o",i)->getNextGate()->getOwnerModule()->getIndex();
					if(index==neigh)
					{
						chPointF =  dynamic_cast<cDelayChannel*> (caches[n]->getParentModule()->gate("face$o",i)->getChannel());
						if(chPointF)
							cout << "Node # " <<  n << " Datarate link at interface " << i << " : " << chPointF->getNominalDatarate() << endl;

					}
				}
			}*/
//		}
//	}

	if(strcmp(phase,"init")!=0)
	{
		tEndAfterFailure = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>( tEndAfterFailure - tStartAfterFailure  ).count();
		cout << "Execution time of the model after failure [ms]: " << duration << endl;
	}
	// De-allocating memory
	for (int n = 0; n < N; ++n)
	{
		delete [] prev_rate[n];
		delete [] curr_rate[n];
		delete [] p_in[n];
		delete [] p_hit[n];
	}

	delete [] prev_rate;
	delete [] curr_rate;
	delete [] p_in;
	delete [] p_hit;

}
