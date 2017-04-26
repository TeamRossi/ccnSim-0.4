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
#include <cmath>
#include <fstream>
#include "limits.h"
#include "base_cache.h"
#include "core_layer.h"
#include "statistics.h"
#include "content_distribution.h"
#include "ccn_data_m.h"

#include "fix_policy.h"
#include "ideal_blind_policy.h"
#include "costaware_policy.h"
#include "ideal_costaware_policy.h"
#include "error_handling.h"

#include "two_lru_policy.h"
#include "two_ttl_policy.h"

#include "lcd_policy.h"
#include "never_policy.h"
#include "always_policy.h"
#include "decision_policy.h"
#include "betweenness_centrality.h"
#include "prob_cache.h"

#include "ccnsim.h"



// Initialization function
void base_cache::initialize()
{
	nodes = getAncestorPar("n");
    level = getAncestorPar("level");
    cache_size = par("C");
	decisor = NULL;

	// Retrieve replacement policy (i.e., TTL vs ALL)
	string forwStr = getParentModule()->par("RS");
	if(forwStr.compare("ttl_cache") == 0)		// We have to retrieve the Tc value of the node from the correspondent file
	{
		TC_PATH = par("tc_file");
		read_tc_value();
	}

    string decision_policy = par("DS");

    // *** Initialize the meta-caching strategy (i.e., cache decision policy) ***
	double target_acceptance_ratio;
	string target_acceptance_ratio_string;

    if (decision_policy.compare("lcd")==0)		// Leave Copy Down
    {
		decisor = new LCD();
    }
    else if (decision_policy.find("fix")==0)	// Fixed probabilistic.
    {
		target_acceptance_ratio_string = decision_policy.substr(3);
		target_acceptance_ratio = atof( target_acceptance_ratio_string.c_str() );
		decisor = new Fix(target_acceptance_ratio);
    }
	else if (decision_policy.find("ideal_blind")==0)	// Ideal Blind
	{
		decisor = new Ideal_blind(this);
    }
	else if (decision_policy.find("ideal_costaware")==0)	// Ideal costaware
	{
		target_acceptance_ratio = 0; // I don't need this parameter
		decisor = new Ideal_costaware(target_acceptance_ratio, this );
	}
	else if (decision_policy.find("costaware")==0)			// Costaware
	{
		target_acceptance_ratio_string = decision_policy.substr( strlen("costaware") );
		target_acceptance_ratio = atof(target_acceptance_ratio_string.c_str());
		decisor = new Costaware(target_acceptance_ratio);
	}
	else if (decision_policy.compare("two_ttl")==0)			// 2-TTL: read Tc value for name cache from file
	{
		TC_NAME_PATH = par("tc_name_file");
		read_tc_name_value();
		decisor = new Two_TTL(tc_name_node);
	}
	else if (decision_policy.compare("two_lru")==0)			// 2-LRU: set the size of the name cache
	{
		name_cache_size = par("NC");
		decisor = new Two_Lru(name_cache_size);
	}
	else if (decision_policy.find("btw")==0)				// Betweenness centrality
	{
		double db = getAncestorPar("betweenness");
		if (fabs(db - 1)<=0.001)
			error ("Node %i betweenness not defined.",getIndex());
		decisor = new Betweenness(db);
    }
	else if (decision_policy.find("prob_cache")==0)			// Probabilistic cache
	{
		decisor = new prob_cache(cache_size);
    }
	else if (decision_policy.find("never")==0)				// Never
	{
		decisor = new Never();
	}
    else if (decision_policy.compare("lce")==0 )			// Leave Copy Everywhere
	{
		decisor = new Always();
	}
	if (decisor==NULL)
	{
        std::stringstream ermsg; 
		ermsg<<"Decision policy \""<<decision_policy<<"\" incorrect";
	    severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

	// INPUT_CHECK
	if ( decision_policy.find("fix")==0 || (decision_policy.find("costaware")==0 && !decision_policy.find("ideal_costaware")== 0))
	{
		if ( strlen( target_acceptance_ratio_string.c_str() ) == 0 )
		{
			std::stringstream ermsg;
			ermsg<<"You forgot to insert a valid value of acceptance rate when "<<
				"specifying the decision policy. Right examples are fix0.01, costaware0.1";
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		if (target_acceptance_ratio <0)
		{
			std::stringstream ermsg;
			ermsg<<"target_acceptance_ratio "<<target_acceptance_ratio<<" is not valid. "<<
					"target_acceptance_ratio_string="<<target_acceptance_ratio_string<<
					"; decision_policy="<<decision_policy;
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
	}

    // Average Cache statistics
    miss = 0;
    hit = 0;

	decision_yes = decision_no = 0;

    //--Per file
	//**mt** DISABLED
	//cache_stats = new cache_stat_entry[__file_bulk + 1];

	#ifdef SEVERE_DEBUG
	initialized = true;
	#endif
}

void base_cache::finish(){

    char name [30];
    sprintf ( name, "p_hit[%d]", getIndex());
    recordScalar (name, hit * 1./(hit+miss));		// Record average hit rate.


    sprintf ( name, "hits[%d]", getIndex());		// Record number of hits.
    recordScalar (name, hit );


    sprintf ( name, "misses[%d]", getIndex());		// Record number of misses.
    recordScalar (name, miss);

    sprintf ( name, "decision_yes[%d]", getIndex());
    recordScalar (name, decision_yes);

    sprintf ( name, "decision_no[%d]", getIndex());
    recordScalar (name, decision_no);

    sprintf ( name, "decision_ratio[%d]", getIndex());
	double decision_ratio = (decision_yes + decision_no == 0 ) ?
			0 : (double)decision_yes / (decision_yes + decision_no) ; 
    recordScalar (name, decision_ratio);

	decisor->finish(getIndex(), this);

    //Per file hit rate
    //sprintf ( name, "hit_node[%d]", getIndex());
    //cOutVector hit_vector(name);
    //for (uint32_t f = 1; f <= __file_bulk; f++)
    //    hit_vector.recordWithTimestamp(f, cache_stats[f].rate() );

	delete decisor;

	//**mt** DISABLED
	//delete [] cache_stats;
}



/*
 * 	Storage handling of a received Data packet. The storage decision depends on the meta-caching strategy.
 *
 * 	Parameters:
 * 		- in: received Data packet.
 */
void base_cache::store(cMessage *in)
{
	if (cache_size ==0)		// The cache has Size=0.
	{
		after_discarding_data();
		return;
	}

    if (decisor->data_to_cache((ccn_data*)in )) 	// The decision is based on the meta-caching strategy.
    {
		decision_yes++;
		data_store( ( (ccn_data* ) in )->getChunk() ); // Store the received chunk inside the local cache. It is implemented
													   // by each derived class according to the chosen replacement policy.
		decisor->after_insertion_action();
	}
	//<aa>
	else after_discarding_data();
	//</aa>
}

//<aa>
// Call it when you decide not to store an incoming data pkt
void base_cache::after_discarding_data()
{
	decision_no++;
}
//</aa>

/*
 *    Storage handling of the received content ID inside the name cache (only with 2-LRU meta-caching).
 *
 *    Parameters:
 *    	 - elem: content ID to be stored.
 */
void base_cache::store_name(chunk_t elem)
{
    if (cache_size ==0)
    {
		std::stringstream ermsg;
		ermsg<<" ALLERT! The size of the name cache is set to 0! Please check.";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}

    data_store(elem);  // Store the content ID inside the Name Cache.
}

void base_cache::store_name_ttl(chunk_t elem)
{
    data_store(elem);  // Store the content ID inside the TTL Name Cache.
}

/*
 * 		Lookup function. The ID of the received Interest is looked up inside the local cache.
 * 		Hit/Miss statistics are gathered.
 *
 * 		Parameters:
 * 			- chunk: content ID of the received Interest.
 */
bool base_cache::lookup(chunk_t chunk )
{
    bool found = false;
    //name_t name = __id(chunk);

    if (data_lookup(chunk))		// The requested content is cached locally.
    {
    	hit++;
    	found = true;

    	//Per file cache statistics(hit)
    	//**mt** DISABLED
    	//if (name <= __file_bulk)
    	//    cache_stats[name].hit++;

    }
    else		// The local cache does not contain the requested content.
    {
        found = false;
		miss++;

		//Per file cache statistics(miss)
		//**mt** DISABLED
		//if ( name <= __file_bulk )
		//	cache_stats[name].miss++;
    }
    return found;
}

double base_cache::get_tc()
{
	double tc = get_tc_node();
	return tc;
}
/*
 * 	Lookup function without hit/miss statistics (used only with 2-LRU meta-caching to lookup the content ID inside the name cache).
 *
 * 	Parameters:
 * 		- chunk: content ID to be looked up.
 */
bool base_cache::lookup_name(chunk_t chunk )
{
    bool found = false;
    //name_t name = __id(chunk);

    if (data_lookup(chunk))			// The content ID is present inside the name cache.
    	found = true;
    else
    	found = false;

    return found;
}


bool base_cache::fake_lookup(chunk_t chunk)
{
    return data_lookup(chunk);
}

/*
 * 	Reset all the statistics.
 */
void base_cache::clear_stat()
{
    hit = miss = 0;

	decision_yes = decision_no = 0;

	//**mt** DISABLED
    //delete [] cache_stats;
    //cache_stats = new cache_stat_entry[__file_bulk+1];
}

/*
 *	Set the size of the local cache in terms of number of objects.
 *
 *	Parameters:
 *		- cSize: size of the cache.
 */
void base_cache::set_size(uint32_t cSize)
{
	cache_size = cSize;
}

uint32_t base_cache::get_decision_yes()
{
	return decision_yes;	
}

uint32_t base_cache::get_decision_no()
{
	return decision_no;
}

void base_cache::set_decision_yes(uint32_t n)
{
	decision_yes = n;
}

void base_cache::set_decision_no(uint32_t n)
{
	decision_no = n;
}

DecisionPolicy* base_cache::get_decisor() const
{
	return decisor;
}

#ifdef SEVERE_DEBUG
bool base_cache::is_initialized(){
	return initialized;
}
#endif

void base_cache::read_tc_value()
{
	ifstream fin_tc;

	string line;
	//int line_num = 0;

	fin_tc.open(TC_PATH);
	if(!fin_tc)
	{
		//std::cout << "\n TC File does NOT exist!\n";
		std::cout << "\n TC File does NOT exist! TC value generated RANDOMLY\n";
	   	fin_tc.close();
	   	//exit(0);
	   	std::random_device rd;
	   	std::mt19937 eng(rd()); // seed the generator
	   	std::uniform_int_distribution<> distr(cache_size/100, cache_size); // define the range
	   	tc_node = (double)distr(eng);
	   	cout << "\t TC = " << tc_node << " s" << endl;
	}
	else
	{
		int line_num = getIndex();     // The line numbre corresponds to the node ID.
		string line;
		if (line_num == 0)
		{
			getline(fin_tc, line);
			tc_node = (double) atof(line.c_str());
			ASSERT2(tc_node > 0 && tc_node < DBL_MAX, "Tc values should be bigger than 0 and smaller than DBL_MAX! Please check your TC file.\n");
		}
		else
		{
			for(int i = 0; i < line_num; ++i)
				getline(fin_tc, line);
			// Take the 'line_num-th' line
			getline(fin_tc, line);
			tc_node = (double) atof(line.c_str());
			ASSERT2(tc_node > 0 && tc_node < DBL_MAX, "Tc values should be bigger than 0 and smaller than DBL_MAX! Please check your TC file.\n");
		}
		fin_tc.close();

		//cout << "NODE # " << getIndex() << " has TC = " << tc_node << " s" << endl;
		cout << "\t TC = " << tc_node << " s" << endl;
	}
}


void base_cache::read_tc_name_value()
{
	ifstream fin_tc;

	string line;
	//int line_num = 0;

	fin_tc.open(TC_NAME_PATH);
	if(!fin_tc)
	{
		//std::cout << "\n TC File does NOT exist!\n";
		std::cout << "\n TC NAME File does NOT exist! TC value generated RANDOMLY\n";
	   	fin_tc.close();
	   	//exit(0);
	   	std::random_device rd;
	   	std::mt19937 eng(rd()); // seed the generator
	   	std::uniform_int_distribution<> distr(cache_size/100, cache_size/10); // define the range
	   	tc_node = (double)distr(eng);
	   	cout << "\t TC = " << tc_node << " s" << endl;
	}
	else
	{
		int line_num = getIndex();     // The line numbre corresponds to the node ID.
		string line;
		if (line_num == 0)
		{
			getline(fin_tc, line);
			tc_name_node = (double) atof(line.c_str());
			ASSERT2(tc_node > 0 && tc_node < DBL_MAX, "Tc values should be bigger than 0 and smaller than DBL_MAX! Please check your TC file.\n");
		}
		else
		{
			for(int i = 0; i < line_num; ++i)
				getline(fin_tc, line);
			// Take the 'line_num-th' line
			getline(fin_tc, line);
			tc_name_node = (double) atof(line.c_str());
			ASSERT2(tc_node > 0 && tc_node < DBL_MAX, "Tc values should be bigger than 0 and smaller than DBL_MAX! Please check your TC file.\n");
		}
		fin_tc.close();

		cout << "NODE # " << getIndex() << " has TC NAME = " << tc_name_node << " s" << endl;
	}
}
