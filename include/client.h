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
#ifndef CLIENT_H_
#define CLIENT_H_

#include <omnetpp.h>
#include <random>
#include "ccnsim.h"
class statistics;
class ccn_data;
using namespace std;
#if OMNETPP_VERSION >= 0x0500
    using namespace omnetpp;
#endif


// Struct used to gather information about the current downloads
struct download {
    filesize_t chunk; 	// Number of chunks that still miss within the file.

    simtime_t start; 	// Starting download time.
    simtime_t last; 	// Last time a chunk has been downloaded.

	//<aa>
	#ifdef SEVERE_DEBUG
		int serial_number;
	#endif
	//</aa>

    download (double m = 0,simtime_t t = 0):chunk(m),start(t),last(t){;}
};

// Struct used to gather statistics for each single file
struct client_stat_entry{
    double avg_distance;		// Average hit distance.
    simtime_t avg_time;			// Average download time.
    double tot_downloads;		// Double type due to the chunkization in CCN. See below.
    unsigned int tot_chunks; 	// Number of chunks downloaded so far.

    client_stat_entry():avg_distance(0),avg_time(0),tot_downloads(0),tot_chunks(0){;}

};



class client : public cSimpleModule {
	public:
		double get_avg_distance();
		double get_tot_downloads();
		simtime_t get_avg_time();
		bool is_active();
		void clear_stat();
		int  getNodeIndex();


		#ifdef SEVERE_DEBUG
		// Returns true iff the content is among the current_downloads.
		bool is_waiting_for (name_t content);
		unsigned int get_interests_sent();
		#endif

		// Set if the client actively sends Interest packets.
		bool active;
		double lambda;

		double getLambda();

		// Only for Shot Noise simulations.
		double getScheduledReq(int);			// Return the # of scheduled requests for the specified popularity class.
		double getValidatedReq(int);			// Return the # of validated requests for the specified popularity class.

		bool stability; 		// Flag indicating steady state for the client.

	protected:
		virtual void initialize();
		virtual void handleMessage(cMessage *){;};		// It should be implemented by each specialized client.
		virtual void finish();

		virtual void handle_incoming_chunk(ccn_data *);
		virtual void request_file(int){;};				// It should be implemented by each specialized client.
		virtual void handle_timers(cMessage*);

		void send_interest(name_t, cnumber_t, int);
		void resend_interest(name_t,cnumber_t,int);

		// List of current downloads for a given file.
		multimap < name_t, download > current_downloads;

		#ifdef SEVERE_DEBUG
		unsigned int interests_sent;
		#endif

		// Single file statistics.
		client_stat_entry* client_stats;

		//double lambda;
		simtime_t check_time;

		// Vectors for Shot Noise statistics.
		vector<double> scheduledReq;		// Number of scheduled requests for each popularity class.
		vector<double> validatedReq;		// Number of validated requests for each popularity class.


    private:

		double tot_downloads; 		// Number of objects downloaded by the client.
		unsigned int tot_chunks;

		// Average statistics.
		simtime_t avg_time;
		double avg_distance;

		double RTT;
};
#endif
