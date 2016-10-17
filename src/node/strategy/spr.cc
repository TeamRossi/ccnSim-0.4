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
#include <omnetpp.h>
#include <algorithm>
#include "spr.h"
#include "ccn_interest.h"
#include "error_handling.h"
#include <sstream>


Register_Class(spr);



bool *spr::get_decision(cMessage *in){

    bool *decision;
    if (in->getKind() == CCN_I){
	ccn_interest *interest = (ccn_interest *)in;
	decision = exploit(interest);
    }
    return decision;

}



//The nearest repository just exploit the host-centric FIB. 
bool *spr::exploit(ccn_interest *interest){

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
int spr::nearest(vector<int>& repositories){
	#ifdef SEVERE_DEBUG
	if (repositories.size()==0)
		severe_error(__FILE__,__LINE__, "repositories has 0 elements");
	#endif
	
    int  min_len = 10000;
    vector<int> targets;

    for (vector<int>::iterator i = repositories.begin(); i!=repositories.end();i++) 	{ 	//Find the shortest (the minimum)
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

