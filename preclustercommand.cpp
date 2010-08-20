/*
 *  preclustercommand.cpp
 *  Mothur
 *
 *  Created by westcott on 12/21/09.
 *  Copyright 2009 Schloss Lab. All rights reserved.
 *
 */

#include "preclustercommand.h"

//**********************************************************************************************************************
inline bool comparePriority(seqPNode first, seqPNode second) {  return (first.numIdentical > second.numIdentical); }
//**********************************************************************************************************************

PreClusterCommand::PreClusterCommand(string option) {
	try {
		abort = false;
		
		//allow user to run help
		if(option == "help") { help(); abort = true; }
		
		else {
			//valid paramters for this command
			string Array[] =  {"fasta", "name", "diffs", "outputdir","inputdir"};
			vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
			
			OptionParser parser(option);
			map<string, string> parameters = parser.getParameters();
			
			ValidParameters validParameter;
			map<string, string>::iterator it;
		
			//check to make sure all parameters are valid for command
			for (map<string, string>::iterator it2 = parameters.begin(); it2 != parameters.end(); it2++) { 
				if (validParameter.isValidParameter(it2->first, myArray, it2->second) != true) {  abort = true;  }
			}
			
			//if the user changes the input directory command factory will send this info to us in the output parameter 
			string inputDir = validParameter.validFile(parameters, "inputdir", false);		
			if (inputDir == "not found"){	inputDir = "";		}
			else {
				string path;
				it = parameters.find("fasta");
				//user has given a template file
				if(it != parameters.end()){ 
					path = hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["fasta"] = inputDir + it->second;		}
				}
				
				it = parameters.find("name");
				//user has given a template file
				if(it != parameters.end()){ 
					path = hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["name"] = inputDir + it->second;		}
				}
			}

			//check for required parameters
			fastafile = validParameter.validFile(parameters, "fasta", true);
			if (fastafile == "not found") { m->mothurOut("fasta is a required parameter for the pre.cluster command."); m->mothurOutEndLine(); abort = true; }
			else if (fastafile == "not open") { abort = true; }	
			
			//if the user changes the output directory command factory will send this info to us in the output parameter 
			outputDir = validParameter.validFile(parameters, "outputdir", false);		if (outputDir == "not found"){	
				outputDir = "";	
				outputDir += hasPath(fastafile); //if user entered a file with a path then preserve it	
			}

			//check for optional parameter and set defaults
			// ...at some point should added some additional type checking...
			namefile = validParameter.validFile(parameters, "name", true);
			if (namefile == "not found") { namefile =  "";  }
			else if (namefile == "not open") { abort = true; }	
			else {  readNameFile();  }
			
			string temp	= validParameter.validFile(parameters, "diffs", false);				if(temp == "not found"){	temp = "1"; }
			convert(temp, diffs); 
		}
				
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "PreClusterCommand");
		exit(1);
	}
}

//**********************************************************************************************************************
PreClusterCommand::~PreClusterCommand(){}	
//**********************************************************************************************************************

void PreClusterCommand::help(){
	try {
		m->mothurOut("The pre.cluster command groups sequences that are within a given number of base mismatches.\n");
		m->mothurOut("The pre.cluster command outputs a new fasta and name file.\n");
		m->mothurOut("The pre.cluster command parameters are fasta, names and diffs. The fasta parameter is required. \n");
		m->mothurOut("The names parameter allows you to give a list of seqs that are identical. This file is 2 columns, first column is name or representative sequence, second column is a list of its identical sequences separated by commas.\n");
		m->mothurOut("The diffs parameter allows you to specify maximum number of mismatched bases allowed between sequences in a grouping. The default is 1.\n");
		m->mothurOut("The pre.cluster command should be in the following format: \n");
		m->mothurOut("pre.cluster(fasta=yourFastaFile, names=yourNamesFile, diffs=yourMaxDiffs) \n");
		m->mothurOut("Example pre.cluster(fasta=amazon.fasta, diffs=2).\n");
		m->mothurOut("Note: No spaces between parameter labels (i.e. fasta), '=' and parameters (i.e.yourFasta).\n\n");
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "help");
		exit(1);
	}
}
//**********************************************************************************************************************

int PreClusterCommand::execute(){
	try {
		
		if (abort == true) { return 0; }
		
		//reads fasta file and return number of seqs
		int numSeqs = readFASTA(); //fills alignSeqs and makes all seqs active
		
		if (m->control_pressed) { return 0; }
	
		if (numSeqs == 0) { m->mothurOut("Error reading fasta file...please correct."); m->mothurOutEndLine(); return 0;  }
		if (diffs > length) { m->mothurOut("Error: diffs is greater than your sequence length."); m->mothurOutEndLine(); return 0;  }
		
		string fileroot = outputDir + getRootName(getSimpleName(fastafile));
		string newFastaFile = fileroot + "precluster" + getExtension(fastafile);
		string newNamesFile = fileroot + "precluster.names";
		ofstream outFasta;
		ofstream outNames;
		
		openOutputFile(newFastaFile, outFasta);
		openOutputFile(newNamesFile, outNames);

		//sort seqs by number of identical seqs
		alignSeqs.sort(comparePriority);
	
		int count = 0;
		int i = 0;

		//think about running through twice...
		list<seqPNode>::iterator itList;
		list<seqPNode>::iterator itList2;
		for (itList = alignSeqs.begin(); itList != alignSeqs.end(); itList++) {			
			//try to merge it with all smaller seqs
			for (itList2 = alignSeqs.begin(); itList2 != alignSeqs.end(); itList2++) {
					
				if (m->control_pressed) { outFasta.close(); outNames.close(); remove(newFastaFile.c_str()); remove(newNamesFile.c_str());  return 0; }
					
				if ((*itList).seq.getName() != (*itList2).seq.getName()) { //you don't want to merge with yourself
					//are you within "diff" bases
					int mismatch = calcMisMatches((*itList).seq.getAligned(), (*itList2).seq.getAligned());
						
					if (mismatch <= diffs) {
						//merge
						(*itList).names += ',' + (*itList2).names;
						(*itList).numIdentical += (*itList2).numIdentical;

						alignSeqs.erase(itList2); itList2--;
						count++;
					}
				}
			}
			
			//ouptut this sequence
			printData(outFasta, outNames, (*itList));
			
			//remove sequence
			alignSeqs.erase(itList); itList--;
			
			i++;
			
			if(i % 100 == 0)	{ m->mothurOut(toString(i) + "\t" + toString(numSeqs - count) + "\t" + toString(count)); m->mothurOutEndLine();	}
		}
				
		if(i % 100 != 0)	{ m->mothurOut(toString(i) + "\t" + toString(numSeqs - count) + "\t" + toString(count)); m->mothurOutEndLine();	}
		
		outFasta.close();
		outNames.close();
		
		if (m->control_pressed) {  remove(newFastaFile.c_str()); remove(newNamesFile.c_str());  return 0; }

		
		m->mothurOut("Total number of sequences before precluster was " + toString(numSeqs) + "."); m->mothurOutEndLine();
		m->mothurOut("pre.cluster removed " + toString(count) + " sequences."); m->mothurOutEndLine(); 
		
		if (m->control_pressed) { remove(newFastaFile.c_str()); remove(newNamesFile.c_str()); return 0; }
		
		m->mothurOutEndLine();
		m->mothurOut("Output File Names: "); m->mothurOutEndLine();
		m->mothurOut(newFastaFile); m->mothurOutEndLine();	
		m->mothurOut(newNamesFile); m->mothurOutEndLine();	
		m->mothurOutEndLine();

		return 0;
		
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "execute");
		exit(1);
	}
}

/**************************************************************************************************/

//this requires the names and fasta file to be in the same order

int PreClusterCommand::readFASTA(){
	try {
		//ifstream inNames;
		ifstream inFasta;
		
		//openInputFile(namefile, inNames);
		openInputFile(fastafile, inFasta);
		
		//string firstCol, secondCol, nameString;
		length = 0;
		
		while (!inFasta.eof()) {
			
			if (m->control_pressed) { inFasta.close(); return 0; }
			
			//inNames >> firstCol >> secondCol;
			//nameString = secondCol;
			
			//gobble(inNames);
			//int size = 1;
			//while (secondCol.find_first_of(',') != -1) { 
			//	size++;
			//	secondCol = secondCol.substr(secondCol.find_first_of(',')+1, secondCol.length());
			//}
			
			Sequence seq(inFasta);  gobble(inFasta);
			
			if (seq.getName() != "") {  //can get "" if commented line is at end of fasta file
				if (namefile != "") {
					itSize = sizes.find(seq.getName());
					
					if (itSize == sizes.end()) { m->mothurOut(seq.getName() + " is not in your names file, please correct."); m->mothurOutEndLine(); exit(1); }
					else{
						seqPNode tempNode(itSize->second, seq, names[seq.getName()]);
						alignSeqs.push_back(tempNode);
						if (seq.getAligned().length() > length) {  length = seq.getAligned().length();  }
					}	
				}else { //no names file, you are identical to yourself 
					seqPNode tempNode(1, seq, seq.getName());
					alignSeqs.push_back(tempNode);
					if (seq.getAligned().length() > length) {  length = seq.getAligned().length();  }
				}
			}
		}
		inFasta.close();
		//inNames.close();
		return alignSeqs.size();
	}
	
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "readFASTA");
		exit(1);
	}
}

/**************************************************************************************************/

int PreClusterCommand::calcMisMatches(string seq1, string seq2){
	try {
		int numBad = 0;
		
		for (int i = 0; i < seq1.length(); i++) {
			//do they match
			if (seq1[i] != seq2[i]) { numBad++; }
			if (numBad > diffs) { return length;  } //to far to cluster
		}
		
		return numBad;
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "calcMisMatches");
		exit(1);
	}
}

/**************************************************************************************************/

void PreClusterCommand::printData(ofstream& outFasta, ofstream& outNames, seqPNode thisSeq){
	try {
		thisSeq.seq.printSequence(outFasta); 
		outNames << thisSeq.seq.getName() << '\t' << thisSeq.names << endl;
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "printData");
		exit(1);
	}
}
/**************************************************************************************************/

void PreClusterCommand::readNameFile(){
	try {
		ifstream in;
		openInputFile(namefile, in);
		string firstCol, secondCol;
				
		while (!in.eof()) {
			in >> firstCol >> secondCol; gobble(in);
			names[firstCol] = secondCol;
			int size = 1;
			while (secondCol.find_first_of(',') != -1) { 
				size++;
				secondCol = secondCol.substr(secondCol.find_first_of(',')+1, secondCol.length());
			}
			sizes[firstCol] = size;
		}
		in.close();
	}
	catch(exception& e) {
		m->errorOut(e, "PreClusterCommand", "readNameFile");
		exit(1);
	}
}

/**************************************************************************************************/


