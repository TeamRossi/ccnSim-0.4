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
#include "random_cache.h"
Register_Class (random_cache);



void random_cache::initialize(){
    base_cache::initialize();
}

void random_cache::data_store(chunk_t chunk){
    cache[chunk] = true;
    if (deq.size() == get_size() ){
        //Replacing a random element
        unsigned int pos = intrand(  deq.size() );
        chunk_t toErase = deq.at(pos);

        deq.at(pos) = chunk;
        cache.erase(toErase);

    } else
        deq.push_back(chunk);

}


bool random_cache::data_lookup(chunk_t chunk){
    bool ret = (cache.find(chunk) != cache.end());
    return ret;

}

bool random_cache::full(){
    return (deq.size()==get_size());
}

/*Deprecated: used in order to fill up caches with random chunks*/
bool random_cache::warmup(){
    int C = get_size();
    int k = getIndex();
    uint64_t chunk=0;

    cout<<"Starting warmup..."<<endl;
    for (int i = k*C+1; i<=(k+1)*C; i++){
	__sid(chunk,i);
	cache[chunk] = true;
	//cout<<"cache index "<<k<<" storing "<<i<<endl;
	//deq.push_back(chunk);
    }

    //vector<file> &catalog = content_distribution::catalog;
    //uint32_t s=0, 
    //         i=1,
    //         F = catalog.size() - 1;
    //uint64_t chunk;
    //file file;

    ////Size represents the chache size expressed in chunks
    //cout<<"Starting warmup..."<<endl;

    //chunk = 0;
    //while (s < get_size() && i <= F){
    //    __sid(chunk,i);
    //    cache[chunk] = true;
    //    deq.push_back(chunk);
    //    i++;
    //    s++;
    //}

    ////Return if the cache has been filled or not
    //if (i>F)
    //    return false;

    cout<<"[OK] Cache full"<<endl;
    return true;



}
