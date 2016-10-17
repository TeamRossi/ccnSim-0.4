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
#ifndef IDEAL_COSTAWARE_PARENT_POLICY_H_
#define IDEAL_COSTAWARE_PARENT_POLICY_H_

//<aa>
#include "decision_policy.h"
#include "error_handling.h"
#include "lru_cache.h"
#include "WeightedContentDistribution.h"
#include "ideal_costaware_grandparent_policy.h"


// This is an abstract class
class Ideal_costaware_parent: public Ideal_costaware_grandparent{
	public:
		Ideal_costaware_parent(double average_decision_ratio_, base_cache* mycache_par):
			Ideal_costaware_grandparent(average_decision_ratio_, mycache_par)
		{			
			// Do nothing
		};

		virtual double compute_content_weight(chunk_t id, double cost)
		{
			double popularity_estimation = 1./pow(id, alpha);
			return cost * popularity_estimation;
		}

		virtual bool decide_with_cache_not_full(chunk_t id, double cost) = 0; // This is an abstract class
};
//<//aa>
#endif

