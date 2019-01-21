#ifndef VENN_H
#define VENN_H
/*
 *  venn.h
 *  Mothur
 *
 *  Created by Sarah Westcott on 3/30/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */


#include "calculator.h"

/***********************************************************************/

class Venn {
public:
	Venn(string, bool, string, int, bool);
	~Venn(){};

	vector<string> getPic(SAbundVector*, vector<Calculator*>);
	vector<string> getPic(vector<SharedRAbundVector*>, vector<Calculator*>, vector<string>);

private:
	Calculator* singleCalc;
	string groupComb, outputDir, inputfile;
	ofstream outsvg;
	MothurOut* m;
	bool nseqs, sharedOtus;
	int fontSize;
    Utils util;
};

/***********************************************************************/

#endif

