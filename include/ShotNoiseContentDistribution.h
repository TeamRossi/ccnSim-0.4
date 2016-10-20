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
#ifndef SHOTNOISECONTENT_DISTRIBUTION_H
#define SHOTNOISECONTENT_DISTRIBUTION_H

#include <omnetpp.h>
#include "ccnsim.h"
#include "content_distribution.h"
#include "zipf.h"
#include "zipf_sampled.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/tokenizer.hpp>


using namespace std;


class ShotNoiseContentDistribution : public content_distribution{
	public:
		#ifdef SEVERE_DEBUG
		ShotNoiseContentDistribution() : initialized(false) {;}
		// http://stackoverflow.com/a/7863971/2110769
		#endif

		// Functions related to the State Flags (ON/OFF) associated to contents.
		void set_state_flag(int);
		void unset_state_flag(int);
		bool check_state_flag(int);

		// Functions related to the Transition Times associated to each contents.
		void set_state_time(int, simtime_t);
		simtime_t get_state_time(int);

		// Struct containing useful information associated to each popularity class.
		struct classInfoEntry
		{
			classInfoEntry (simtime_t _ton, double _eV, double _classAlpha, long double _numContents):
						   ton(_ton), toff(0), eV(_eV), classAlpha(_classAlpha), numContents(_numContents), lambdaClass(0), percReq(0) {}

			simtime_t ton;						// Mean ON time of the class (they are exponentially distributed).
			simtime_t toff;						// Deterministic OFF time of the class.
			double eV;							// Mean number of requests during the ON periods.
			double classAlpha;					// Zipf's exponent of the class.
			long double numContents;			// Number of contents inside the class.
			double lambdaClass;					// Aggregated request arrival rate of the class.
			double lambdaClient;				// Request arrival rate of the single client for contents of the relative class.
			double percReq;						// Probability to generate a request for the relative class.
			unsigned int mostPopular;			// ID of the most popular content of the class.
			unsigned int lessPopular;			// ID of the less popular content of the class.
		};

		vector<classInfoEntry>* classInfo;      // Vector containing information of all the classes.
		double totalLambda = 0;					// Aggregated request arrival rate of all the classes.
		long double totalContents = 0;          // Total number of contents.
		int numOfClasses = 0;					// Total number of classes.
		int tOffMultFactor = 0;					// Toff multiplicative factor (i.e., Toff = k * Ton).
		long double totalRequests = 0;			// User-specified total number of simulated requests.
		double steadySimTime = 0;				// Calculated steady state simulation time (it depends on totalRequests, totalLambda, and numClients).
		int numClients = 0;						// Number of simulated clients in the network.
		bool shotNoiseActive = false; 			// Flag indicating if the shot noise model is active.

		//static vector<zipf_distribution*> zipfClasses;   // Structure containing the popularity distributions of each class.
		static vector<zipf_sampled*> zipfClasses;   // Structure containing the popularity distributions of each class.

//		#ifdef SEVERE_DEBUG
//		virtual bool isInitialized();
//		#endif


    protected:
		void initialize();
		void finish();

		void import_catalog_features(const char*);
		void initialize_contents();


	private:
		boost::dynamic_bitset<>* state_flags;     	// Vector of State flags (i.e., 1=ON, 0=OFF) per each content.
		vector<simtime_t>*	state_times;			// Vector of Transition Times per each content.


		#ifdef SEVERE_DEBUG
		bool initialized;
		#endif
};
#endif
