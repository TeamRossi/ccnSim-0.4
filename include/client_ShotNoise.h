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
#ifndef CLIENT_SHOTNOISE_H_
#define CLIENT_SHOTNOISE_H_

#include <omnetpp.h>
#include <random>
#include "ccnsim.h"
#include "client.h"
#include "ShotNoiseContentDistribution.h"


using namespace std;


class client_ShotNoise : public client {
	public:
    protected:
		virtual void initialize(int);				// Multi-stage initialization.
		int numInitStages() const;
		virtual void handleMessage(cMessage *);
		virtual void finish();

		virtual void request_file(int);
		bool validateRequest(int, name_t);

    private:
		static ShotNoiseContentDistribution* snPointer;		// Pointer to the ShotNoiseContentDistribution class.

		vector<cMessage *> arrivals;		// There is a message for each popularity class in order to trigger the respective
											// request process.
		cMessage *timer;

		vector<bool> onOffClass;			// Vector indicating which class will be modeled as ON-OFF
											// (e.g., if false, requests for that class will be always generated).
};
#endif
