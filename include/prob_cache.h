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
#ifndef PROB_CACHE_H_
#define PROB_CACHE_H_

#include "decision_policy.h"

//The most simple (and used) policy: cache data always
class prob_cache: public DecisionPolicy{
    public:
	prob_cache(int c):N(c){;}

	bool data_to_cache(ccn_data *data){

	    //cout<<"Capacity:"<<data->getCapacity()<<endl;
	    //cout<<"TSB:"<<data->getTSB()<<endl;
	    //cout<<"TSI:"<<data->getTSI()<<endl;

	    double times_in, w, p, x;
	    bool decision = false;

	    times_in = data->getCapacity() * 1./ (10*N);
	    w = data->getTSB() * 1./data->getTSI();
	    p = w * times_in;
	    //x = ((double) rand() / (RAND_MAX));
	    x = getRNG(0)->doubleRand();
	    //cout<<"TimesIn: "<<times_in<<endl;
	    //cout<<"Weight: "<<w<<endl;
	    //cout<<x<<"<=>"<<p<<endl;

	    if ( x < p){
		decision = true;
	    }

	    data->setCapacity(data->getCapacity()-N);
	    data->setTSB(data->getTSB()+1);



	    return decision;

	}
    private:
	int N;
};

#endif
	
