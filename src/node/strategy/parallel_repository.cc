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
#include "parallel_repository.h"
#include "ccn_interest.h"

Register_Class(parallel_repository);

bool *parallel_repository::get_decision(cMessage *in){//check this function

    bool *decision;
    if (in->getKind() == CCN_I){
	ccn_interest *interest = (ccn_interest *)in;
	decision = exploit(interest);
    }
    return decision;

}



bool *parallel_repository::exploit(ccn_interest *interest){

    int outif,
	gsize;

    gsize = __get_outer_interfaces();
    bool *decision = new bool[gsize];
    std::fill(decision,decision+gsize,0);

    vector<int> repos = (interest->get_repos());
    for (vector<int>::iterator it = repos.begin(); it!=repos.end();it++){
    
    //<aa>
    const int_f FIB_entry = get_FIB_entry(*it);
    //</aa>
	outif = FIB_entry.id;
	decision[outif]=true;
    }

    return decision;

}
