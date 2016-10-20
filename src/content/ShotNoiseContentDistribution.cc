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
#include "ccnsim.h"
#include "ShotNoiseContentDistribution.h"
#include "content_distribution.h"
#include <error_handling.h>
#include <core_layer.h>
#include <fstream>

Register_Class(ShotNoiseContentDistribution);

//vector<zipf_distribution*> ShotNoiseContentDistribution::zipfClasses;
vector<zipf_sampled*> ShotNoiseContentDistribution::zipfClasses;

void ShotNoiseContentDistribution::initialize()
{
	cout << "Initialize SHOT NOISE content distribution...\tTime:\t" << SimTime() << "\n";

	classInfo = new vector<classInfoEntry> ();
	state_flags = new boost::dynamic_bitset<> ();
	state_times = new vector<simtime_t>	();

	const char *fileName = par("shot_noise_file").stringValue();	// Read the configuration file name.
	tOffMultFactor = par("toff_mult_factor");						// Read the Toff multiplicative factor.
	totalRequests = par("num_tot_req");
	numClients = getAncestorPar("num_clients");

	// Read configuration file.
	import_catalog_features(fileName);

	// Initialize both State Flags and Transition Times for each content.
	initialize_contents();

	content_distribution::initialize();

	#ifdef SEVERE_DEBUG
	initialized = true;
	#endif
}

void ShotNoiseContentDistribution::finish()
{
	delete classInfo;
	delete state_flags;
	delete state_times;
	for (uint32_t i=0; i<zipfClasses.size(); i++)
		delete zipfClasses[i];
}


/*
 *  Read parameters for the Shot Noise Model from the specified configuration file.
 *  We suppose that the configuration file contains classes in a decreasing order,
 *  meaning that the first line reports the characteristics of the most popular contents,
 *  while those of the least popular one are reported in the last line .
 *
 *  Parameters:
 *  - path: file name of the configuration file.
 */
void ShotNoiseContentDistribution::import_catalog_features(const char* path)
{
	ifstream fin;
	string line, temp;
	int line_pos = 0;
	int class_num = 0;
	int content_counter = 0;

	classInfoEntry* tempInfoEntry = new classInfoEntry (0,0,0,0);

    fin.open(path);
    if(!fin)
	{
		std::stringstream ermsg;
		ermsg<<"Shot Noise configuration file not present! Please check.";
		fin.close();
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
    else
    {
    	while(std::getline(fin, line))
    	{
    		if(class_num > 0)  // Skip the first line
    		{
    			boost::char_separator<char> sep("\t");
	    		boost::tokenizer<boost::char_separator<char> > tokens(line, sep);
	    		boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();

	    		++it;			// Skip the class ID feature.
	    		++line_pos;

	    		while(it != tokens.end())
	    		{
	    			switch(line_pos)
	    			{
	    			case 1:
	    				tempInfoEntry->ton = (simtime_t)(double)atof((*it).c_str());
	    				break;
	    			case 2:
	    				tempInfoEntry->eV = (double)atof((*it).c_str());
	    				break;
	    			case 3:
	    				tempInfoEntry->classAlpha = (double)atof((*it).c_str());
	    				break;
	    			case 4:
	    				tempInfoEntry->numContents = (long double)atof((*it).c_str());
	    				break;
	    			default:
	    				cout << "Error in parsing the Shot Noise model configuration file!\n";
	    			}
	    			++it;
	    			++line_pos;
	    		}
	    		tempInfoEntry->toff = (simtime_t)((double)tOffMultFactor*tempInfoEntry->ton);
	    		// Calculate the lambda of the class as lambda_i = (eV_i * numCont_i)/(Ton_i).
	    		tempInfoEntry->lambdaClass = (double)((tempInfoEntry->eV*tempInfoEntry->numContents) / SIMTIME_DBL(tempInfoEntry->ton));
	    		tempInfoEntry->lambdaClient = (double)(tempInfoEntry->lambdaClass / numClients);

	    		// Calculate the total request rate.
	    		totalLambda += tempInfoEntry->lambdaClass;
	    		totalContents += tempInfoEntry->numContents;
	    		tempInfoEntry->mostPopular = content_counter+1;
	    		tempInfoEntry->lessPopular = totalContents;
	    		content_counter = totalContents;

	    		classInfo->push_back(*tempInfoEntry);
    		}
    		++class_num;
    		line_pos=0;
    	}

    	cout << "Number of classes:\t" << classInfo->size() << "\n";
    	numOfClasses = classInfo->size();

	    for(uint32_t i=0; i<classInfo->size(); i++)
	    {
	    	cout << "Class #:\t" << i+1 << endl;
	    	cout << "Ton = " << classInfo->operator [](i).ton << " s\n";
	    	cout << "Toff = " << classInfo->operator [](i).toff << "\n";
	    	cout << "E[V] = " << classInfo->operator [](i).eV << "\n";
	    	cout << "Alpha = " << classInfo->operator [](i).classAlpha << "\n";
	    	cout << "# Contents = " << classInfo->operator [](i).numContents << "\n";
	    	cout << "Lambda = " << classInfo->operator [](i).lambdaClass << "\n";
	    	// Calculate the percentage of requests related to class 'i'.
	    	classInfo->operator [](i).percReq = (double)(classInfo->operator [](i).lambdaClass / totalLambda);
	    	cout << "Percentage of Requests = " << classInfo->operator [](i).percReq << "\n";
	    	cout << "Most popular content ID:\t" << classInfo->operator[](i).mostPopular << "\n";
	    	cout << "Less popular content ID:\t" << classInfo->operator[](i).lessPopular << "\n";
	    }

	    // Compute the steady-state time. It will be used by the "statistics" module to set the global time_steady.
	    steadySimTime = (double)((totalRequests / totalLambda));

	    cout << "Total Number of Contents:\t" << totalContents << "\n";
	    cout << "Total Lambda:\t" << totalLambda << "\n";
	    cout << "Number of Clients:\t" << numClients << "\n";
	    cout << "Computed steady state simulation time:\t" << steadySimTime << "\n";
    }
    delete tempInfoEntry;
}

/*
 *  Initialize State flags and the initial Transition Times for each content.
 */
void ShotNoiseContentDistribution::initialize_contents()
{
	// If Ton_i > steadySimTime for class 'i', the respective contents will be always ON.
	// Therefore, they will be not modeled as part of the ON-OFF process (i.e., state_flags
	// and state_times will not be initialized).

	// Calculate the number of contents worth to be simulated as ON-OFF.
	long double onOffContents = 0;
	for (uint32_t i=0; i<classInfo->size(); i++)
	{
		if (classInfo->operator [](i).ton < steadySimTime)
			onOffContents += classInfo->operator [](i).numContents;
		else
			break;
	}

	// Initialize the vector containing the State flags (initially all the contents are ON).
	state_flags->resize(onOffContents, true);

	// Initializing the vector containing the Transition Times.
	state_times->resize(onOffContents);

	int i=0;
	for(uint32_t j=1; j<=onOffContents; j++)
	{
		state_times->operator[](j-1) = (simtime_t)exponential(classInfo->operator [](i).ton);
		if(j==classInfo->operator [](i).lessPopular)
		{
			cout << "Class:\t" << i+1 << "\tContent:\t" << j << "\tTransition Time:\t" << state_times->operator[](j-1) << "\n";
			++i;  	// Use the parameters of the subsequent content class.
		}
	}

	// Initialize the structure containing the Zipf distributions for all the classes.
	for(int k=0; k<numOfClasses; k++)
	{
		//zipf_distribution* tempZipf = new zipf_distribution(classInfo->operator [](k).classAlpha, 0, classInfo->operator [](k).numContents, 1, k+1);
		/* zipf_sampled init: new zipf_sampled(a,b,c,d)
		 * where:
		 * 	a = number of contents in k-th class;
		 * 	b = zipf's exponent of k-th class;
		 * 	c = aggregate rate of that class (not actually used);
		 * 	d = downscaling factor for k-th class (default = 1).
		 */
		zipf_sampled* tempZipf = new zipf_sampled(classInfo->operator [](k).numContents, classInfo->operator [](k).classAlpha, classInfo->operator [](k).lambdaClass, 1);  // Zipf with rejection-inversion sampling
		//tempZipf->zipf_initialize();
		tempZipf->zipf_sampled_initialize();
		zipfClasses.push_back(tempZipf);
		//cout << "90th percentile of class " << k+1 << ":\t" << zipfClasses[k]->value(0.9) << endl;
	}
}

/*
 * 	Set the state of the specified content as "ON".
 *
 * 	Parameters:
 * 	- contentID: ID of the specified content.
 */
void ShotNoiseContentDistribution::set_state_flag(int contentID)
{
	if ((uint32_t)contentID < state_flags->size())
		state_flags->operator[](contentID) = 1;
	else
		cout << "Trying to set the state flag of a non existent content!\n";
}

/*
 * 	Set the state of the specified content as "OFF".
 *
 * 	Parameters:
 * 	- contentID: ID of the specified content.
 */
void ShotNoiseContentDistribution::unset_state_flag(int contentID)
{
	if ((uint32_t)contentID < state_flags->size())
		state_flags->operator[](contentID) = 0;
	else
		cout << "Trying to unset the state flag of a non existent content!\n";
}

/*
 * 	Check the state of the specified content.
 *
 * 	Parameters:
 * 	- contentID: ID of the specified content.
 */
bool ShotNoiseContentDistribution::check_state_flag(int contentID)
{
	bool active = false;
	if ((uint32_t)contentID < state_flags->size())
	{
		if(state_flags->operator[](contentID) == 1)
		active = true;
	}
	else
		cout << "Trying to check the state flag of a non existent content!\n";
	return active;

}

/*
 * 	Set the transition time of the specified content.
 *
 * 	Parameters:
 * 	- contentID: ID of the specified content.
 * 	- timeStamp: new transition time for contentID.
 */
void ShotNoiseContentDistribution::set_state_time(int contentID, simtime_t timeStamp)
{
	if ((uint32_t)contentID < state_flags->size())
		state_times->operator[](contentID) = timeStamp;
	else
		cout << "Trying to set the transition time of a non existent content!\n";
}

/*
 * 	Check the transition time of the specified content.
 *
 * 	Parameters:
 * 	- contentID: ID of the specified content.
 */
simtime_t ShotNoiseContentDistribution::get_state_time(int contentID)
{
	simtime_t temp;
	if ((uint32_t)contentID < state_flags->size())
		temp = state_times->operator[](contentID);
	else
		cout << "Trying to get the transition time of a non existent content!\n";
	return temp;
}

