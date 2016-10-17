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
#include "ProbabilisticSplitStrategy.h"
#include "ccnsim.h"
#include "ccn_interest.h"
#include "ccn_interest_m.h"
#include "base_cache.h"
#include "error_handling.h"
#include <sstream> // to use stringstream

Register_Class(ProbabilisticSplitStrategy);


void ProbabilisticSplitStrategy::initialize()
{
    strategy_layer::initialize();
    
    
    // ref: omnet 4.3 manual, sec 4.5.4
	const char *vstr = par("split_factors").stringValue(); // e.g. "aa bb cc";
	split_factors = cStringTokenizer(vstr).asDoubleVector();

	int node_index = getParentModule()->getIndex();
	std::stringstream msg; 
	msg<<"node "<< node_index;
	msg<<" has a  ProbabilisticSplitStrategy. Split factor string is "<<
    	vstr;
    debug_message(__FILE__, __LINE__, msg.str().c_str() );


	// Verify if split factors are correct
	unsigned out_gates = getParentModule()->gateSize("face$o");
	if (split_factors.size() != out_gates )
		severe_error(__FILE__,__LINE__, "The number of slipt factors is different from the number of output gates");
	double sum = 0;
	for (unsigned i=0; i < split_factors.size() ; i++ )
		sum += split_factors[i];
	if (sum != 1)
		severe_error(__FILE__,__LINE__, "The sum of slipt factors should be 1");
}
bool* ProbabilisticSplitStrategy::get_decision(cMessage *in){

    bool *decision;
    if (in->getKind() == CCN_I){
	ccn_interest *interest = (ccn_interest *)in;
	decision = exploit(interest);
    }
    return decision;
}


int ProbabilisticSplitStrategy::decide_out_gate(vector<int_f> FIB_entries)
{
	int out_gate = UNDEFINED_VALUE;

	if(FIB_entries.size() == 1)
		out_gate = FIB_entries[0].id;
	else{
		while (out_gate == UNDEFINED_VALUE) 
		{	//extract an out_gate until a valid one is found
			double r = uniform(0, 1);
			double sum = 0; unsigned i = 0;
			
			while (1)
			{
				sum += split_factors[i];
				if (sum > r)
					break;
				i++;
				#ifdef SEVERE_DEBUG
				if ( ( i == split_factors.size() && sum < 1 ) ||
					  i > split_factors.size() || sum > 1  ){
					std::stringstream msg;
					msg<<"split_factors.size()="<<split_factors.size()<<
						"; i="<<i<<"; sum="<<sum;
					severe_error(__FILE__,__LINE__, msg.str().c_str() );
				}
				#endif
			}
			unsigned int chosen_gate = i;
			#ifdef SEVERE_DEBUG
			if (chosen_gate >= split_factors.size())
				severe_error(__FILE__, __LINE__, "");
			#endif

						
			// If the chosen gate is included in the FIB_entries,
			// use it. Otherwise, start again the while loop
			for (unsigned int j=0; j < FIB_entries.size(); j++){
				int_f entry = FIB_entries[j];
				if (entry.id == (int)chosen_gate){
					out_gate = chosen_gate; break;
				}
			}
			
			//std::stringstream msg;
			//msg<<"chosen_gate="<<i<<" and out_gate is "<<out_gate;
			//debug_message(__FILE__,__LINE__, msg.str().c_str() );

		}
	}
	
	#ifdef SEVERE_DEBUG
	if(out_gate == UNDEFINED_VALUE)
		severe_error(__FILE__,__LINE__,"It was not possible to find an output gate");
	#endif
	
	return out_gate;
}


int ProbabilisticSplitStrategy::decide_target_repository(ccn_interest *interest)
{
	int repository;
	if (interest->getRep_target() == UNDEFINED_VALUE)
    {
    	// Get all the repositories that store the content demanded by the
    	// interest
		vector<int> repos = interest->get_repos();
		
		// Choose one of them
		repository = repos[intrand(repos.size())];
		interest->setRep_target(repository);
    }else 
		repository = interest->getRep_target();
	
	return repository;
}


bool* ProbabilisticSplitStrategy::exploit(ccn_interest *interest)
{
    int repository,
	gsize;

    gsize = __get_outer_interfaces(); //number of gates
	repository = decide_target_repository(interest);

    bool *decision = new bool[gsize];
	std::fill(decision,decision+gsize,0);
    	
	const vector<int_f> FIB_entries = get_FIB_entries(repository);
	int out_gate = decide_out_gate(FIB_entries);
	decision[out_gate]=true;
    return decision;
}

vector<int> ProbabilisticSplitStrategy::choose_paths(int num_paths)
{
	std::stringstream msg;
	//msg<<"I'm inside node with id "<< getParentModule()->getId()
	//	<< " and with index " << getParentModule()->getIndex();
	//msg<<"I'm inside ProbabilisticSplitStrategy::choose_paths\n\n\n\n\n\n\n\n\n\n\n\n\n";
	//debug_message(__FILE__,__LINE__,msg.str().c_str() );
	vector<int> v;
	for (int i=0; i<num_paths; i++)
		v.push_back( i );
	return v;
}

void ProbabilisticSplitStrategy::finish(){
    MultipathStrategyLayer::finish();
}
//</aa>
