//
//  sharedlcrvector.cpp
//  Mothur
//
//  Created by Sarah Westcott on 1/21/20.
//  Copyright © 2020 Schloss Lab. All rights reserved.
//

#include "sharedlcrvector.hpp"

/***********************************************************************/
SharedLCRVector::SharedLCRVector() : DataVector(), maxRank(0), numBins(0), numSeqs(0), group("") {}
/***********************************************************************/
SharedLCRVector::SharedLCRVector(int n) : DataVector(), data(n,0) , maxRank(0), numBins(n), numSeqs(0), group("") {}
/***********************************************************************/
SharedLCRVector::SharedLCRVector(vector<float> rav) :  DataVector(), maxRank(0), numBins(rav.size()), numSeqs(0), group("")  {
    try {
        data.assign(numBins, 0);
        for(int i=0;i<rav.size();i++){ set(i, rav[i]); }
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "SharedLCRVector");
        exit(1);
    }
}
/***********************************************************************/
SharedLCRVector::SharedLCRVector(vector<float> rav, float mr, int nb, float ns) :  DataVector(), group(""){
    try {
        numBins = nb;
        maxRank = mr;
        numSeqs = ns;
        data = rav;
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "SharedLCRVector");
        exit(1);
    }
}
/***********************************************************************/
SharedLCRVector::SharedLCRVector(ifstream& f) : DataVector(), maxRank(0), numBins(0), numSeqs(0) {
    try {
        f >> label >> group >> numBins;
        
        data.assign(numBins, 0);
        float inputData;
        
        for(int i=0;i<numBins;i++){
            f >> inputData;
            set(i, inputData);
        }
        
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "SharedLCRVector");
        exit(1);
    }
}
/***********************************************************************/
SharedLCRVector::SharedLCRVector(ifstream& f, string l, string g, int n) : DataVector(), maxRank(0), numBins(n), numSeqs(0) {
    try {
        label = l;
        group = g;
        data.assign(numBins, 0);
        
        float inputData;
        for(int i=0;i<numBins;i++){
            f >> inputData;
            set(i, inputData);
        }
        
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "SharedLCRVector");
        exit(1);
    }
}
/***********************************************************************/
void SharedLCRVector::set(int binNumber, float newBinSize){
    try {
        int oldBinSize = data[binNumber];
        data[binNumber] = newBinSize;
        
        if(newBinSize > maxRank)    {    maxRank = newBinSize;    }
        
        numSeqs += (newBinSize - oldBinSize);
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "set");
        exit(1);
    }
}
/***********************************************************************/
float SharedLCRVector::get(int index){ return data[index]; }
/***********************************************************************/
void SharedLCRVector::clear(){
    numBins = 0;
    maxRank = 0;
    numSeqs = 0;
    group = "";
    data.clear();
}
/***********************************************************************/
void SharedLCRVector::push_back(float binSize){
    try {
        data.push_back(binSize);
        numBins++;
        
        if(binSize > maxRank){ maxRank = binSize; }
        
        numSeqs += binSize;
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "push_back");
        exit(1);
    }
}
/***********************************************************************/
float SharedLCRVector::remove(int bin){
    try {
        float abund = data[bin];
        data.erase(data.begin()+bin);
        numBins--;
        
        if(abund == maxRank){ maxRank = util.max(data); }
        
        numSeqs -= abund;
        
        return abund;
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "remove");
        exit(1);
    }
}
/***********************************************************************/
float SharedLCRVector::remove(vector<int> bins){
    try {
        if (bins.size() == 0) { return 0; }
        
        int numRemoved = 0;
        vector<float> newData; int binIndex = 0;
        for (int i = 0; i < data.size(); i++) {
            if (m->getControl_pressed()) { break; }
            
            if (i != bins[binIndex]) {
                newData.push_back(data[i]);
            }else if (i == bins[binIndex]) {
                binIndex++;
                numRemoved += data[i];
                if (binIndex > bins.size()) { //removed all bins
                    newData.insert(newData.end(), data.begin()+i, data.end()); //add rest of good bins
                    break;
                }
            }
        }
        
        data = newData;
        numBins = data.size();
        
        vector<float>::iterator it = max_element(data.begin(), data.end());
        maxRank = *it;
        
        numSeqs -= numRemoved;
        
        return numRemoved;
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "remove");
        exit(1);
    }
}
/***********************************************************************/
void SharedLCRVector::resize(int size){
    data.resize(size);
    
    vector<float>::iterator it = max_element(data.begin(), data.end());
    maxRank = *it;
    numSeqs = util.sum(data);
    numBins = size;
}
/***********************************************************************/
int SharedLCRVector::size(){ return data.size(); }
/***********************************************************************/
void SharedLCRVector::print(ostream& output){
    try {
        output << label;
        output << '\t' << group << '\t' << numBins;
        
        for(int i=0;i<numBins;i++){        output  << '\t' << data[i];        }
        output << endl;
    }
    catch(exception& e) {
        m->errorOut(e, "SharedLCRVector", "nonSortedPrint");
        exit(1);
    }
}
/***********************************************************************/
