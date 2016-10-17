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
#ifndef IDEAL_COSTAWARE_GRANDPARENT_POLICY_H_
#define IDEAL_COSTAWARE_GRANDPARENT_POLICY_H_

//<aa>
#include "decision_policy.h"
#include "error_handling.h"
#include "costaware_ancestor_policy.h"
#include "lru_cache.h"
#include "WeightedContentDistribution.h"

// This is an abstract class

// This class represent a decision policy that decides to cache an object only if the new 
// object has a weight greater than the eviction candidate. Doing this way, the insertion
// of the new object makes the cache content "more valuable".
// See data_to_cache(..) to understand better.
class Ideal_costaware_grandparent: public Costaware_ancestor{
	protected:
		double alpha;
		lru_cache* mycache; // cache I'm attached to

    public:
		Ideal_costaware_grandparent(double average_decision_ratio_, base_cache* mycache_par):
			Costaware_ancestor(average_decision_ratio_)
		{

			if (kappa>1 || kappa<0){
				std::stringstream ermsg; 
				ermsg<<"kappa="<<kappa<<" is not valid";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
			}
			alpha = content_distribution_module->get_alpha();
			mycache = dynamic_cast<lru_cache*>(mycache_par);

			// CHECK{
				if( mycache == NULL ){
					std::stringstream ermsg; 
					ermsg<<"This policy works only with lru";
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

			chunk_t content_index = data_msg->getChunk();
			double cost = data_msg->getPrice();
			double x;
			if (! mycache->full() )
				decision = decide_with_cache_not_full(content_index, cost);
			else{

				double new_content_weight = compute_content_weight(content_index,cost);


				lru_pos* lru_element_descriptor = mycache->get_lru();
				content_index = lru_element_descriptor->k;
				cost = lru_element_descriptor->cost;
				double lru_weight = compute_content_weight(content_index,cost);

				if (new_content_weight > lru_weight)
				{	// Inserting this content in the cache would make it better
					decision = true;

				// a small kappa means that we tend to renew the cache often
                        		x = getRNG(0)->doubleRand();
					//x = ((double) rand() / (RAND_MAX));
				}
				else if ( x < kappa )
					decision = false;
				else
					decision = true;
			}

			if (decision == true)
				set_last_accepted_content_price(data_msg );

			return decision;
		};

		virtual double compute_correction_factor(){
			return 0;
		};

		virtual void after_insertion_action()
		{
			DecisionPolicy::after_insertion_action();
			#ifdef SEVERE_DEBUG
			if ( get_last_accepted_content_price() == UNSET_COST ){
				std::stringstream ermsg; 
				ermsg<<"cost_of_the_last_accepted_element="<<get_last_accepted_content_price() <<
					", while it MUST NOT be a negative number. Something goes wrong with the "<<
					"initialization of this attribute";
				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );

			}
			#endif

			// Annotate the cost of the last inserted element
			mycache->get_mru()->cost = get_last_accepted_content_price();

			#ifdef SEVERE_DEBUG
			// Unset this field to check if it is set again at the appropriate time
			// without erroneously use an old value
			last_accepted_content_price = UNSET_COST;
			#endif
			
		}

		virtual double compute_content_weight(chunk_t id, double cost)=0; // This is an abstract class
		virtual bool decide_with_cache_not_full(chunk_t id, double cost)=0;
};
//<//aa>
#endif

