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
#include "strategy_layer.h"
#include <sstream>
#include "error_handling.h"
#include "content_distribution.h"
#include "statistics.h"
ifstream strategy_layer::fdist;
ifstream strategy_layer::frouting;


using namespace std;


void strategy_layer::initialize()
{
	//cout << "STRATEGY LAYER" << endl;
	/*cChannel *chPointF;
	for (int i = 0; i<getParentModule()->gateSize("face$o");i++)
	{
		if (!__check_client(i))
		{
			chPointF =  getParentModule()->gate("face$o",i)->getChannel();
			if(chPointF)
				cout << "Node # " <<  getParentModule()->getIndex() << " Datarate link at interface " << i << " : " << chPointF->getNominalDatarate() << endl;
		}
	}*/
	// *** Messages for link failure/recovery and route re-computation ***
	failure = new cMessage("failure", FAILURE);
	recovery = new cMessage("recovery", RECOVERY);
	new_routes = new cMessage("new_routes", NEW_ROUTES);
	new_model = new cMessage("new_model", NEW_MODEL);
	// ***

	fail_time = par("fail_time");
	fail_duration = par("fail_duration");
	fail_transient = par("fail_transient");

	fail_scenario = par("fail_scenario");

	for (int i = 0; i<getParentModule()->gateSize("face$o");i++)	// Cycle over all the interfaces.
    {
    	int index ;
    	if (!__check_client(i))
    	{
    		// If the module attached to the i-th interface is NOT a client, get its ID (should be the neighbor ID).
    		index = getParentModule()->gate("face$o",i)->getNextGate()->getOwnerModule()->getIndex();
    		gatelu[index] = i;		// Associate the neighbor ID to the local interface ID to reach it.

    		if(fail_scenario)
    		{
				// If a repo is attached to 'index', disable the link between the current node and 'index'
				for (int k=0; k < content_distribution::num_repos; k++)
				{
					//if(content_distribution::repositories[k]==index && getParentModule()->getIndex()!=34)  // for grid100_1
					if(content_distribution::repositories[k]==index && getParentModule()->getIndex()!=2)  // for tree_extra
					{
						failRecInt = i;
						scheduleAt(simTime() + fail_time, failure);
						break;
					}
				}
    		}
    	}
    }
    
    string fileradix = par("routing_file").stringValue();
    string filerout = fileradix+".rou";
    string filedist = fileradix+".dist";
    if (fileradix!= "")
    {
    	if (!fdist.is_open())
    	{
    		fdist.open(filedist.c_str());
    		frouting.open(filerout.c_str());
    	}
    	populate_from_file(); 	// Building forwarding table.
	}
    else
    	populate_routing_table(); // Building forwarding table.

	if(fail_scenario)
	{
		// Each node will recompute new routes after a transient period (after the failure).
		scheduleAt(simTime() + fail_time + fail_transient, new_routes);
		// Each node will recompute new routes after a transient period (after the recovery).
		//scheduleAt(simTime() + fail_time + fail_duration + fail_transient, new_routes);
	}

}

void strategy_layer::finish()
{
    fdist.close();
    frouting.close();
    delete failure;
    delete recovery;
    delete new_routes;
    delete new_model;
}

void strategy_layer::handleMessage(cMessage *in)
{
	int index;
	int oppositeGateIndex;
	cDelayChannel *chPointF;
	cDelayChannel *chPointR;
    switch (in->getKind()){
    case FAILURE:
    	index = getParentModule()->gate("face$o",failRecInt)->getNextGate()->getOwnerModule()->getIndex();
    	oppositeGateIndex = getParentModule()->gate("face$o",failRecInt)->getNextGate()->getIndex();
    	cout << simTime() << "\tNODE # " << getParentModule()->getIndex() << " is DISABLING the link towards node " << index << endl;
    	chPointF =  dynamic_cast<cDelayChannel *> (getParentModule()->gate("face$o",failRecInt)->getChannel());
    	chPointF->setDisabled(true);
    	chPointF =  dynamic_cast<cDelayChannel *> (getParentModule()->gate("face$o",failRecInt)->getNextGate()->getOwnerModule()->gate("face$o",oppositeGateIndex)->getChannel());
    	chPointF->setDisabled(true);
    	//Schedule the recovery
    	//scheduleAt(simTime() + fail_duration, recovery);
    	delete failure;
    	break;
    case RECOVERY:
    	index = getParentModule()->gate("face$o",failRecInt)->getNextGate()->getOwnerModule()->getIndex();
    	oppositeGateIndex = getParentModule()->gate("face$o",failRecInt)->getNextGate()->getIndex();
    	cout << simTime() << "\tNODE # " << getParentModule()->getIndex() << " is RECOVERING the link towards node " << index << endl;
    	chPointR =  dynamic_cast<cDelayChannel *> (getParentModule()->gate("face$o",failRecInt)->getChannel());
    	chPointR->setDisabled(false);
    	chPointR =  dynamic_cast<cDelayChannel *> (getParentModule()->gate("face$o",failRecInt)->getNextGate()->getOwnerModule()->gate("face$o",oppositeGateIndex)->getChannel());
    	chPointR->setDisabled(false);
    	break;
    case NEW_ROUTES:
    	cout << simTime() << "\tNODE # " << getParentModule()->getIndex() << " **** CALCULATING NEW ROUTES *****\n";

    	//chrono::high_resolution_clock::time_point tsNewRoute = chrono::high_resolution_clock::now();
    	FIB.clear();
    	populate_routing_table();
    	//chrono::high_resolution_clock::time_point teNewRoute = chrono::high_resolution_clock::now();
    	//auto dur = chrono::duration_cast<chrono::milliseconds>( teNewRoute - teNewRoute ).count();
    	//cout << "Execution Time New Routes Node # " << getParentModule()->getIndex() << " [ms]: " << dur << endl;

    	scheduleAt(simTime() + (double)(0.1), new_model);

    	delete new_routes;
    	break;
    case NEW_MODEL:
    	// Execute the module to calculate the new steady state probability
    	if(getParentModule()->getIndex() == 0)    // Schedule the model execution only from one node, and after that all
    											  // nodes have computed the new routes.
    	{

			cModule* pSubModule = getParentModule()->getParentModule()->getSubmodule("statistics");
			if (pSubModule)
			{
				statistics* pClass2Module = dynamic_cast<statistics*>(pSubModule);
				if(pClass2Module)
				{
					cout << "Entering in Model Computation after new routes..." << endl;
					//pClass2Module->cacheFillModel("newRoutes");
					pClass2Module->cacheFillModel_Scalable_Approx("newRoutes");
					// Calling the new stabilization check
					cout << "Time New Route: " << simTime() << endl;
					cout << "Entering in NEW STABILIZATION CHECK..." << endl;
					pClass2Module->checkStability();
				}
			}
    	}
    	delete new_model;
    	break;
    default:
    	cout << "*** Strategy Layer Node # " << getParentModule()->getIndex() << " : received a not identified message! ***\n";
    	exit(1);
    }
}


// Populate the host-centric routing table.
// That comes from a centralized process based on the ctopology class.
void strategy_layer::populate_routing_table()
{
    deque<int> p;
    cTopology topo;
    vector<string> types;

    // Extract topology map
    types.push_back("modules.node.node");		// We are interested in getting all nodes.
    topo.extractByNedTypeName( types );
    cTopology::Node *node = topo.getNode( getParentModule()->getIndex() ); 	// Taking the reference to the current node.

    // As the node topology is defined as a vector of nodes (see Omnet++ manual), cTopology
    // associates the node i with the node whose Index is i.

	// Find shortest paths from all nodes towards the target node 'dest'.
    for (int dest = 0; dest < topo.getNumNodes(); dest++)
    {
		if (dest != getParentModule()->getIndex())	// Skip ourself.
		{
			cTopology::Node *to   = topo.getNode( dest );  	// Get the target node.
			topo.weightedMultiShortestPathsTo( to );		// Results are stored inside the cTopology object.
			if (node->getNumPaths() == 0)					// The current node does not have any path to reach the target.
			{
				cout << "strategy_layer.cc:"<<__LINE__<<": ERROR: No paths connecting"
					<<" node "<<getParentModule()->getIndex() <<" to node "<< dest <<
					" have been found"<<endl;
				exit(-5);
			}

			// Choose the paths towards the target according to the chosen forwarding strategy.
			vector<int> paths = choose_paths(node->getNumPaths());
//			cout << "The number of paths from " << getParentModule()->getIndex() <<" to "<< dest << "are: " << paths.size() << endl;
			for (unsigned int i=0; i<paths.size(); i++)
			{
				int output_gate = node->getPath( i )->getLocalGate()->getIndex();  // Extract to ID of the output interface
				//int output_gate = node->getPath( i )->getLocalGateId();  // Extract to ID of the output interface
																				   // to reach the target.
				int distance = node->getDistanceToTarget();						   // Extract the distance to reach the target.
				add_FIB_entry(dest, output_gate, distance);						   // Add the corresponding FIB entry.
				//int nextNode = getParentModule()->gate("face$o",output_gate)->getNextGate()->getOwnerModule()->getIndex();
				//cout << "*** FIB ***\tNODE # " << getParentModule()->getIndex() << " Dest # " << dest << " Next Hop # " << nextNode << " at Distance # " << distance << endl;
			}
			

			//cout<<getParentModule()->gate("face$o",FIB[d].id)->getNextGate()->getOwnerModule()->getIndex()+1<<" ";
			//cout<<FIB[d].len<<" ";
		}
		else
			;//cout<<getParentModule()->getIndex()+1<<" ";
			//cout<<0<<" ";
    }
}

void strategy_layer::populate_from_file()
{
    string rline, dline;
    getline(frouting, rline);
    getline(fdist, dline);

    istringstream diis (dline);
    istringstream riis (rline);
    int n = getAncestorPar("n");

    int cell1, cell2;
    int k = 0;

    while (k<n){
	riis>>cell1;
	diis>>cell2;
	int out_interface = gatelu[cell1-1];
	int distance = cell2;
	add_FIB_entry(k, out_interface, distance);
	k++;
    }
}

/**
 * distance: the length of the path to reach the destination node passing through
 * the specified interface
 */
void strategy_layer::add_FIB_entry(int destination_node_index, int interface_index, int distance)
{
	int_f FIB_entry;
	FIB_entry.id = interface_index;
	FIB_entry.len = distance;
	FIB[destination_node_index].push_back(FIB_entry);
	
	#ifdef SEVERE_DEBUG
	vector<int_f> entry_vec = FIB[destination_node_index];
	int_f entry_just_added = entry_vec.back();
	int output_gates = getParentModule()->gateSize("face$o");
	if (entry_just_added.id >= output_gates){
		std::stringstream msg; msg<<"gate "<<entry_just_added.id<<" is invalid"<<
				". gate_size is "<< output_gates;
		severe_error(__FILE__,__LINE__, msg.str().c_str() );
	}
	#endif
}

const vector<int_f> strategy_layer::get_FIB_entries(
		int destination_node_index)
{
	vector<int_f> entries = FIB[destination_node_index] ;
	return entries;
}

int strategy_layer::get_out_interface(int destination_node)
{
	//int id = FIB[destination_node].operator [](0).id;
	//vector<int_f> entries = FIB.operator [](destination_node);
	/*cout << "lunghezza vettore: " << entries.size() << endl;
	for (int i=0; i < entries.size(); i++)
	{
		cout << "FIB entry # " << i << " with path length: " << entries[i].len << endl;
	}
	int id = FIB[destination_node].operator [](0).id;*/
	int id;
	if (FIB.count(destination_node) > 0)
		id = FIB.operator [](destination_node).front().id;  // like MonopathStrategy::get_FIB_entry.
	else
		id = 1000;
	return id;
}
