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
 
//<aa>
#include <omnetpp.h>
#include "MonopathStrategyLayer.h"
#include "ccnsim.h"
#include "error_handling.h"

//Register_Class(MonopathStrategyLayer);

const int_f MonopathStrategyLayer::get_FIB_entry(
		int destination_node_index)
{
	const vector<int_f> FIB_entries = get_FIB_entries(destination_node_index);
	#ifdef SEVERE_DEBUG
	int output_gates = getParentModule()->gateSize("face$o");
	std::stringstream msg;
	int_f entry = FIB_entries.front();
	msg<<"I'm inside node with id "<< getParentModule()->getId()
		<< " and with index " << getParentModule()->getIndex();
	msg<<". gate size is "<<output_gates << ", node to reach is "
		<< destination_node_index << ", the gate is "<< entry.id;
	if (entry.id >= output_gates){
		severe_error(__FILE__,__LINE__, "selected gate is invalid");
	}
	#endif
	return FIB_entries.front();
}

vector<int> MonopathStrategyLayer::choose_paths(int num_paths)
{
	std::stringstream msg;
	//msg<<"I'm inside node with id "<< getParentModule()->getId()
	//	<< " and with index " << getParentModule()->getIndex();
	//msg<<"I'm inside MonopathStrategyLayer::choose_paths\n\n\n\n\n\n\n\n\n\n\n\n\n";
	//debug_message(__FILE__,__LINE__,msg.str().c_str() );
	vector<int> v;
	v.push_back( num_paths == 1 ? 0 : intrand (num_paths) );
	return v;
}

void MonopathStrategyLayer::initialize(){
	strategy_layer::initialize();
}

void MonopathStrategyLayer::finish(){
	strategy_layer::finish();
}
//</aa>
