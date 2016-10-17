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
#ifndef IDEAL_BLIND_POLICY_H_
#define IDEAL_BLIND_POLICY_H_

//<aa>
#include "decision_policy.h"
#include "error_handling.h"
#include "lru_cache.h"

class Ideal_blind: public DecisionPolicy{
	protected:
		lru_cache* mycache; // cache I'm attached to

    public:
		Ideal_blind(base_cache* mycache_par):
			DecisionPolicy()
		{
			mycache = dynamic_cast<lru_cache*>(mycache_par);

			// CHECK{
				if( mycache == NULL ){
					std::stringstream ermsg; 
					ermsg<<"this policy works only with lru";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			// }CHECK

		};

		virtual bool data_to_cache(ccn_data * data_msg)
		{
			bool decision;

			#ifdef SEVERE_DEBUG
			if( !mycache->is_initialized() ){
				std::stringstream ermsg; 
				ermsg<<"base_cache is not initialized.";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
			#endif

			if (! mycache->full() )
				decision = true;
			else{

				chunk_t new_content_index = data_msg->getChunk();
				lru_pos* lru_element_descriptor = mycache->get_lru();
				chunk_t lru_index = lru_element_descriptor->k;

				if (new_content_index <= lru_index)
					// Inserting this content in the cache would make it better
					decision = true;
				else 
					decision = false;

			}
			return decision;
		};
};
//<//aa>
#endif

