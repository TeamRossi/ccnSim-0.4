#ifndef __CCNSIM_H___
#define __CCNSIM_H___

#include <vector>
#include <omnetpp.h>

//<aa>
// If SEVERE_DEBUG is enabled, overabundant checks will be performed in order to avoid inconsistent
// state. It will slow down the simulation (for example a run of 10 s disabling SEVERE_DEBUG
// may take 14 s when enabling it), but if you are considerably modifying 
// ccnSim source code, it is advisable to 
// enable it for some runs, just to check that there are not erroneous things happening.
// This does not affect in any way the results.
//#define SEVERE_DEBUG

#define UNDEFINED_VALUE -1
//</aa>

//System packets
#define CCN_I 100   //ccn interest 
#define CCN_D 200   //ccn data 
#define GHOST 5    //ghost interest

//Clients timers
#define ARRIVAL 300 //arrival of a request
#define ARRIVAL_TTL 350 //arrival of a request for the ModelGraft scenario
#define TIMER 400   //arrival of a request 

//Statistics timers
#define FULL_CHECK 2000
#define STABLE_CHECK 3000
#define END 4000

//Strategy Layer messages (mostly for link failure)
#define FAILURE 700
#define RECOVERY 800
#define NEW_ROUTES 900
#define NEW_MODEL 1000

//Core Layer Timer (Link Load Check)
#define LOAD_CHECK 1010

//Base Cache Timer (Expire Check)
#define TTL_CHECK 1500
#define TWO_TTL_CHECK 1501
#define CONT_SHIFT 50

//Client_Window (Window TO)
#define WinTO 1020

//Typedefs
//Catalogs fields
typedef unsigned int info_t; //representation for a catalog  entry [size|repos]
typedef unsigned short filesize_t; //representation for the size part within the catalog entry

//<aa>  Each repo_t variable is used to indicate the set of repositories where
//              a certain content is stored. A repo_t variable must be interpreted as a
//              binary string. For a certain content, the i-th bit is 1 if and only if the
//              content is stored in the i-th repository.
//</aa>
typedef unsigned short repo_t; //representation for the repository part within the catalog entry

typedef unsigned long interface_t; //representation of a PIT entry (containing interface information)

//Chunk fields
typedef unsigned long long  chunk_t; //representation for any chunk flying within the system. It represents a pair [name|number]typedef unsigned int cnumber_t; //represents the number part of the chunk
typedef unsigned int cnumber_t; //represents the number part of the chunk
typedef unsigned long name_t; //represents the name part of the chunk

#include "client.h"
//Useful data structure. Use that instead of cSimpleModule, when you deal with caches, strategy_layers, and core_layers
class abstract_node: public cSimpleModule{
    public:
        abstract_node():cSimpleModule(){;}

        virtual cModule *__find_sibling(std::string mod_name){
            return getParentModule()->getModuleByPath(mod_name.c_str());
        }

        virtual int __get_outer_interfaces(){
            return getParentModule()->gateSize("face");
        }

        //<aa> Check whether the module attached to that interface is a client or not</aa>
        bool __check_client(int interface){
            client *c;
            bool check= false;
            c = dynamic_cast<client *>
                        (getParentModule()->gate("face$o",interface)->getNextGate()->getOwnerModule());
            if (c)
                        check=true;
            return check;
        }

        bool __check_client_active(int interface){
                    client *c = new client();
                    bool check= false;
                    c = dynamic_cast<client *>
                                (getParentModule()->gate("face$o",interface)->getNextGate()->getOwnerModule());
                    if (c->is_active())
                                check=true;
                    delete c;
                    return check;
                }

        //<aa>  If there is a client attached to the specified interface, it will be returned.
        //              Otherwise a null pointer will be returned
        cSimpleModule* __get_attached_client(int interface)
        {
            client *c = dynamic_cast<client *>
                        (getParentModule()->gate("face$o",interface)->getNextGate()->getOwnerModule());
            return c;
        }
        //</aa>

        virtual int getIndex()
        {
            return getParentModule()->getIndex();
        }

};
//Macros
//--------------
//Chunk handling
//--------------
//Basically a chunk is a 64-bit integer composed by two parts: the chunk_number, and the chunk id
//<aa> The first 32 bits indicate the chunk_id, the other 32 bits indicate the chunk_number </aa>
#define NUMBER_OFFSET   32
#define ID_OFFSET        0

//Bitmasks
#define CHUNK_MSK ( (std::uint64_t) 0xFFFFFFFF << NUMBER_OFFSET)
#define ID_MSK    ( (std::uint64_t) 0xFFFFFFFF << ID_OFFSET )

//Macros
#define __chunk(h) ( ( h & CHUNK_MSK )  >> NUMBER_OFFSET )// get chunk number
#define __id(h)    ( ( h & ID_MSK )     >> ID_OFFSET) //get chunk id

#define __schunk(h,c) h = ( (h & ~CHUNK_MSK) | ( (std::uint64_t ) c  << NUMBER_OFFSET)) //set chunk number
#define __sid(h,id)   h = ( (h & ~ ID_MSK)   | ( (std::uint64_t ) id << ID_OFFSET)) //set chunk id

inline chunk_t next_chunk (chunk_t c){

    cnumber_t n = __chunk(c);
    __schunk(c, (n+1) );
    return c;

}




//--------------
//Catalog handling
//--------------
//The catalog is a huge array of file entries. Within each entry is an 
//information field 32-bits long. These 32 bits are composed by:
//[file_size|repositories]
//
//
#define SIZE_OFFSET     16
#define REPO_OFFSET     0

//Bitmasks
#define REPO_MSK (0xFFFF << REPO_OFFSET)
#define SIZE_MSK (0xFFFF << SIZE_OFFSET)

#define __info(f) ( content_distribution::catalog[f].info) //retrieve info about the given content 

#define __size(f)  ( (__info(f) & SIZE_MSK) >> SIZE_OFFSET ) //set the size of a given file
#define __repo(f)  ( (__info(f) & REPO_MSK) >> REPO_OFFSET )

#define __ssize(f,s) ( __info(f) = (__info(f) & ~SIZE_MSK ) | s << SIZE_OFFSET )
#define __srepo(f,r) ( __info(f) = (__info(f) & ~REPO_MSK ) | r << REPO_OFFSET )

//<aa>
// File statistics. Doing statistics for all files would be tremendously
// slow for huge catalog size, and at the same time quite useless
// (statistics for the 12345234th file are not so meaningful at all)
// Therefore, we compute statistics only for the first __file_bulk files
// (see client.cc)
// </aa>
#define __file_bulk (content_distribution::perfile_bulk + 1)




//-----------
//PIT handling 
//-----------
//Each entry within a PIT contains a field that indicates through
//which interface the back-coming interest should be sent
//<aa>  This field is f, a bit string that contains a 1 in the i-th place
// if the i-th is set </aa>
//
#define __sface(f,b)  ( f = f | ((interface_t)1 << b ) ) //Set the b-th bit
#define __uface(f,b)  ( f = f & ~((interface_t)1<<b) ) //Unset the b-th bit
#define __face(f,b)   ( f & ((interface_t)1<<b) ) //Check the b-th bit
//
//
//

//---------------------------
//Statistics utility functions
//---------------------------
//Calculate the average of a vector of elements
template <class T>
double average(std::vector<T> v){
    T s =(T) 0;
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++)
        s += *i;
    return (double) s * 1./v.size();
}


template <class T>
double variance(std::vector<T> v){

    T s = (T) 0;
    T ss = (T) 0;
    unsigned int N = v.size();

    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++){
                s += *i;
                ss += (*i)*(*i);
    }
    return (double)(1./(N-1)) * (ss - (s*s)*(1./N));
}
#endif
