//=========================================================================
//  CTOPOLOGY.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Member functions of
//     cTopology : network topology to find shortest paths etc.
//
//  Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2008 Andras Varga
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <deque>
#include <algorithm>
#include <sstream>
#include "ctopology.h"
#include "cpar.h"
#include "globals.h"
#include "cexception.h"
#include "cproperty.h"
#include "patternmatcher.h"

#ifdef WITH_PARSIM
#include "ccommbuffer.h"
#endif

USING_NAMESPACE

Register_Class(cTopology);


cTopology::LinkIn *cTopology::Node::getLinkIn(int i)
{
    if (i<0 || i>=num_in_links)
        throw cRuntimeError("cTopology::Node::getLinkIn: invalid link index %d", i);
    return (cTopology::LinkIn *)in_links[i];
}

cTopology::LinkOut *cTopology::Node::getLinkOut(int i)
{
    if (i<0 || i>=num_out_links)
        throw cRuntimeError("cTopology::Node::getLinkOut: invalid index %d", i);
    return (cTopology::LinkOut *)(out_links+i);
}

//----

cTopology::cTopology(const char *name) : cOwnedObject(name)
{
    num_nodes = 0;
    nodev = NULL;
}

cTopology::cTopology(const cTopology& topo) : cOwnedObject()
{
    nodev = NULL;
    setName(topo.getName());
    cTopology::operator=(topo);
}

cTopology::~cTopology()
{
    clear();
}

std::string cTopology::info() const
{
    std::stringstream out;
    out << "n=" << num_nodes;
    return out.str();
}

void cTopology::parsimPack(cCommBuffer *buffer)
{
    throw cRuntimeError(this,"parsimPack() not implemented");
}

void cTopology::parsimUnpack(cCommBuffer *buffer)
{
    throw cRuntimeError(this,"parsimUnpack() not implemented");
}

cTopology& cTopology::operator=(const cTopology&)
{
    throw cRuntimeError(this,"operator= not implemented yet");
}

void cTopology::clear()
{
    for (int i=0; i<num_nodes; i++)
    {
	//nodev[i].P.clear();
        delete [] nodev[i].in_links;
        delete [] nodev[i].out_links;
    }
    delete [] nodev;

    num_nodes = 0;
    nodev = NULL;
}

//---

static bool selectByModulePath(cModule *mod, void *data)
{
    // actually, this is selectByModuleFullPathPattern()
    const std::vector<std::string>& v = *(const std::vector<std::string> *)data;
    std::string path = mod->getFullPath();
    for (int i=0; i<(int)v.size(); i++)
        if (PatternMatcher(v[i].c_str(), true, true, true).matches(path.c_str()))
            return true;
    return false;
}

static bool selectByNedTypeName(cModule *mod, void *data)
{
    const std::vector<std::string>& v = *(const std::vector<std::string> *)data;
    return std::find(v.begin(), v.end(), mod->getNedTypeName()) != v.end();
}

static bool selectByProperty(cModule *mod, void *data)
{
    struct ParamData {const char *name; const char *value;};
    ParamData *d = (ParamData *)data;
    cProperty *prop = mod->getProperties()->get(d->name);
    if (!prop)
        return false;
    const char *value = prop->getValue(cProperty::DEFAULTKEY, 0);
    if (d->value)
        return opp_strcmp(value, d->value)==0;
    else
        return opp_strcmp(value, "false")!=0;
}

static bool selectByParameter(cModule *mod, void *data)
{
    struct PropertyData{const char *name; const char *value;};
    PropertyData *d = (PropertyData *)data;
    return mod->hasPar(d->name) && (d->value==NULL || mod->par(d->name).str()==std::string(d->value));
}

//---

void cTopology::extractByModulePath(const std::vector<std::string>& fullPathPatterns)
{
    extractFromNetwork(selectByModulePath, (void *)&fullPathPatterns);
}

void cTopology::extractByNedTypeName(const std::vector<std::string>& nedTypeNames)
{
    extractFromNetwork(selectByNedTypeName, (void *)&nedTypeNames);
}

void cTopology::extractByProperty(const char *propertyName, const char *value)
{
    struct {const char *name; const char *value;} data = {propertyName, value};
    extractFromNetwork(selectByProperty, (void *)&data);
}

void cTopology::extractByParameter(const char *paramName, const char *paramValue)
{
    struct {const char *name; const char *value;} data = {paramName, paramValue};
    extractFromNetwork(selectByParameter, (void *)&data);
}

//---

static bool selectByPredicate(cModule *mod, void *data)
{
    cTopology::Predicate *predicate = (cTopology::Predicate *)data;
    return predicate->matches(mod);
}

void cTopology::extractFromNetwork(Predicate *predicate)
{
    extractFromNetwork(selectByPredicate, (void *)predicate);
}

void cTopology::extractFromNetwork(bool (*selfunc)(cModule *,void *), void *data)
{
    clear();

    Node *temp_nodev = new Node[simulation.getLastModuleId()];

    // Loop through all modules and find those which have the required
    // parameter with the (optionally) required value.
    int k=0;
    for (int mod_id=0; mod_id<=simulation.getLastModuleId(); mod_id++)
    {
        cModule *mod = simulation.getModule(mod_id);
        if (mod && selfunc(mod,data))
        {
            // ith module is OK, insert into nodev[]
            temp_nodev[k].module_id = mod_id;
            temp_nodev[k].wgt = 0;
            temp_nodev[k].enabl = true;

            // init auxiliary variables
            temp_nodev[k].known = 0;
            temp_nodev[k].dist = INFINITY;
            temp_nodev[k].out_path = NULL;

            // create in_links[] arrays (big enough...)
            temp_nodev[k].num_in_links = 0;
            temp_nodev[k].in_links = new cTopology::Link *[mod->gateCount()];

            k++;
        }
    }
    num_nodes = k;

    nodev = new Node[num_nodes];
    memcpy(nodev, temp_nodev, num_nodes*sizeof(Node));
    delete [] temp_nodev;

    // Discover out neighbors too.
    for (int k=0; k<num_nodes; k++)
    {
        // Loop through all its gates and find those which come
        // from or go to modules included in the topology.

        cModule *mod = simulation.getModule(nodev[k].module_id);
        cTopology::Link *temp_out_links = new cTopology::Link[mod->gateCount()];

        int n_out=0;
        for (cModule::GateIterator i(mod); !i.end(); i++)
        {
            cGate *gate = i();
            if (gate->getType()!=cGate::OUTPUT)
                continue;

            // follow path
            cGate *src_gate = gate;
            do {
                gate = gate->getNextGate();
            }
            while(gate && !selfunc(gate->getOwnerModule(),data));

            // if we arrived at a module in the topology, record it.
            if (gate)
            {
                temp_out_links[n_out].src_node = nodev+k;
                temp_out_links[n_out].src_gate = src_gate->getId();
                temp_out_links[n_out].dest_node = getNodeFor(gate->getOwnerModule());
                temp_out_links[n_out].dest_gate = gate->getId();
                temp_out_links[n_out].wgt = 1.0;
                cDelayChannel *chPointF =  dynamic_cast<cDelayChannel *> (src_gate->getChannel());
                //cChannel *chPointF =  src_gate->getChannel();
                if(!chPointF->isDisabled())
                    temp_out_links[n_out].enabl = true;
                else
                {
                    temp_out_links[n_out].enabl = false;
                    std::cout << "NODE # " << nodev+k << " has a disabled link!\n";
                }
                n_out++;
            }
        }
        nodev[k].num_out_links = n_out;

        nodev[k].out_links = new cTopology::Link[n_out];
        memcpy(nodev[k].out_links, temp_out_links, n_out*sizeof(cTopology::Link));
        delete [] temp_out_links;
    }

    // fill in_links[] arrays
    for (int k=0; k<num_nodes; k++)
    {
        for (int l=0; l<nodev[k].num_out_links; l++)
        {
            cTopology::Link *link = &nodev[k].out_links[l];
            link->dest_node->in_links[link->dest_node->num_in_links++] = link;
        }
    }
}

cTopology::Node *cTopology::getNode(int i)
{
    if (i<0 || i>=num_nodes)
        throw cRuntimeError(this,"invalid node index %d",i);
    return nodev+i;
}

cTopology::Node *cTopology::getNodeFor(cModule *mod)
{
    // binary search can be done because nodev[] is ordered

    int lo, up, index;
    for ( lo=0, up=num_nodes, index=(lo+up)/2;
          lo<index;
          index=(lo+up)/2 )
    {
        // cycle invariant: nodev[lo].mod_id <= mod->getId() < nodev[up].mod_id
        if (mod->getId() < nodev[index].module_id)
             up = index;
        else
             lo = index;
    }
    return (mod->getId() == nodev[index].module_id) ? nodev+index : NULL;
}

void cTopology::calculateUnweightedSingleShortestPathsTo(Node *_target)
{
    // multiple paths not supported :-(

    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    for (int i=0; i<num_nodes; i++)
    {
       nodev[i].known = false;   // not really needed for unweighted
       nodev[i].dist = INFINITY;
       nodev[i].out_path = NULL;
    }
    target->dist = 0;

    std::deque<Node*> q;

    q.push_back(target);

    while (!q.empty())
    {
       Node *v = q.front();
       q.pop_front();

       // for each w adjacent to v...
       for (int i=0; i<v->num_in_links; i++)
       {
           if (!(v->in_links[i]->enabl)) continue;

           Node *w = v->in_links[i]->src_node;
           if (!w->enabl) continue;

           if (w->dist == INFINITY)
           {
               w->dist = v->dist + 1;
               w->out_path = v->in_links[i];
               q.push_back(w);
           }
       }
    }
}


void cTopology::weightedSingleShortestPathsTo(Node *_target)
{
    // multiple paths not supported :-(

    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    for (int i=0; i<num_nodes; i++)
    {
       nodev[i].known = false;   // not really needed for unweighted
       nodev[i].dist = INFINITY;
       nodev[i].out_path = NULL;
    }
    target->dist = 0;

    std::deque<Node*> q;

    q.push_back(target);

    while (!q.empty())
    {
       //find shortest in q
       //and use it instead of these two instuctions for retrieve the nodet
       Node *hold;
	for (int j=0; j<q.size();j++){
		for (int k=0; k<q.size()-1;k++){
			if ( q[ k ]->dist > q[ k +1 ]->dist){
				hold = q[ k ];
				q[ k ] = q[ k + 1 ];
				q[ k + 1 ]=hold;
			}
		}
	}

       Node *v = q.front();
       q.pop_front();
       v->known = true;
       
       // for each w adjacent to v...
       for (int i=0; i<v->num_in_links; i++)
       {
           if (!(v->in_links[i]->enabl)) continue;

           Node *w = v->in_links[i]->src_node;
           if (!w->enabl) continue;

           if (w->dist == INFINITY || (w->known==false && w->dist> ( v->dist + v->in_links[i]->wgt+v->wgt+v->num_in_links*0.01 ) ))
           {
               w->dist = v->dist + v->in_links[i]->wgt+v->wgt+v->num_in_links*0.01;
               w->out_path = v->in_links[i];
               q.push_back(w);
           }
       }
    }
}


void cTopology::weightedMultiShortestPathsTo(Node *_target)
{

    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    for (int i=0; i<num_nodes; i++)
    {
       nodev[i].known = false;   // not really needed for unweighted
       //<aa> At first, all the nodes are considered at infty distance
       //from the target </aa>
       nodev[i].dist = INFINITY; 
       nodev[i].out_path = NULL;
    }
    target->dist = 0; //<aa> The distace between target and iself is 0 </aa>

	//<aa> double ended queue, a queue that can be expanded in both the ends </aa>
    std::deque<Node*> q; 

    q.push_back(target);

    while (!q.empty())
    {
       //find shortest in q
       //For unweighted shortest paths it's the first element of q is always the node with the shortest distance
       Node *v = q.front();
       q.pop_front();
       v->known = true;
       
       // for each w adjacent to v...
       for (int i=0; i<v->num_in_links; i++)
       {
           if (!(v->in_links[i]->enabl)) continue;

           Node *w = v->in_links[i]->src_node;
           if (!w->enabl) continue;

           if (w->dist == INFINITY || (w->known==false && w->dist >  v->dist + 1  ))
           {
		       w->out_paths.clear();
               w->dist = v->dist + v->in_links[i]->wgt;//1;
               w->out_paths.push_back(v->in_links[i]);
               q.push_back(w);
           } else if ( w->dist != INFINITY && w->known==false && w->dist == v->dist + 1){
	       w->out_paths.push_back(v->in_links[i]);
	   }

       }
    }
}

void cTopology::extractWeight(std::string file){
using namespace std;
     ifstream fi;
     char row[100];
     char *path;
     int o,d;
     int weight;
     fi.open(file.c_str());

     while (fi >> row){
         path = strtok(row,":");
         o = atoi(path); //origin node
	 Node * source=getNode(o);
         path = strtok(NULL,":");
         d = atoi(path); //destination node
	 Node * dest=getNode(d);
         path = strtok(NULL,":");
         weight = atoi(path); //weight
         //cout<<"Link between "<<o<<" and "<<d<<" with weight = "<<weight<<endl;
	 //cout<<"Numero link "<<source->num_out_links<<endl;
   	 for (int i=0; i<source->num_out_links; i++){
		Node *try_dest = source->getLinkOut(i)->dest_node;
		//cModule* prova =try_dest->getModule();
		//int id =prova->getIndex();
		//cout<<"ID : "<<id<<endl;
		if(d==((try_dest->getModule())->getIndex())){
			cTopology::Link * link=source->getLinkOut(i);
			//double old=link->getWeight();
			link->setWeight(weight);
			//double new_=link->getWeight();
			//cout<<old<<" "<<new_<<" "<<weight<<endl;
			//cout<<"Settato "<<d<<"=="<<(try_dest->getModule())->getIndex()<<endl;
			}
	 
	}     

     }
     fi.close();

}

#include <map>

/* Betweenness centrality calculation. All the input parameters are provided by
 * the cTopology class. The algorithm has been published on the paper "A faster
 * Algorithm for Betweenness Centrality by Ulrik Brandes and has a complexity
 * of O(nm)
 */
void cTopology::betweenness_centrality(){
    std::deque<Node*> S,Q;

    std::map< int,std::deque<Node *> > P;

    Node *v, 
	 *w; 

    int t; //temporary iterator

    //Betweenness is zero (for each node) at the beginning
    for (t = 0; t < num_nodes; t++)
       nodev[t].btw = 0;

    int s = 0;

    for (int s = 0;s<num_nodes;s++){
	//
	//For each source reinitialize the variables
	//i.e., repeat the algorithm for each source
	//
	S.clear();
	Q.clear();

	for (int t = 0; t < num_nodes;t++){
	    P.clear();
	    nodev[t].sigma = 0;
	    nodev[t].delta = 0;
	    nodev[t].dist = -1;
	}

	nodev[s].sigma = 1;
	nodev[s].dist  = 0;

	//
	//Node v is the (pointer to the) current node
	//Node w is the (pointer to the) neighbour
	//
	Q.push_back( nodev+s );
	while (!Q.empty()){

	    Node *v = Q.front(); // given the node v
	    Q.pop_front();
	    //std::cout<<"popping "<<v->module_id<<endl;

	    S.push_back(v);
	    for (int i = 0; i < v->num_in_links; i++) {

	        w = v->in_links[i]->src_node;

		if ( w->dist < 0){
		   //std::cout<<"enqueuing "<< w->module_id << endl;
		   w->dist = v->dist+1;
		   Q.push_back(w);
		}

		if (w->dist == v->dist + 1){
		   w->sigma += v->sigma;
		   P[w->module_id].push_back(v); //See Lemma 3 of [brandes01jms]
		}


	    }

	}

	//
	//Node w is the (pointer to the) current node
	//Node v is the (pointer to the) neighbour
	//
	while (!S.empty()){
	    w = S.back(); S.pop_back();

	    for (std::deque<Node *>::iterator iv = P[w->module_id].begin(); iv != P[w->module_id].end(); iv++){
	        v = *iv;
	        v->delta += (v->sigma/w->sigma)*(1+w->delta); //See Theorem 6 of [brandes01jms]
	    }

	    if (w->module_id != nodev[s].module_id)
		w->btw += w->delta;
	}

    }

}

