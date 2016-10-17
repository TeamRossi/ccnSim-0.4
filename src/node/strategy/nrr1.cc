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
 *    ccnsim@listes.telecom-paristech.fr
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
#include "ccn_interest.h"
#include "ccnsim.h"
#include "nrr1.h"
Register_Class(nrr1);


bool *nrr1::get_decision(cMessage *in){//check this function
    bool *decision;
    int gsize = __get_outer_interfaces();
    decision = new bool[gsize];
    std::fill(decision,decision+gsize,0);
    ccn_interest *interest;
    int dyn_TTL = par("TTL1");


    if (in->getKind() == CCN_I){
        interest = (ccn_interest *)in; //safely cast
	if (interest->getNfound()){
	    decision = exploit_nearest(interest);
	}else if (interest->getHops() >= dyn_TTL){
	    int gsize = __get_outer_interfaces();
	    decision = new bool[gsize];
	    std::fill(decision,decision+gsize,0);
	}else {
            decision  = explore(interest);
        }
    }
    return decision;

}


/*
 * Explore the network if the target is not yet defined. The target is the node
 * (repository or cache) which stores the nearest copy of the data.
 */
bool *nrr1::explore(ccn_interest *interest){
    int arrival_gate,
        gsize;
    bool *decision;

    gsize = __get_outer_interfaces();
    arrival_gate = interest->getArrivalGate()->getIndex();
    decision = new bool[gsize];

    std::fill(decision,decision+gsize,1);
    decision[arrival_gate] = false;
    return decision;
}




/*
 * Exploit the knowledge of the network acquired during the exploration phase.
 * Of course, the given target may withdraw the content. In this case is the
 * given target that explores again the network looking for content close to
 * himself.
 */
bool *nrr1::exploit(ccn_interest *interest){


    bool *decision;
    int outif,
	gsize,
	target;

    gsize = __get_outer_interfaces();
    target = interest->getTarget();

    if (interest->getTarget() == getIndex()){//failure
        interest->setTarget(-1);
        return explore(interest);
    }

	//<aa>
	const int_f FIB_entry = get_FIB_entry(target);
	//</aa>
    outif = FIB_entry.id;

    decision = new bool[gsize];
    std::fill(decision,decision+gsize,0);
    decision[outif]=true;

    return decision;

}

bool *nrr1::exploit_nearest(ccn_interest *interest){

    int repository,
        outif,
        gsize;

    gsize = __get_outer_interfaces();

    vector<int> repos = interest->get_repos();
    repository = nearest(repos);

	//<aa>
	const int_f FIB_entry = get_FIB_entry(repository);
	//</aa>
    outif = FIB_entry.id;


    bool *decision = new bool[gsize];
    std::fill(decision,decision+gsize,0);
    decision[outif]=true;

    return decision;

}

int nrr1::nearest(vector<int>& repositories){
    int  min_len = 10000;
    vector<int> targets;

    for (vector<int>::iterator i = repositories.begin(); i!=repositories.end();i++){ 	//Find the shortest (the minimum)
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
    return targets[intrand(targets.size())];
}

