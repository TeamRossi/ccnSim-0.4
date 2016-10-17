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
#ifndef COSTAWARE_POLICY_H_
#define COSTAWARE_POLICY_H_

//<aa>
#include "ccnsim.h"
#include "decision_policy.h"
#include "error_handling.h"
#include "WeightedContentDistribution.h"
#include "costaware_parent_policy.h"

class Costaware: public Costaware_parent{
    public:
		Costaware(double average_decision_ratio_):
			Costaware_parent(average_decision_ratio_)
		{
			correction_factor = compute_correction_factor();

			#ifdef SEVERE_DEBUG
				std::stringstream msg; 
				msg<<"correction_factor="<< correction_factor;
				debug_message(__FILE__, __LINE__, msg.str().c_str() );
			#endif

		}

		virtual double compute_correction_factor(){
			return average_decision_ratio / (catalog_split[1] +  catalog_split[2] * pow(priceratio,kappa) ) ;
		}

};
//<//aa>
#endif

