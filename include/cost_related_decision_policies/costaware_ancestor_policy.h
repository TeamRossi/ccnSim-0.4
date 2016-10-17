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
#ifndef COSTAWARE_ANCESTOR_POLICY_H_
#define COSTAWARE_ANCESTOR_POLICY_H_

//<aa>
#include "decision_policy.h"
#include "error_handling.h"
#include "WeightedContentDistribution.h"

#define UNSET_COST -1

// This is an abstract class
class Costaware_ancestor: public DecisionPolicy{
    protected:
		double average_decision_ratio;
		double correction_factor; // An object will be cached with prob correction_factor * cost
		double kappa; //cost-aware exponent. See [icn14]
		double priceratio; //See [icn14]
		vector<double> catalog_split; //It corresponds to the split ratio vector (see [icn14])
		double last_accepted_content_price;
		WeightedContentDistribution* content_distribution_module;

    public:
		Costaware_ancestor(double average_decision_ratio_)
		{
			last_accepted_content_price = UNSET_COST;
			average_decision_ratio = average_decision_ratio_;

		    vector<string> ctype;
			ctype.push_back("modules.content.WeightedContentDistribution");
			cTopology topo;
	   		topo.extractByNedTypeName(ctype);


			#ifdef SEVERE_DEBUG
				if (num_content_distribution_modules != 1){
					std::stringstream msg; 
					msg<<"Found "<< num_content_distribution_modules << ". It MUST be 1";
					severe_error(__FILE__, __LINE__, msg.str().c_str() );
				}
			#endif

			cTopology::Node *content_distribution_node = topo.getNode(0);
			content_distribution_module = 
					(WeightedContentDistribution*) content_distribution_node->getModule();

			#ifdef SEVERE_DEBUG
			if ( !content_distribution_module->isInitialized() ){
					std::stringstream msg; 
					msg<<"content_distribution_module is not initialized";
					severe_error(__FILE__, __LINE__, msg.str().c_str() );
			}
			#endif

			catalog_split = content_distribution_module->get_catalog_split();
			unsigned num_repos = catalog_split.size();
			priceratio = content_distribution_module->get_priceratio();
			kappa = content_distribution_module->get_kappa();
			
			{ //check
				if (num_repos != 3){
					std::stringstream msg; 
					msg<<"num_repos = "<< num_repos << ". But only 3 is admitted";
					severe_error(__FILE__, __LINE__, msg.str().c_str() );
				}
			}


			#ifdef SEVERE_DEBUG
				std::stringstream msg; 
				msg<<"Decision policy initialized. kappa="<< kappa <<", catalog_split: ";
				for (unsigned j=0; j<num_repos; j++) {
					msg << catalog_split[j] <<",";
				}
				debug_message(__FILE__, __LINE__, msg.str().c_str() );
			#endif

		}


		virtual bool data_to_cache(ccn_data * data_msg) = 0;

		virtual void finish (int nodeIndex, base_cache* cache_p){			
		    char name [30];
			sprintf ( name, "correction_factor[%d]", nodeIndex);
			cache_p->recordScalar (name, correction_factor);

			sprintf ( name, "kappa[%d]", nodeIndex);
			cache_p->recordScalar (name, kappa);
		};

		virtual double get_last_accepted_content_price(){
			#ifdef SEVERE_DEBUG
			if (last_accepted_content_price == UNSET_COST){
					std::stringstream msg; 
					msg<<"last_accepted_content_price has nevere been set";
					severe_error(__FILE__, __LINE__, msg.str().c_str() );
			}
			#endif

			return last_accepted_content_price;
		};

		virtual void set_last_accepted_content_price(ccn_data * data_msg){
			last_accepted_content_price = data_msg->getPrice();
		}

		virtual double compute_correction_factor()=0; // This is an abstract class
};
//<//aa>
#endif

// References
// [icn14] A. Araldo, D. Rossi, F. Martignon, “Design and Evaluation of Cost-aware Information Centric Routers”, to appear in ACM Conference on Information-Centric Networking (ICN), Paris, 2014 
