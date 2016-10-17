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
#ifndef INTEREST_H_
#define INTEREST_H_
#include "ccn_interest_m.h"
#include "content_distribution.h"
#include "ccnsim.h"
#include <deque>
#include <algorithm>

//<aa>
#include "error_handling.h"
//</aa>

class ccn_interest: public ccn_interest_Base{
protected:

	std::deque<int> path;

public:
	ccn_interest(const char *name=NULL, int kind=0):ccn_interest_Base(name,kind){;}
	ccn_interest(const ccn_interest_Base& other) : ccn_interest_Base(other.getName() ){ operator=(other); }
	ccn_interest& operator=(const ccn_interest& other){
		if (&other==this) return *this;
		ccn_interest_Base::operator=(other);
		path = other.path;
		return *this;
	}
	virtual ccn_interest *dup() const {return new ccn_interest(*this);}
	virtual	void setPathArraySize(unsigned int size){;}
	virtual unsigned int getPathArraySize() const{return path.size();}
	virtual int getPath(unsigned int k) const{return path[k];}
	virtual void setPath(unsigned int k, int path_var){path[k] = path_var;}
	virtual void setPath(std::deque<int> new_path){path = new_path;}
	virtual void pushPath (int path_var){path.push_back( path_var );}
	virtual bool find(int index){return std::find(path.begin(),path.end(),index)!=path.end();}

	virtual int popPath(){
	    int front=path.front();
	    path.pop_front();
	    return front;
	}

	virtual name_t get_name(){return __id(chunk_var);}
	virtual name_t get_chunk_number(){return __chunk(chunk_var);}


	virtual vector<int> get_repos()
	{
	    repo_t repo;
	    vector<int> repos;
	    int i;

	    repo = __repo(__id(chunk_var));
	    i = 0;

	    while (repo)
	    {
			if (repo & 1) 
				repos.push_back(content_distribution::repositories[i]);
			repo >>= 1;
			i++;
	    }

		//<aa>
		#ifdef SEVERE_DEBUG
		if (repos.size() ==0){
			std::stringstream ermsg; 
			ermsg<<"There are 0 repositories for interest "<<__id(chunk_var);
			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
		}
		#endif
		//</aa>

	    return repos;
	}

	
};
Register_Class(ccn_interest);
#endif 
