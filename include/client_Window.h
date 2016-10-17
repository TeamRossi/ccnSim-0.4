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
#ifndef CLIENT_WINDOW_H_
#define CLIENT_WINDOW_H_

#include <omnetpp.h>
#include <random>
#include "ccnsim.h"
#include "client.h"


using namespace std;

class client_Window : public client {
	public:

    protected:
		virtual void initialize();
		virtual void handleMessage(cMessage *);
		virtual void finish();

		virtual void request_file(int);		// For IRM clients the class_num will be always '0' by default.

		void request_file_window(int);

    private:
		cMessage *arrival;		// Message to trigger content requests.
		cMessage *timer;		// Message to trigger timers.

		cMessage *winTimeout;	// Message to trigger window reset.
		int defWinSize;			// Initial and default window size.
		int maxWinSize;			// Maximum window size.
		int currWinSize;		// Current window size.
		int inFlightPkts; 		// Number of in flight packets.
		double winTout;		// Time interval to reset the window size
};
#endif
