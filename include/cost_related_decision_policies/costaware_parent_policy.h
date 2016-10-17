/*
 * ccnSim is a scalable chunk-level simulator for Content Centric
 * Networks (CCN), that we developed in the context of ANR Connect
 * (http://www.anr-connect.org/)
 *
 * People:
 *    Giuseppe Rossini (Former lead developer, mailto giuseppe.rossini@enst.fr)
 *    Raffaele Chiocchetti (Former developer, mailto raffaele.chiocchetti@gmail.com)
 *    Andrea Araldo (Principal suspect 1.0, mailto araldo@lri.fr)
 *    Michele Tortelli (Principal suspect 1.1, mailto michele.tortelli@telecom-paristech.fr)
 *    Dario Rossi (Occasional debugger, mailto dario.rossi@enst.fr)
 *    Emilio Leonardi (Well informed outsider, mailto emilio.leonardi@tlc.polito.it)
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
#ifndef COSTAWARE_PARENT_POLICY_H_
#define COSTAWARE_PARENT_POLICY_H_

//<aa>
#include "ccnsim.h"
#include "decision_policy.h"
#include "error_handling.h"
#include "costaware_ancestor_policy.h"
#include "WeightedContentDistribution.h"

// This is an abstract class
class Costaware_parent: public Costaware_ancestor{
    public:
		Costaware_parent(double average_decision_ratio_):
			Costaware_ancestor(average_decision_ratio_){}

		virtual bool data_to_cache(ccn_data * data_msg)
		{
			double x = getRNG(0)->doubleRand();
			//double x = ((double) rand() / (RAND_MAX)); 	
			double price = data_msg->getPrice();
			double acceptance_probability = (double)  (pow(price, kappa) )* correction_factor;

			#ifdef SEVERE_DEBUG
			if ( acceptance_probability >= 1 ){
				std::stringstream msg; 
				msg<<"The probability to accept the object whose price is "<< price <<" is "<<
					acceptance_probability << ". It is too high. It may be an error. If you are sure "<<
					"that it is not, disable this error message";
				severe_error(__FILE__, __LINE__, msg.str().c_str() );
			}
			#endif


			if (x < acceptance_probability )
					return true;

			return false;
		}

		virtual double compute_correction_factor()= 0; // This is an abstract class
};
//<//aa>
#endif

