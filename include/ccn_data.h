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
#ifndef DATA_H_
#define DATA_H_
#include "ccnsim.h"
#include "ccn_data_m.h"
#include "content_distribution.h"
#include <deque>
#include <algorithm>

class ccn_data: public ccn_data_Base{
protected:

	std::deque<int> path;

public:
	ccn_data(const char *name=NULL, int kind=0):ccn_data_Base(name,kind){;}
	ccn_data(const ccn_data_Base& other) : ccn_data_Base(other.getName() ){ operator=(other); }
	ccn_data& operator=(const ccn_data& other){
		if (&other==this) return *this;
		ccn_data_Base::operator=(other);
		path = other.path;
		return *this;
	}
	virtual ccn_data *dup() const {return new ccn_data(*this);}

	//Utility functions which return 
	//different header fields of the packet
	uint32_t get_name(){ return __id(chunk_var);}
	uint32_t get_chunk_num(){ return __chunk(chunk_var);}
	uint64_t get_next_chunk(){ return next_chunk(chunk_var); }

	uint32_t get_size(){return __size(__id(chunk_var)); }
	
};
Register_Class(ccn_data);
#endif 
