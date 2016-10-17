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

//<aa>
#include "error_handling.h"
#include <sstream>
#include<iostream>
#include <cstdlib>
#include <cmath> // for using fabs
#include <limits> // for using numeric_limits

using namespace std;

void generic_message(const char* source_file_name, int code_line, const char* tag,
			const char* message){
	cout << source_file_name << " " << code_line <<": "<< tag<<" : " 
			<< message << endl;
}

void severe_error(const char* source_file_name, int code_line, 
				const char* error_message)
{
	generic_message(source_file_name, code_line, "ERROR", error_message);
	exit(3);
}

void debug_message(const char* source_file_name, int code_line, 
		const char* error_message)
{
	generic_message(source_file_name, code_line, "DEBUG", error_message);
}

void severe_error(const char* source_file_name, int code_line, 
				std::stringstream error_message)
{
	severe_error(source_file_name, code_line, error_message.str().c_str() );
	exit(3);
}

void debug_message(const char* source_file_name, int code_line, 
		std::stringstream error_message)
{
	debug_message(source_file_name, code_line, error_message.str().c_str() );
}

bool double_equality( double a, double b)
{
	if ( fabs(a-b) > std::numeric_limits<double>::epsilon() * 1e2 )
		return false;
	return true;
}


//</aa>
