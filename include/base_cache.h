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
#ifndef B_CACHE_H_
#define B_CACHE_H_


#include "ccnsim.h"
class DecisionPolicy;



//	Base cache class. It provides the interface to implement the different replacement policy that could characterize a cache.

// Struct containing cache statistics.
struct cache_stat_entry{
    unsigned int  miss; 	// Total number of misses
    unsigned int  hit; 		// Total number of hit
    cache_stat_entry():miss(0),hit(0){;}
    double rate(){ return hit *1./(hit+miss);} 	// Return the hit rate of the cache.
};

class base_cache : public abstract_node{
    friend class statistics;
    protected:

		void initialize();
		void handleMessage (cMessage *){;}
		void finish();

		// Interface function (will be specialized by the derived classes)
		virtual void data_store (chunk_t) = 0; 
		virtual bool data_lookup(chunk_t) = 0;
		virtual double get_tc_node() = 0;



		#ifdef SEVERE_DEBUG
		bool initialized;
		#endif
	
		double tc_node;				// Used in reading Tc from file
		double tc_name_node; 		// (if 2-LRU) Used in reading Tc of name cache from file
		void read_tc_value();
		void read_tc_name_value();
		virtual void ttl_cache_check(){;}

		int cache_size;

    public:
		#ifdef SEVERE_DEBUG
		base_cache():abstract_node(){initialized=false; };
		#endif

		virtual void dump(){cout<<"Not implemented"<<endl;}

		virtual void flush(){cout<<"Not implemented"<<endl;}

		uint32_t get_size() { return cache_size; }
		void set_size(uint32_t);

		virtual bool fake_lookup(chunk_t);
		bool lookup(chunk_t);

		// Lookup without hit/miss statistics (used with the 2-LRU meta-caching strategy to lookup the name cache)
		bool lookup_name(chunk_t);

		void store (cMessage *);
		void store_name(chunk_t);    // Store the content ID inside the name cache (only with 2-LRU meta-caching).
		void store_name_ttl(chunk_t);    // Store the content ID inside the ttl name cache (only with 2-LRU meta-caching).

		void clear_stat();

		virtual uint32_t get_decision_yes();
		virtual uint32_t get_decision_no();
		virtual void set_decision_yes(uint32_t n);
		virtual void set_decision_no(uint32_t n);
		virtual DecisionPolicy* get_decisor() const;
		virtual void after_discarding_data(); // Call it when you decide not to store an incoming data pkt

		double get_tc();

		#ifdef SEVERE_DEBUG
		virtual bool is_initialized();
		#endif

		virtual bool full() = 0;

		uint32_t decision_yes;

		bool stability;

    private:
		int name_cache_size;   		// Size of the name cache expressed in number of content IDs (only with 2-LRU meta-caching).
		int nodes;
		int level;

		DecisionPolicy *decisor;

		// Average statistics
		uint32_t miss;
		uint32_t hit;

		//uint32_t decision_yes;
		uint32_t decision_no;


		// Per file statistics
		cache_stat_entry *cache_stats;

		const char* TC_PATH;
		const char* TC_NAME_PATH;
};

#endif
