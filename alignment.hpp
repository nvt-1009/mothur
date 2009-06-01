#ifndef DPALIGNMENT_H
#define DPALIGNMENT_H

/*
 *  dpalignment.h
 *  
 *
 *  Created by Pat Schloss on 12/15/08.
 *  Copyright 2008 Patrick D. Schloss. All rights reserved.
 *
 *  This is a class for an abstract datatype for classes that implement various types of alignment	algorithms.
 *	As of 12/18/08 these included alignments based on blastn, needleman-wunsch, and the	Gotoh algorithms
 * 
 */
using namespace std;

#include "mothur.h"
#include "alignmentcell.hpp"

/**************************************************************************************************/

class Alignment {
	
public:
	Alignment(int);
	Alignment();
	virtual void align(string, string) = 0;
	
	float getAlignmentScore();
	string getSeqAAln();
	string getSeqBAln();
	int getCandidateStartPos();
	int getCandidateEndPos();
	int getTemplateStartPos();
	int getTemplateEndPos();

	int getPairwiseLength();
//	int getLongestTemplateGap();

protected:
	void traceBack();
	string seqA, seqAaln;
	string seqB, seqBaln;
	int seqAstart, seqAend;
	int seqBstart, seqBend;
	int pairwiseLength;
	int nRows, nCols, lA, lB;
	vector<vector<AlignmentCell> > alignment;
};

/**************************************************************************************************/

#endif