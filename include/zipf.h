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
#ifndef ZIPF_H_
#define ZIPF_H_
#include <vector>

using namespace std;

class zipf_distribution{
    public:
		zipf_distribution(double a, int n):alpha(a),F(n){;};
		zipf_distribution(double a, double p, int n):alpha(a),q(p),F(n){;};
		// New constructors with catalog aggregation and class number.
		zipf_distribution(double a, double p, int n, double k):alpha(a),q(p),F(n),perc_aggr(k){;};
		zipf_distribution(double a, double p, int n, double k, int z):alpha(a),q(p),F(n),perc_aggr(k),class_num(z){;};
		zipf_distribution(){zipf_distribution(0,0);}
		void zipf_initialize();

		// 	Return the index of the content y such that the the sum of the
		//	probabilities of contents from 0 to y is >= p
		unsigned int value (double p);

		double get_normalization_constant();
		
		unsigned int get_ID_aggr();
		unsigned int get_catalog_card();
		double get_perc_aggr();
		double get_alpha();


    private:
		vector<double> cdfZipf;
		double alpha;
		double q;
		int F;
		double perc_aggr; 		// Specified percentile for catalog aggregation (e.g., 90-th percentile = 0.9).
		unsigned int ID_aggr;	// Content ID associated to the specified percentile for the catalog aggregation.
		int class_num = 1;    	// Class number to which the Zipf distribution is referred. Default is 1.

		double normalization_constant;
};
#endif
