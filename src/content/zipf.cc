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
 *    ccnsim@listes.telecom-paristech.fr
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
#include "zipf.h"
#include <iostream>
#include <cmath>

using namespace std;

/*
 * 		Initialization of the Zipf's CDF according to the specified percentile for the catalog aggregation.
 */
void zipf_distribution::zipf_initialize(){

	// Return if the cdf has been already initialized.
    if (cdfZipf.size() != 0)
    	return;

    double c = 0;
    double num = 0;

    cout<<"Initializing Zipf distribution of Class # "<< class_num << " ..."<<  endl;

    for (int i=1; i<=F; i++)
    {
    	c += (1.0 / pow(i+q,alpha));
    }
    c = 1.0 / c;

    normalization_constant = c;

    cout << "Chosen percentile:\t" << perc_aggr << "\n";

    if (perc_aggr == 1.0)  		// The catalog is not aggregated.
    {
    	cdfZipf.resize(F+1);
    	cdfZipf[0] = -1;

    	for (int i=1; i<=F; i++)
    	{
    		num += (1.0 / pow(i+q,alpha));
    		cdfZipf[i] = num*c;
    	}
    }
    else if (perc_aggr > 0 && perc_aggr < 1.0)  // Catalog aggregation after the perc_aggr-th percentile.
    {
    	double temp = 0;
    	for (int i=1; i<=F; i++)
    	{
    		num += (1.0 / pow(i+q,alpha));
    		temp = num*c;
    		if (temp >= perc_aggr)
    		{
    			ID_aggr = i;
    			num=0;
    			cout << "\nContent ID of correspondent to the chosen percentile:\t" << ID_aggr << "\n";
    			cdfZipf.resize(ID_aggr+2);
    			cdfZipf[0] = -1;
    			for (uint32_t j=1; j<=ID_aggr+1; j++)
    			{
    				num += (1.0 / pow(j+q,alpha));
    				if(j<ID_aggr+1)
    					cdfZipf[j] = num*c;
    				else
    					cdfZipf[j] = 1;
    			}
    			cout << "Size of the cdf vector:\t" << cdfZipf.size() << "\n";
    			cout << "Last two values of the cdf:\t" << cdfZipf[ID_aggr] << "\t" << cdfZipf[ID_aggr+1] << "\n";
    			break;
    		}
    	}
    }
    else
    {
    	cout << "Insert a percentile value < 1!";
    }

    cout<<"Zipf initialization completed"<<endl;
}

double zipf_distribution::get_normalization_constant(){
	return normalization_constant;
}

unsigned int zipf_distribution::get_ID_aggr()
{
	return ID_aggr;
}

unsigned int zipf_distribution::get_catalog_card()
{
	return F;
}

double zipf_distribution::get_perc_aggr()
{
	return perc_aggr;
}

/*
 * 	Binary search within the Zipf cdf.
 */
unsigned int zipf_distribution::value(double p){

    unsigned int upper, lower,atry, last_try;
    
    lower = -1;
    upper = cdfZipf.size()-1;
    atry = -1;
    last_try = -1;

    // Binary search to find the nearest value y whose cdf is x
    while (1){
		atry = floor((lower+upper+1)/2);

		if (last_try == atry)
			break;

		if (cdfZipf[atry] >= p)
			upper=atry;
		else
			lower = atry-1;

		last_try = atry;
    }

    return upper;
}

double zipf_distribution::get_alpha()
{
	return alpha;
}
