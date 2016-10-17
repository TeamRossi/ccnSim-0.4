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
#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "ccnsim.h"
#include <omnetpp.h>
#include <fstream>

#include <boost/unordered_map.hpp>

using namespace std;
using namespace boost;


struct int_f
{
    int  id; 	// Interface ID.
    int  len;	// Path length to reach a destination node through the specified interface.

    bool operator<(int_f other){
	return (other.len > this->len);
    }
};


//	Basic strategy layer. It defines a get_decision function in order to handle the forwarding of Interest packets.
class strategy_layer: public abstract_node{
    public:
		/**
		 * It returns an array of boolean values, one for each output gate.
		 * The Interest is forwarded towards the gates whose ID corresponds the the positions of 1s
		 * inside the array.
		 */
		virtual bool* get_decision(cMessage *)=0;
		
		// Useful only for the execution of the model with NRR
		virtual bool* exploit_model(long m) = 0;

		static ifstream fdist;
		static ifstream frouting;
		const vector<int_f> get_FIB_entries(int destination_node_index);
		int get_out_interface(int destination_node);
    protected:
		virtual void initialize();
		virtual void finish();

		void populate_routing_table();
		void populate_from_file();

		void add_FIB_entry(int destination_node_index, int interface_index,	int distance);
		virtual vector<int> choose_paths(int num_paths)=0;

		void handleMessage(cMessage *);

	private:
		// Associates to each destination node, an output interface to reach it.
		unordered_map <int ,vector<int_f> > FIB; 	
		unordered_map <int, int> gatelu;	
		int nodes;
		// Messages for link failure/recovery and route re-computation.
		cMessage *failure;
		cMessage *recovery;
		cMessage *new_routes;
		cMessage *new_model;  // New model computation.

		int failRecInt;   	// Index of the interface attached to the link to be failed and recovered.

		double fail_time;			// Scheduled failure time;
		double fail_duration;		// Scheduled recovery time;
		double fail_transient;		// Time period before route re-computation;

		bool fail_scenario;			// If 'true', a dynamic scenario is simulated.
};
#endif
