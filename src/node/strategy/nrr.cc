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
#include <omnetpp.h>
#include <algorithm>
#include "nrr.h"
#include "ccnsim.h"
#include "ccn_interest.h"
#include "base_cache.h"
#include "error_handling.h"

Register_Class(nrr);

struct lookup{
    chunk_t elem;
    lookup(chunk_t e):elem(e){;}
    bool operator() (Centry c) const { return c.cache->fake_lookup(elem); }
};

struct lookup_len{
    chunk_t elem;
    int len;
    lookup_len(chunk_t e,int l):elem(e),len(l){;}
    bool operator() (Centry c) const { return c.cache->fake_lookup(elem) && c.len ==len; }
};


void nrr::initialize(){
    strategy_layer::initialize();
    vector<string> ctype;
    ctype.push_back("modules.node.node");
    TTL = par("TTL2");

    cTopology topo;
    topo.extractByNedTypeName(ctype);
    for (int i = 0;i<topo.getNumNodes();i++)
	{
		if (i==getIndex()) continue;
		//base_cache *cptr = (base_cache *)topo.getNode(i)->getModule()->getModuleByPath("content_store");
		base_cache *cptr = (base_cache *)topo.getNode(i)->getModule()->getSubmodule("content_store");
		if(contStore)
		{
			//<aa>
			const int_f FIB_entry = get_FIB_entry(i);
			if (FIB_entry.len <= TTL)
				cfib.push_back( Centry ( cptr, FIB_entry.len ) );
		}
		else
		{
			std::stringstream ermsg;
			ermsg<<"ERROR - TTL CACHE: cannot retrieve the pointer to the content store. Please check";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
    }	
    //Commented the following if block
	//</aa>
	/**
	if (FIB[i].len <= TTL)
	    cfib.push_back( Centry ( cptr, FIB[i].len ) );
    }
    */
    
    sort(cfib.begin(), cfib.end());
}

bool *nrr::get_decision(cMessage *in){

    bool *decision;
    if (in->getKind() == CCN_I){
		ccn_interest *interest = (ccn_interest *)in;
		decision = exploit(interest);
    }
    return decision;

}



//The nearest repository just exploit the host-centric FIB. 
bool *nrr::exploit(ccn_interest *interest){

    int repository,
	node,
	output_iface,
	gsize,
	times;

	output_iface = -1;

    gsize = __get_outer_interfaces();
    bool *decision = new bool[gsize];
    std::fill(decision,decision+gsize,0);

	//<aa>
	#ifdef SEVERE_DEBUG
		vector<Centry>::iterator node_it; // This iterator will point to the target node

//		if (interest->getChunk() == 243 && interest->getOrigin()==0)
//		{
//			std::stringstream ermsg; 
//			ermsg<<"I am node "<<getIndex()<<
//				"; I received interest for object ="<<interest->getChunk() <<
//				" issued by client attached to node "<< interest->getOrigin()<<
//				". its target is "<<interest->getTarget()<<
//				". Serial number="<<interest->getSerialNumber();
//			debug_message(__FILE__,__LINE__,ermsg.str().c_str() );
//		}
	#endif
	//</aa>

    if (interest->getTarget() == -1 
		//<aa> 	The interest has no target node () the preferential node to be sent 
		// 		to</aa>
		// || interest->getTarget() == getIndex()
		//<aa> The target of the interest is this node </aa>

	){
	    //find the first occurrence in the sorted vector of caches.
		vector<Centry>::iterator it = 
			std::find_if (cfib.begin(),cfib.end(),lookup(interest->getChunk()) );

		vector<int> repos = interest->get_repos();
		repository = nearest(repos);

		//<aa>
		const int_f FIB_entry = get_FIB_entry(repository);
		//</aa>

		if (it!=cfib.end() && it->len <= FIB_entry.len+1)
		{//found!!!
			//<aa>	It is possible to reach the content through the interface indicated
			//		by 'it'. Moreover, this path is shorter than the path related to
			//		the FIB_entry </aa>


			//<aa>
			vector<int> potential_targets;
			int select;

			// Compute target node: dummy way
			{
				#ifdef SEVERE_DEBUG
					vector< vector<Centry>::iterator > potential_targets_it;
				#endif
				// Take all the targets with minimum distance and randomly choose one of them
				for (vector<Centry>::iterator it2 = cfib.begin(); 
					it2 != cfib.end() && it2->len <= it->len;
					it2++
				){
						if (it2->cache->fake_lookup(interest->getChunk() ) )
						{	potential_targets.push_back( it2->cache->getIndex() );
							#ifdef SEVERE_DEBUG
								if (it2->len < it->len){
									std::stringstream ermsg; 
									ermsg<<"I am node "<<getIndex()<<". ERROR ";
									severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
								}
								potential_targets_it.push_back(it2);
							#endif
						}
				}
				select = intrand(potential_targets.size() );			
				node = potential_targets[select];
				#ifdef SEVERE_DEBUG
					node_it = potential_targets_it[select];
				#endif
			}


			//<aa>
			// The previous block of code replace :
			// Compute target node: efficient way
			if (false) //This is the old code. It is more efficient but it has some problem
			{
				times = std::count_if (cfib.begin(),cfib.end(),
						lookup_len(interest->getChunk(),it->len) );
				int select = intrand(times);
				it+=select;
				// node = it->cache->getIndex();

				#ifdef SEVERE_DEBUG
				if ( it->cache->fake_lookup(interest->getChunk() ) == false )
				{
					std::stringstream ermsg; 
					ermsg<<"I am node "<<getIndex()<<". I set node "<<
						 node <<" as target for chunk "<<interest->getChunk() <<
						". Serial number="<<interest->getSerialNumber()<<
						". But node "<< 
						node<<" does not contain that chunk."<<
						times<<" nodes are holding the chunk in question.";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
				#endif
			}
			//</aa>

			//<aa> Slightly modified
			output_iface = get_FIB_entry(node).id;
			//</aa>
			interest->setTarget(node);

			//<aa>
			#ifdef SEVERE_DEBUG
				times = std::count_if (cfib.begin(),cfib.end(),
						lookup_len(interest->getChunk(),it->len) );
				if (times == 1){
					// The node calculated above should be obtained also as follows
					int select = intrand(times);
					it+=select;
					int node_verification = it->cache->getIndex();
					if (node_verification != node){
						std::stringstream ermsg; 
						ermsg<<"I am node "<<getIndex()<<". node="<< node <<
							"; node_verification="<<node_verification<<
							". They must be the same.";
						severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
					}
				}

				if ( node_it->cache->fake_lookup(interest->getChunk() ) == false ){
					std::stringstream ermsg; 
					ermsg<<"I am node "<<getIndex()<<". I set node "<<
						 node <<" as target for chunk "<<interest->getChunk() <<
						". Serial number="<<interest->getSerialNumber()<<
						". But node "<< 
						node<<" does not contain that chunk."<<
						times<<" nodes are holding the chunk in question.";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}

				if ( output_iface != get_FIB_entry(interest->getTarget() ).id )
				{
					std::stringstream ermsg; 
					ermsg<<"I am node "<<getIndex()<<". I set node "<<node <<
						" as target for interest for chunk "<<interest->getChunk() <<
						". Serial number="<<interest->getSerialNumber()<<
						". To reach that node I should use interface  "<< 
						get_FIB_entry(interest->getTarget() ).id <<
						"; but I set interface "<<
						output_iface <<" as output interface. This is an error";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			#endif
			//</aa>

		}else{//not found
			//<aa> There are no alternatives to the FIB entry to reach the content</aa>
			output_iface = FIB_entry.id;
			interest->setTarget(repository);
		}

    }
	//<aa>
	else if (interest->getTarget() == getIndex() )
	{
		vector<int> repos = interest->get_repos();
		repository = nearest(repos);
		const int_f FIB_entry = get_FIB_entry(repository);

		output_iface = FIB_entry.id;
		interest->setTarget(repository);
		interest->setAggregate(false);

//		#ifdef SEVERE_DEBUG
//			std::stringstream ermsg; 
//			ermsg<<"I am node "<<getIndex()<<
//				".I am the target for  an interest for chunk "<<
//				interest->getChunk() <<", issued by client attached to node "<< interest->getOrigin()<<
//				". Serial number "<<interest->getSerialNumber() <<
//				" but I do not have that chunk. I set the repo "<<repository<<" as target node";
//			debug_message(__FILE__,__LINE__,ermsg.str().c_str() );
//		#endif
	}
	//</aa>
	else 
	{
		//<aa>	The interest has already a target that is not this node. 
		// I will send the
		//		interest toward that target </aa>
		const int_f FIB_entry2 = get_FIB_entry(interest->getTarget() );
		output_iface = FIB_entry2.id;
    }

	//<aa>
	#ifdef SEVERE_DEBUG
	if ( output_iface == -1 ){
		std::stringstream ermsg; 
		ermsg<<"I am node "<<getIndex()<<".The output iface for interest for chunk "
			<< interest->getChunk() <<" issued by client attached to node "<< interest->getOrigin()<<
			" was not assigned";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif
	//</aa>

    decision[output_iface] = true;
    return decision;
}

/*
 * 		Exploit function for the model execution with NRR.
 */
bool *nrr::exploit_model(long content){

	// We should return a vector of bool indicating the output interfaces.
	// CONSIDER: Nel modello, quando si calcola il rate in ingresso ad un nodo, e quindi le p_hit dei vicini,
	//			 abbiamo bisogno di un modo per determinare il numero di potenziali destinatari (con out interface
	//			 diverse) a cui i vicini possono inviare (random) l'Interest ricevuto, in modo da stabilire volta
	//			 per volta gli ri,j.

	//cout << "*** Exploit MODEL *** Node # " << getIndex() << " with # " << __get_outer_interfaces() << " interf" << endl;

	unsigned long long m = (unsigned long long)content;
    int	gsize;

    gsize = __get_outer_interfaces();
    bool *output_ifaces = new bool[gsize];
    fill_n(output_ifaces, gsize, false);

    // Find the first occurrence in the sorted vector of caches.
	vector<Centry>::iterator it = std::find_if (cfib.begin(),cfib.end(),lookup(m) );

	// Find the original repo of the content
	repo_t repo = __repo(m+1);
	int l = 0;
	int repo_ID;
	while (repo)
	{
		if (repo & 1)
		{
			repo_ID = content_distribution::repositories[l];
		}
		repo >>= 1;
		l++;
	}

	const int_f FIB_entry = get_FIB_entry(repo_ID);

	if (it!=cfib.end() && it->len <= FIB_entry.len+1)    // A nearer cached copy has been found
	{
		//<aa>	It is possible to reach the content through the interface indicated
		//		by 'it'. Moreover, this path is shorter than the path related to
		//		the FIB_entry </aa>

		//<aa>
		vector<int> potential_targets;
		int node;

		// Take all the targets with minimum distance and randomly choose one of them
		for (vector<Centry>::iterator it2 = cfib.begin();
				it2 != cfib.end() && it2->len <= it->len;
				it2++)
		{
			if (it2->cache->fake_lookup(m) )
			{
				potential_targets.push_back( it2->cache->getIndex() );
			}
		}

		//select = intrand(potential_targets.size() );	// We return the list of the output interfaces to reach all
														// the potential targets.
		//node = potential_targets[select];
		for(uint32_t i=0; i < potential_targets.size(); i++)
		{
			node = potential_targets[i];
			output_ifaces[get_FIB_entry(node).id] = true;
		}
	}
	else  //not found
	{
		//<aa> There are no alternatives to the FIB entry to reach the content</aa>
		output_ifaces[FIB_entry.id] = true;
	}
    return output_ifaces;
}


int nrr::nearest(vector<int>& repositories){
    int  min_len = 10000;
    vector<int> targets
	//<aa>
			(0);
	//</aa>

    for (vector<int>::iterator i = repositories.begin(); i!=repositories.end();i++){ 		//Find the shortest (the minimum)
    	//<aa>
    	const int_f FIB_entry = get_FIB_entry(*i);
    	//</aa>
        if (FIB_entry.len < min_len ){
            min_len = FIB_entry.len;
            targets.clear();
            targets.push_back(*i);
        }else if (FIB_entry.len == min_len)
		    targets.push_back(*i);
    }

	//<aa>
	#ifdef SEVERE_DEBUG
	if (repositories.size() == 0){
		std::stringstream ermsg; 
		ermsg<<"There are 0 repositories. It's not admitted";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	int test = targets[0]; // Just to see if targets is properly allocated
	#endif
	//</aa>

    return targets[intrand(targets.size())];
}

void nrr::finish(){
    ;
    //string id = "nodegetIndex()+"]";
}

