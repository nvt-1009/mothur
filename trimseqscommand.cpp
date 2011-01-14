/*
 *  trimseqscommand.cpp
 *  Mothur
 *
 *  Created by Pat Schloss on 6/6/09.
 *  Copyright 2009 Patrick D. Schloss. All rights reserved.
 *
 */

#include "trimseqscommand.h"
#include "needlemanoverlap.hpp"

//**********************************************************************************************************************

vector<string> TrimSeqsCommand::getValidParameters(){	
	try {
		string Array[] =  {"fasta", "flip", "oligos", "maxambig", "maxhomop", "group","minlength", "maxlength", "qfile", 
									"qthreshold", "qwindowaverage", "qstepsize", "qwindowsize", "qaverage", "rollaverage",
									"keepfirst", "removelast",
									"allfiles", "qtrim","tdiffs", "pdiffs", "bdiffs", "processors", "outputdir","inputdir"};
		vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "getValidParameters");
		exit(1);
	}
}

//**********************************************************************************************************************

TrimSeqsCommand::TrimSeqsCommand(){	
	try {
		abort = true;
		//initialize outputTypes
		vector<string> tempOutNames;
		outputTypes["fasta"] = tempOutNames;
		outputTypes["qual"] = tempOutNames;
		outputTypes["group"] = tempOutNames;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "TrimSeqsCommand");
		exit(1);
	}
}

//**********************************************************************************************************************

vector<string> TrimSeqsCommand::getRequiredParameters(){	
	try {
		string Array[] =  {"fasta"};
		vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "getRequiredParameters");
		exit(1);
	}
}

//**********************************************************************************************************************

vector<string> TrimSeqsCommand::getRequiredFiles(){	
	try {
		vector<string> myArray;
		return myArray;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "getRequiredFiles");
		exit(1);
	}
}

//***************************************************************************************************************

TrimSeqsCommand::TrimSeqsCommand(string option)  {
	try {
		
		abort = false;
		comboStarts = 0;
		
		//allow user to run help
		if(option == "help") { help(); abort = true; }
		
		else {
			//valid paramters for this command
			string AlignArray[] =  {	"fasta", "flip", "oligos", "maxambig", "maxhomop", "group","minlength", "maxlength", "qfile", 
								"qthreshold", "qwindowaverage", "qstepsize", "qwindowsize", "qaverage", "rollaverage",
								"keepfirst", "removelast",
								"allfiles", "qtrim","tdiffs", "pdiffs", "bdiffs", "processors", "outputdir","inputdir"};
			
			vector<string> myArray (AlignArray, AlignArray+(sizeof(AlignArray)/sizeof(string)));
			
			OptionParser parser(option);
			map<string,string> parameters = parser.getParameters();
			
			ValidParameters validParameter;
			map<string,string>::iterator it;
			
			//check to make sure all parameters are valid for command
			for (it = parameters.begin(); it != parameters.end(); it++) { 
				if (validParameter.isValidParameter(it->first, myArray, it->second) != true) {  abort = true;  }
			}
			
			//initialize outputTypes
			vector<string> tempOutNames;
			outputTypes["fasta"] = tempOutNames;
			outputTypes["qual"] = tempOutNames;
			outputTypes["group"] = tempOutNames;
			
			//if the user changes the input directory command factory will send this info to us in the output parameter 
			string inputDir = validParameter.validFile(parameters, "inputdir", false);		
			if (inputDir == "not found"){	inputDir = "";		}
			else {
				string path;
				it = parameters.find("fasta");
				//user has given a template file
				if(it != parameters.end()){ 
					path = m->hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["fasta"] = inputDir + it->second;		}
				}
				
				it = parameters.find("oligos");
				//user has given a template file
				if(it != parameters.end()){ 
					path = m->hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["oligos"] = inputDir + it->second;		}
				}
				
				it = parameters.find("qfile");
				//user has given a template file
				if(it != parameters.end()){ 
					path = m->hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["qfile"] = inputDir + it->second;		}
				}
				
				it = parameters.find("group");
				//user has given a template file
				if(it != parameters.end()){ 
					path = m->hasPath(it->second);
					//if the user has not given a path then, add inputdir. else leave path alone.
					if (path == "") {	parameters["group"] = inputDir + it->second;		}
				}
			}

			
			//check for required parameters
			fastaFile = validParameter.validFile(parameters, "fasta", true);
			if (fastaFile == "not found") { m->mothurOut("fasta is a required parameter for the trim.seqs command."); m->mothurOutEndLine(); abort = true; }
			else if (fastaFile == "not open") { abort = true; }	
			
			//if the user changes the output directory command factory will send this info to us in the output parameter 
			outputDir = validParameter.validFile(parameters, "outputdir", false);		if (outputDir == "not found"){	
				outputDir = "";	
				outputDir += m->hasPath(fastaFile); //if user entered a file with a path then preserve it	
			}
		
			
			//check for optional parameter and set defaults
			// ...at some point should added some additional type checking...
			string temp;
			temp = validParameter.validFile(parameters, "flip", false);
			if (temp == "not found"){	flip = 0;	}
			else if(m->isTrue(temp))	{	flip = 1;	}
		
			temp = validParameter.validFile(parameters, "oligos", true);
			if (temp == "not found"){	oligoFile = "";		}
			else if(temp == "not open"){	abort = true;	} 
			else					{	oligoFile = temp;		}
			
			temp = validParameter.validFile(parameters, "group", true);
			if (temp == "not found"){	groupfile = "";		}
			else if(temp == "not open"){	abort = true;	} 
			else					{	groupfile = temp;		}
			
			temp = validParameter.validFile(parameters, "maxambig", false);		if (temp == "not found") { temp = "-1"; }
			convert(temp, maxAmbig);  

			temp = validParameter.validFile(parameters, "maxhomop", false);		if (temp == "not found") { temp = "0"; }
			convert(temp, maxHomoP);  

			temp = validParameter.validFile(parameters, "minlength", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, minLength); 
			
			temp = validParameter.validFile(parameters, "maxlength", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, maxLength);
			
			temp = validParameter.validFile(parameters, "bdiffs", false);		if (temp == "not found") { temp = "0"; }
			convert(temp, bdiffs);
			
			temp = validParameter.validFile(parameters, "pdiffs", false);		if (temp == "not found") { temp = "0"; }
			convert(temp, pdiffs);
			
			temp = validParameter.validFile(parameters, "tdiffs", false);		if (temp == "not found") { int tempTotal = pdiffs + bdiffs;  temp = toString(tempTotal); }
			convert(temp, tdiffs);
			
			if(tdiffs == 0){	tdiffs = bdiffs + pdiffs;	}
			
			temp = validParameter.validFile(parameters, "qfile", true);	
			if (temp == "not found")	{	qFileName = "";		}
			else if(temp == "not open")	{	abort = true;		}
			else						{	qFileName = temp;	}
			
			temp = validParameter.validFile(parameters, "qthreshold", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, qThreshold);
			
			temp = validParameter.validFile(parameters, "qtrim", false);		if (temp == "not found") { temp = "F"; }
			qtrim = m->isTrue(temp);

			temp = validParameter.validFile(parameters, "rollaverage", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, qRollAverage);

			temp = validParameter.validFile(parameters, "qwindowaverage", false);if (temp == "not found") { temp = "0"; }
			convert(temp, qWindowAverage);

			temp = validParameter.validFile(parameters, "qwindowsize", false);	if (temp == "not found") { temp = "50"; }
			convert(temp, qWindowSize);

			temp = validParameter.validFile(parameters, "qstepsize", false);	if (temp == "not found") { temp = "1"; }
			convert(temp, qWindowStep);

			temp = validParameter.validFile(parameters, "qaverage", false);		if (temp == "not found") { temp = "0"; }
			convert(temp, qAverage);

			temp = validParameter.validFile(parameters, "keepfirst", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, keepFirst);

			temp = validParameter.validFile(parameters, "removelast", false);	if (temp == "not found") { temp = "0"; }
			convert(temp, removeLast);
			
			temp = validParameter.validFile(parameters, "allfiles", false);		if (temp == "not found") { temp = "F"; }
			allFiles = m->isTrue(temp);
			
			temp = validParameter.validFile(parameters, "processors", false);	if (temp == "not found") { temp = "1"; }
			convert(temp, processors); 
			
			if ((oligoFile != "") && (groupfile != "")) {
				m->mothurOut("You given both a oligos file and a groupfile, only one is allowed."); m->mothurOutEndLine(); abort = true;
			}
												
			
			if(allFiles && (oligoFile == "") && (groupfile == "")){
				m->mothurOut("You selected allfiles, but didn't enter an oligos or group file.  Ignoring the allfiles request."); m->mothurOutEndLine();
			}
			if((qAverage != 0 && qThreshold != 0) && qFileName == ""){
				m->mothurOut("You didn't provide a quality file name, quality criteria will be ignored."); m->mothurOutEndLine();
				qAverage=0;
				qThreshold=0;
			}
			if(!flip && oligoFile=="" && !maxLength && !minLength && (maxAmbig==-1) && !maxHomoP && qFileName == ""){		
				m->mothurOut("You didn't set any options... quiting command."); m->mothurOutEndLine();
				abort = true;
			}
		}

	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "TrimSeqsCommand");
		exit(1);
	}
}

//**********************************************************************************************************************

void TrimSeqsCommand::help(){
	try {
		m->mothurOut("The trim.seqs command reads a fastaFile and creates .....\n");
		m->mothurOut("The trim.seqs command parameters are fasta, flip, oligos, group, maxambig, maxhomop, minlength, maxlength, qfile, qthreshold, qaverage, diffs, qtrim and allfiles.\n");
		m->mothurOut("The fasta parameter is required.\n");
		m->mothurOut("The group parameter allows you to enter a group file for your fasta file.\n");
		m->mothurOut("The flip parameter will output the reverse compliment of your trimmed sequence. The default is false.\n");
		m->mothurOut("The oligos parameter .... The default is "".\n");
		m->mothurOut("The maxambig parameter .... The default is -1.\n");
		m->mothurOut("The maxhomop parameter .... The default is 0.\n");
		m->mothurOut("The minlength parameter .... The default is 0.\n");
		m->mothurOut("The maxlength parameter .... The default is 0.\n");
		m->mothurOut("The tdiffs parameter is used to specify the total number of differences allowed in the sequence. The default is pdiffs + bdiffs.\n");
		m->mothurOut("The bdiffs parameter is used to specify the number of differences allowed in the barcode. The default is 0.\n");
		m->mothurOut("The pdiffs parameter is used to specify the number of differences allowed in the primer. The default is 0.\n");
		m->mothurOut("The qfile parameter .....\n");
		m->mothurOut("The qthreshold parameter .... The default is 0.\n");
		m->mothurOut("The qaverage parameter .... The default is 0.\n");
		m->mothurOut("The allfiles parameter .... The default is F.\n");
		m->mothurOut("The qtrim parameter .... The default is F.\n");
		m->mothurOut("The trim.seqs command should be in the following format: \n");
		m->mothurOut("trim.seqs(fasta=yourFastaFile, flip=yourFlip, oligos=yourOligos, maxambig=yourMaxambig,  \n");
		m->mothurOut("maxhomop=yourMaxhomop, minlength=youMinlength, maxlength=yourMaxlength)  \n");	
		m->mothurOut("Example trim.seqs(fasta=abrecovery.fasta, flip=..., oligos=..., maxambig=..., maxhomop=..., minlength=..., maxlength=...).\n");
		m->mothurOut("Note: No spaces between parameter labels (i.e. fasta), '=' and parameters (i.e.yourFasta).\n");
		m->mothurOut("For more details please check out the wiki http://www.mothur.org/wiki/Trim.seqs .\n\n");

	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "help");
		exit(1);
	}
}


//***************************************************************************************************************

TrimSeqsCommand::~TrimSeqsCommand(){	/*	do nothing	*/	}

//***************************************************************************************************************

int TrimSeqsCommand::execute(){
	try{
	
		if (abort == true) { return 0; }
		
		numFPrimers = 0;  //this needs to be initialized
		numRPrimers = 0;
		vector<string> fastaFileNames;
		vector<string> qualFileNames;
		
		string trimSeqFile = outputDir + m->getRootName(m->getSimpleName(fastaFile)) + "trim.fasta";
		outputNames.push_back(trimSeqFile); outputTypes["fasta"].push_back(trimSeqFile);
		string scrapSeqFile = outputDir + m->getRootName(m->getSimpleName(fastaFile)) + "scrap.fasta";
		outputNames.push_back(scrapSeqFile); outputTypes["fasta"].push_back(scrapSeqFile);
		string trimQualFile = outputDir + m->getRootName(m->getSimpleName(fastaFile)) + "trim.qual";
		string scrapQualFile = outputDir + m->getRootName(m->getSimpleName(fastaFile)) + "scrap.qual";
		if (qFileName != "") {  outputNames.push_back(trimQualFile); outputNames.push_back(scrapQualFile);  outputTypes["qual"].push_back(trimQualFile); outputTypes["qual"].push_back(scrapQualFile); }
		string groupFile = "";
		if (groupfile == "") { groupFile = outputDir + m->getRootName(m->getSimpleName(fastaFile)) + "groups"; }
		else{
			groupFile = outputDir + m->getRootName(m->getSimpleName(groupfile)) + "trim.groups";
			outputNames.push_back(groupFile); outputTypes["group"].push_back(groupFile);
			groupMap = new GroupMap(groupfile);
			groupMap->readMap();
			
			if(allFiles){
				for (int i = 0; i < groupMap->namesOfGroups.size(); i++) {
					groupToIndex[groupMap->namesOfGroups[i]] = i;
					groupVector.push_back(groupMap->namesOfGroups[i]);
					fastaFileNames.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) +  groupMap->namesOfGroups[i] + ".fasta"));
					
					//we append later, so we want to clear file
					ofstream outRemove;
					m->openOutputFile(fastaFileNames[i], outRemove);
					outRemove.close();
					if(qFileName != ""){
						qualFileNames.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) +  groupMap->namesOfGroups[i] + ".qual"));
						ofstream outRemove2;
						m->openOutputFile(qualFileNames[i], outRemove2);
						outRemove2.close();
					}
				}
			}
			comboStarts = fastaFileNames.size()-1;
		}
		
		if(oligoFile != ""){
			outputNames.push_back(groupFile); outputTypes["group"].push_back(groupFile);
			getOligos(fastaFileNames, qualFileNames);
		}

		vector<unsigned long int> fastaFilePos;
		vector<unsigned long int> qFilePos;
		
		setLines(fastaFile, qFileName, fastaFilePos, qFilePos);
		
		for (int i = 0; i < (fastaFilePos.size()-1); i++) {
			lines.push_back(new linePair(fastaFilePos[i], fastaFilePos[(i+1)]));
			if (qFileName != "") {  qLines.push_back(new linePair(qFilePos[i], qFilePos[(i+1)]));  }
		}	
		if(qFileName == "")	{	qLines = lines;	} //files with duds
		
		#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
				if(processors == 1){
					driverCreateTrim(fastaFile, qFileName, trimSeqFile, scrapSeqFile, trimQualFile, scrapQualFile, groupFile, fastaFileNames, qualFileNames, lines[0], qLines[0]);
				}else{
					createProcessesCreateTrim(fastaFile, qFileName, trimSeqFile, scrapSeqFile, trimQualFile, scrapQualFile, groupFile, fastaFileNames, qualFileNames); 
				}	
		#else
				driverCreateTrim(fastaFile, qFileName, trimSeqFile, scrapSeqFile, trimQualFile, scrapQualFile, groupFile, fastaFileNames, qualFileNames, lines[0], qLines[0]);
		#endif
		
		if (m->control_pressed) {  return 0; }			
			
		set<string> blanks;
		for(int i=0;i<fastaFileNames.size();i++){
			if (m->isBlank(fastaFileNames[i])) {   blanks.insert(fastaFileNames[i]);	}
			else if (filesToRemove.count(fastaFileNames[i]) > 0) {  remove(fastaFileNames[i].c_str()); }
			else {
				ifstream inFASTA;
				string seqName;
				m->openInputFile(fastaFileNames[i], inFASTA);
				ofstream outGroups;
				string outGroupFilename = outputDir + m->getRootName(m->getSimpleName(fastaFileNames[i])) + "groups";
				
				//if the fastafile is on the blanks list then the groups file should be as well
				if (blanks.count(fastaFileNames[i]) != 0) { blanks.insert(outGroupFilename); }
				
				m->openOutputFile(outGroupFilename, outGroups);
				outputNames.push_back(outGroupFilename); outputTypes["group"].push_back(outGroupFilename);  
				
				string thisGroup = "";
				if (i > comboStarts) {
					map<string, int>::iterator itCombo;
					for(itCombo=combos.begin();itCombo!=combos.end(); itCombo++){
						if(itCombo->second == i){	thisGroup = itCombo->first;	combos.erase(itCombo);  break;  }
					}
				}else{ thisGroup = groupVector[i]; }
					
				while(!inFASTA.eof()){
					if(inFASTA.get() == '>'){
						inFASTA >> seqName;
						outGroups << seqName << '\t' << thisGroup << endl;
					}
					while (!inFASTA.eof())	{	char c = inFASTA.get(); if (c == 10 || c == 13){	break;	}	}
				}
				outGroups.close();
				inFASTA.close();
			}
		}
		
		for (set<string>::iterator itBlanks = blanks.begin(); itBlanks != blanks.end(); itBlanks++) {  remove((*(itBlanks)).c_str()); }
		
		blanks.clear();
		if(qFileName != ""){
			for(int i=0;i<qualFileNames.size();i++){
				if (m->isBlank(qualFileNames[i])) {  blanks.insert(qualFileNames[i]);	}
				else if (filesToRemove.count(qualFileNames[i]) > 0) {  remove(qualFileNames[i].c_str()); }
			}
		}
		
		for (set<string>::iterator itBlanks = blanks.begin(); itBlanks != blanks.end(); itBlanks++) {  remove((*(itBlanks)).c_str()); }
		
		if (m->control_pressed) { 
			for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); }
			return 0;
		}

		m->mothurOutEndLine();
		m->mothurOut("Output File Names: "); m->mothurOutEndLine();
		for (int i = 0; i < outputNames.size(); i++) {	m->mothurOut(outputNames[i]); m->mothurOutEndLine();	}
		m->mothurOutEndLine();
		
		return 0;	
			
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "execute");
		exit(1);
	}
}
		
/**************************************************************************************/

int TrimSeqsCommand::driverCreateTrim(string filename, string qFileName, string trimFile, string scrapFile, string trimQFile, string scrapQFile, string groupFile, vector<string> fastaNames, vector<string> qualNames, linePair* line, linePair* qline) {	
		
	try {
		
		ofstream outFASTA;
		m->openOutputFile(trimFile, outFASTA);
		
		ofstream scrapFASTA;
		m->openOutputFile(scrapFile, scrapFASTA);
		
		ofstream outQual;
		ofstream scrapQual;
		if(qFileName != ""){
			m->openOutputFile(trimQFile, outQual);
			m->openOutputFile(scrapQFile, scrapQual);
		}
		
		ofstream outGroups;
		
		if (oligoFile != "") {		
			m->openOutputFile(groupFile, outGroups);   
		}
		
		ifstream inFASTA;
		m->openInputFile(filename, inFASTA);
		inFASTA.seekg(line->start);
		
		ifstream qFile;
		if(qFileName != "")	{	m->openInputFile(qFileName, qFile);	qFile.seekg(qline->start);  }
		
		
		for (int i = 0; i < fastaNames.size(); i++) { //clears old file
			ofstream temp;
			m->openOutputFile(fastaNames[i], temp);
			temp.close();
		}
		for (int i = 0; i < qualNames.size(); i++) { //clears old file
			ofstream temp;
			m->openOutputFile(qualNames[i], temp);
			temp.close();
		}
		
			
		bool done = false;
		int count = 0;
	
		while (!done) {
				
			if (m->control_pressed) { 
				inFASTA.close(); outFASTA.close(); scrapFASTA.close();
				if (oligoFile != "") {	 outGroups.close();   }

				if(qFileName != ""){
					qFile.close();
				}
				for (int i = 0; i < outputNames.size(); i++) {	remove(outputNames[i].c_str()); }

				return 0;
			}
			
			int success = 1;
			

			Sequence currSeq(inFASTA); m->gobble(inFASTA);

			QualityScores currQual;
			if(qFileName != ""){
				currQual = QualityScores(qFile, currSeq.getNumBases());  m->gobble(qFile);
			}
			
			string origSeq = currSeq.getUnaligned();
			if (origSeq != "") {
				int groupBar, groupPrime;
				string trashCode = "";
				int currentSeqsDiffs = 0;

				if(barcodes.size() != 0){
					success = stripBarcode(currSeq, currQual, groupBar);
					if(success > bdiffs)		{	trashCode += 'b';	}
					else{ currentSeqsDiffs += success;  }
				}
				
				if(numFPrimers != 0){
					success = stripForward(currSeq, currQual, groupPrime);
					if(success > pdiffs)		{	trashCode += 'f';	}
					else{ currentSeqsDiffs += success;  }
				}
				
				if (currentSeqsDiffs > tdiffs)	{	trashCode += 't';   }
				
				if(numRPrimers != 0){
					success = stripReverse(currSeq, currQual);
					if(!success)				{	trashCode += 'r';	}
				}

				if(keepFirst != 0){
					success = keepFirstTrim(currSeq, currQual);
				}
				
				if(removeLast != 0){
					success = removeLastTrim(currSeq, currQual);
					if(!success)				{	trashCode += 'l';	}
				}

				
				if(qFileName != ""){
					int origLength = currSeq.getNumBases();
					
					if(qThreshold != 0)			{	success = currQual.stripQualThreshold(currSeq, qThreshold);			}
					else if(qAverage != 0)		{	success = currQual.cullQualAverage(currSeq, qAverage);				}
					else if(qRollAverage != 0)	{	success = currQual.stripQualRollingAverage(currSeq, qRollAverage);	}
					else if(qWindowAverage != 0){	success = currQual.stripQualWindowAverage(currSeq, qWindowStep, qWindowSize, qWindowAverage);	}
					else						{	success = 1;				}
					
					//you don't want to trim, if it fails above then scrap it
					if ((!qtrim) && (origLength != currSeq.getNumBases())) { success = 0; }
					
					if(!success)				{	trashCode += 'q';	}
				}				
		
				if(minLength > 0 || maxLength > 0){
					success = cullLength(currSeq);
					if(!success)				{	trashCode += 'l';	}
				}
				if(maxHomoP > 0){
					success = cullHomoP(currSeq);
					if(!success)				{	trashCode += 'h';	}
				}
				if(maxAmbig != -1){
					success = cullAmbigs(currSeq);
					if(!success)				{	trashCode += 'n';	}
				}
				
				if(flip){		// should go last			
					currSeq.reverseComplement();
					currQual.flipQScores();	
				}
				
				if(trashCode.length() == 0){
					currSeq.setAligned(currSeq.getUnaligned());
					currSeq.printSequence(outFASTA);
					currQual.printQScores(outQual);
					
					if(barcodes.size() != 0){
						string thisGroup = groupVector[groupBar];
						int indexToFastaFile = groupBar;
						if (primers.size() != 0){
							//does this primer have a group
							if (groupVector[groupPrime] != "") {  
								thisGroup += "." + groupVector[groupPrime]; 
								indexToFastaFile = combos[thisGroup];
							}
						}
						outGroups << currSeq.getName() << '\t' << thisGroup << endl;
						if(allFiles){
							ofstream outTemp;
							m->openOutputFileAppend(fastaNames[indexToFastaFile], outTemp);
							//currSeq.printSequence(*fastaFileNames[indexToFastaFile]);
							currSeq.printSequence(outTemp);
							outTemp.close();
							
							if(qFileName != ""){
								//currQual.printQScores(*qualFileNames[indexToFastaFile]);
								ofstream outTemp2;
								m->openOutputFileAppend(qualNames[indexToFastaFile], outTemp2);
								currQual.printQScores(outTemp2);
								outTemp2.close();							
							}
						}
					}
					
					if (groupfile != "") {
						string thisGroup = groupMap->getGroup(currSeq.getName());
						
						if (thisGroup != "not found") {
							outGroups << currSeq.getName() << '\t' << thisGroup << endl;
							if (allFiles) {
								ofstream outTemp;
								m->openOutputFileAppend(fastaNames[groupToIndex[thisGroup]], outTemp);
								currSeq.printSequence(outTemp);
								outTemp.close();
								if(qFileName != ""){
									ofstream outTemp2;
									m->openOutputFileAppend(qualNames[groupToIndex[thisGroup]], outTemp2);
									currQual.printQScores(outTemp2);
									outTemp2.close();							
								}
							}
						}else{
							m->mothurOut(currSeq.getName() + " is not in your groupfile, adding to group XXX."); m->mothurOutEndLine();
							outGroups << currSeq.getName() << '\t' << "XXX" << endl;
							if (allFiles) {  
								m->mothurOut("[ERROR]: " + currSeq.getName() + " will not be added to any .group.fasta or .group.qual file."); m->mothurOutEndLine();
							}
						}
					}
				}
				else{
					currSeq.setName(currSeq.getName() + '|' + trashCode);
					currSeq.setUnaligned(origSeq);
					currSeq.setAligned(origSeq);
					currSeq.printSequence(scrapFASTA);
					currQual.printQScores(scrapQual);
				}
				count++;
			}
			
			#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
				unsigned long int pos = inFASTA.tellg();
				if ((pos == -1) || (pos >= line->end)) { break; }
			#else
				if (inFASTA.eof()) { break; }
			#endif
				
			//report progress
			if((count) % 1000 == 0){	m->mothurOut(toString(count)); m->mothurOutEndLine();		}
			
		}
		//report progress
		if((count) % 1000 != 0){	m->mothurOut(toString(count)); m->mothurOutEndLine();		}

		
		inFASTA.close();
		outFASTA.close();
		scrapFASTA.close();
		if (oligoFile != "") {	 outGroups.close();   }
		if(qFileName != "")	{	qFile.close();	scrapQual.close(); outQual.close();	}
		
		return count;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "driverCreateTrim");
		exit(1);
	}
}

/**************************************************************************************************/

int TrimSeqsCommand::createProcessesCreateTrim(string filename, string qFileName, string trimFile, string scrapFile, string trimQFile, string scrapQFile, string groupFile, vector<string> fastaNames, vector<string> qualNames) {
	try {
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
		int process = 1;
		int exitCommand = 1;
		processIDS.clear();
		
		//loop through and create all the processes you want
		while (process != processors) {
			int pid = fork();
			
			if (pid > 0) {
				processIDS.push_back(pid);  //create map from line number to pid so you can append files in correct order later
				process++;
			}else if (pid == 0){
				for (int i = 0; i < fastaNames.size(); i++) {
					fastaNames[i] = (fastaNames[i] + toString(getpid()) + ".temp");
					//clear old file if it exists
					ofstream temp;
					m->openOutputFile(fastaNames[i], temp);
					temp.close();
					if(qFileName != ""){
						qualNames[i] = (qualNames[i] + toString(getpid()) + ".temp");
						//clear old file if it exists
						ofstream temp2;
						m->openOutputFile(qualNames[i], temp2);
						temp2.close();
					}
				}
				
				driverCreateTrim(filename, qFileName, (trimFile + toString(getpid()) + ".temp"), (scrapFile + toString(getpid()) + ".temp"), (trimQFile + toString(getpid()) + ".temp"), (scrapQFile + toString(getpid()) + ".temp"), (groupFile + toString(getpid()) + ".temp"), fastaNames, qualNames, lines[process], qLines[process]);
				exit(0);
			}else { 
				m->mothurOut("[ERROR]: unable to spawn the necessary processes."); m->mothurOutEndLine(); 
				for (int i = 0; i < processIDS.size(); i++) { kill (processIDS[i], SIGINT); }
				exit(0);
			}
		}
		
		//parent do my part
		for (int i = 0; i < fastaNames.size(); i++) {
			//clear old file if it exists
			ofstream temp;
			m->openOutputFile(fastaNames[i], temp);
			temp.close();
			if(qFileName != ""){
				//clear old file if it exists
				ofstream temp2;
				m->openOutputFile(qualNames[i], temp2);
				temp2.close();
			}
		}
		
		driverCreateTrim(filename, qFileName, trimFile, scrapFile, trimQFile, scrapQFile, groupFile, fastaNames, qualNames, lines[0], qLines[0]);
		
		
		//force parent to wait until all the processes are done
		for (int i=0;i<processIDS.size();i++) { 
			int temp = processIDS[i];
			wait(&temp);
		}
		
		//append files
		for(int i=0;i<processIDS.size();i++){
			
			m->mothurOut("Appending files from process " + toString(processIDS[i])); m->mothurOutEndLine();
			
			m->appendFiles((trimFile + toString(processIDS[i]) + ".temp"), trimFile);
			remove((trimFile + toString(processIDS[i]) + ".temp").c_str());
			m->appendFiles((scrapFile + toString(processIDS[i]) + ".temp"), scrapFile);
			remove((scrapFile + toString(processIDS[i]) + ".temp").c_str());
			
			m->mothurOut("Done with fasta files"); m->mothurOutEndLine();
			
			if(qFileName != ""){
				m->appendFiles((trimQFile + toString(processIDS[i]) + ".temp"), trimQFile);
				remove((trimQFile + toString(processIDS[i]) + ".temp").c_str());
				m->appendFiles((scrapQFile + toString(processIDS[i]) + ".temp"), scrapQFile);
				remove((scrapQFile + toString(processIDS[i]) + ".temp").c_str());
			
				m->mothurOut("Done with quality files"); m->mothurOutEndLine();
			}
			
			m->appendFiles((groupFile + toString(processIDS[i]) + ".temp"), groupFile);
			remove((groupFile + toString(processIDS[i]) + ".temp").c_str());
			
			m->mothurOut("Done with group file"); m->mothurOutEndLine();
			
			for (int j = 0; j < fastaNames.size(); j++) {
				m->appendFiles((fastaNames[j] + toString(processIDS[i]) + ".temp"), fastaNames[j]);
				remove((fastaNames[j] + toString(processIDS[i]) + ".temp").c_str());
			}
			
			if(qFileName != ""){
				for (int j = 0; j < qualNames.size(); j++) {
					m->appendFiles((qualNames[j] + toString(processIDS[i]) + ".temp"), qualNames[j]);
					remove((qualNames[j] + toString(processIDS[i]) + ".temp").c_str());
				}
			}
			
			if (allFiles) { m->mothurOut("Done with allfiles"); m->mothurOutEndLine(); }
		}
	
		return exitCommand;
#endif		
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "createProcessesCreateTrim");
		exit(1);
	}
}

/**************************************************************************************************/

int TrimSeqsCommand::setLines(string filename, string qfilename, vector<unsigned long int>& fastaFilePos, vector<unsigned long int>& qfileFilePos) {
	try {
		
		//set file positions for fasta file
		fastaFilePos = m->divideFile(filename, processors);
		
		if (qfilename == "") { return processors; }
		
		//get name of first sequence in each chunk
		map<string, int> firstSeqNames;
		for (int i = 0; i < (fastaFilePos.size()-1); i++) {
			ifstream in;
			m->openInputFile(filename, in);
			in.seekg(fastaFilePos[i]);
		
			Sequence temp(in); 
			firstSeqNames[temp.getName()] = i;
		
			in.close();
		}
				
		//seach for filePos of each first name in the qfile and save in qfileFilePos
		ifstream inQual;
		m->openInputFile(qfilename, inQual);
		
		string input;
		while(!inQual.eof()){	
			input = m->getline(inQual);

			if (input.length() != 0) {
				if(input[0] == '>'){ //this is a sequence name line
					istringstream nameStream(input);
					
					string sname = "";  nameStream >> sname;
					sname = sname.substr(1);
					
					map<string, int>::iterator it = firstSeqNames.find(sname);
					
					if(it != firstSeqNames.end()) { //this is the start of a new chunk
						unsigned long int pos = inQual.tellg(); 
						qfileFilePos.push_back(pos - input.length() - 1);	
						firstSeqNames.erase(it);
					}
				}
			}
			
			if (firstSeqNames.size() == 0) { break; }
		}
		inQual.close();
		
		
		if (firstSeqNames.size() != 0) { 
			for (map<string, int>::iterator it = firstSeqNames.begin(); it != firstSeqNames.end(); it++) {
				m->mothurOut(it->first + " is in your fasta file and not in your quality file, not using quality file."); m->mothurOutEndLine();
			}
			qFileName = "";
			return processors;
		}

		//get last file position of qfile
		FILE * pFile;
		unsigned long int size;
		
		//get num bytes in file
		pFile = fopen (qfilename.c_str(),"rb");
		if (pFile==NULL) perror ("Error opening file");
		else{
			fseek (pFile, 0, SEEK_END);
			size=ftell (pFile);
			fclose (pFile);
		}
		
		qfileFilePos.push_back(size);
		
		return processors;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "setLines");
		exit(1);
	}
}

//***************************************************************************************************************

void TrimSeqsCommand::getOligos(vector<string>& outFASTAVec, vector<string>& outQualVec){
	try {
		ifstream inOligos;
		m->openInputFile(oligoFile, inOligos);
		
		ofstream test;
		
		string type, oligo, group;
		int index=0;
		//int indexPrimer = 0;
		
		while(!inOligos.eof()){
			inOligos >> type; m->gobble(inOligos);
		 			
			if(type[0] == '#'){
				while (!inOligos.eof())	{	char c = inOligos.get(); if (c == 10 || c == 13){	break;	}	} // get rest of line if there's any crap there
			}
			else{
				//make type case insensitive
				for(int i=0;i<type.length();i++){	type[i] = toupper(type[i]);  }
				
				inOligos >> oligo;
				
				for(int i=0;i<oligo.length();i++){
					oligo[i] = toupper(oligo[i]);
					if(oligo[i] == 'U')	{	oligo[i] = 'T';	}
				}
				
				if(type == "FORWARD"){
					group = "";
					
					// get rest of line in case there is a primer name
					while (!inOligos.eof())	{	
						char c = inOligos.get(); 
						if (c == 10 || c == 13){	break;	}
						else if (c == 32 || c == 9){;} //space or tab
						else { 	group += c;  }
					} 
					
					//check for repeat barcodes
					map<string, int>::iterator itPrime = primers.find(oligo);
					if (itPrime != primers.end()) { m->mothurOut("primer " + oligo + " is in your oligos file already."); m->mothurOutEndLine();  }
					
						primers[oligo]=index; index++;
						groupVector.push_back(group);
					
						if(allFiles){
							outFASTAVec.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
							if(qFileName != ""){
								outQualVec.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
							}
							if (group == "") { //if there is not a group for this primer, then this file will not get written to, but we add it to keep the indexes correct
								filesToRemove.insert((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
								if(qFileName != ""){
									filesToRemove.insert((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
								}
							}else {
								outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
								outputTypes["fasta"].push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
								if(qFileName != ""){
									outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
									outputTypes["qual"].push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
								}							
							}
						}
					
				}
				else if(type == "REVERSE"){
					Sequence oligoRC("reverse", oligo);
					oligoRC.reverseComplement();
					revPrimer.push_back(oligoRC.getUnaligned());
				}
				else if(type == "BARCODE"){
					inOligos >> group;
					
					//check for repeat barcodes
					map<string, int>::iterator itBar = barcodes.find(oligo);
					if (itBar != barcodes.end()) { m->mothurOut("barcode " + oligo + " is in your oligos file already."); m->mothurOutEndLine();  }
					
						barcodes[oligo]=index; index++;
						groupVector.push_back(group);
						
						if(allFiles){
							outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
							outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
							outFASTAVec.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + group + ".fasta"));
							if(qFileName != ""){
								outQualVec.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
								outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
								outputTypes["qual"].push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + group + ".qual"));
							}							
						}
					
				}else{	m->mothurOut(type + " is not recognized as a valid type. Choices are forward, reverse, and barcode. Ignoring " + oligo + "."); m->mothurOutEndLine();  }
			}
			m->gobble(inOligos);
		}
		
		inOligos.close();
		
		//add in potential combos
		if(allFiles){
			comboStarts = outFASTAVec.size()-1;
			for (map<string, int>::iterator itBar = barcodes.begin(); itBar != barcodes.end(); itBar++) {
				for (map<string, int>::iterator itPrime = primers.begin(); itPrime != primers.end(); itPrime++) {
					if (groupVector[itPrime->second] != "") { //there is a group for this primer
						outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".fasta"));
						outputTypes["fasta"].push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".fasta"));
						outFASTAVec.push_back((outputDir + m->getRootName(m->getSimpleName(fastaFile)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".fasta"));
						combos[(groupVector[itBar->second] + "." + groupVector[itPrime->second])] = outFASTAVec.size()-1;
						
						if(qFileName != ""){
							outQualVec.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".qual"));
							outputNames.push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".qual"));
							outputTypes["qual"].push_back((outputDir + m->getRootName(m->getSimpleName(qFileName)) + groupVector[itBar->second] + "." + groupVector[itPrime->second] + ".qual"));
						}
					}
				}
			}
		}
		
		numFPrimers = primers.size();
		numRPrimers = revPrimer.size();
		
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "getOligos");
		exit(1);
	}
}
//***************************************************************************************************************

int TrimSeqsCommand::stripBarcode(Sequence& seq, QualityScores& qual, int& group){
	try {
		
		string rawSequence = seq.getUnaligned();
		int success = bdiffs + 1;	//guilty until proven innocent
		
		//can you find the barcode
		for(map<string,int>::iterator it=barcodes.begin();it!=barcodes.end();it++){
			string oligo = it->first;
			if(rawSequence.length() < oligo.length()){	//let's just assume that the barcodes are the same length
				success = bdiffs + 10;					//if the sequence is shorter than the barcode then bail out
				break;	
			}
			
			if(compareDNASeq(oligo, rawSequence.substr(0,oligo.length()))){
				group = it->second;
				seq.setUnaligned(rawSequence.substr(oligo.length()));
				
				if(qual.getName() != ""){
					qual.trimQScores(oligo.length(), -1);
				}
				
				success = 0;
				break;
			}
		}
		
		//if you found the barcode or if you don't want to allow for diffs
//		cout << success;
		if ((bdiffs == 0) || (success == 0)) { return success;  }
		
		else { //try aligning and see if you can find it
//			cout << endl;

			int maxLength = 0;

			Alignment* alignment;
			if (barcodes.size() > 0) {
				map<string,int>::iterator it=barcodes.begin();

				for(it;it!=barcodes.end();it++){
					if(it->first.length() > maxLength){
						maxLength = it->first.length();
					}
				}
				alignment = new NeedlemanOverlap(-1.0, 1.0, -1.0, (maxLength+bdiffs+1));  

			}else{ alignment = NULL; } 
			
			//can you find the barcode
			int minDiff = 1e6;
			int minCount = 1;
			int minGroup = -1;
			int minPos = 0;
			
			for(map<string,int>::iterator it=barcodes.begin();it!=barcodes.end();it++){
				string oligo = it->first;
//				int length = oligo.length();
				
				if(rawSequence.length() < maxLength){	//let's just assume that the barcodes are the same length
					success = bdiffs + 10;
					break;
				}
				
				//use needleman to align first barcode.length()+numdiffs of sequence to each barcode
				alignment->align(oligo, rawSequence.substr(0,oligo.length()+bdiffs));
				oligo = alignment->getSeqAAln();
				string temp = alignment->getSeqBAln();
		
				int alnLength = oligo.length();
				
				for(int i=oligo.length()-1;i>=0;i--){
					if(oligo[i] != '-'){	alnLength = i+1;	break;	}
				}
				oligo = oligo.substr(0,alnLength);
				temp = temp.substr(0,alnLength);
				
				int numDiff = countDiffs(oligo, temp);
				
//				cout << oligo << '\t' << temp << '\t' << numDiff << endl;				
				
				if(numDiff < minDiff){
					minDiff = numDiff;
					minCount = 1;
					minGroup = it->second;
					minPos = 0;
					for(int i=0;i<alnLength;i++){
						if(temp[i] != '-'){
							minPos++;
						}
					}
				}
				else if(numDiff == minDiff){
					minCount++;
				}

			}

			if(minDiff > bdiffs)	{	success = minDiff;		}	//no good matches
			else if(minCount > 1)	{	success = bdiffs + 100;	}	//can't tell the difference between multiple barcodes
			else{													//use the best match
				group = minGroup;
				seq.setUnaligned(rawSequence.substr(minPos));
				
				if(qual.getName() != ""){
					qual.trimQScores(minPos, -1);
				}
				success = minDiff;
			}
			
			if (alignment != NULL) {  delete alignment;  }
			
		}
//		cout << success << endl;
		
		return success;
		
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "stripBarcode");
		exit(1);
	}

}

//***************************************************************************************************************

int TrimSeqsCommand::stripForward(Sequence& seq, QualityScores& qual, int& group){
	try {
		string rawSequence = seq.getUnaligned();
		int success = pdiffs + 1;	//guilty until proven innocent
		
		//can you find the primer
		for(map<string,int>::iterator it=primers.begin();it!=primers.end();it++){
			string oligo = it->first;
			if(rawSequence.length() < oligo.length()){	//let's just assume that the primers are the same length
				success = pdiffs + 10;					//if the sequence is shorter than the barcode then bail out
				break;	
			}
			
			if(compareDNASeq(oligo, rawSequence.substr(0,oligo.length()))){
				group = it->second;
				seq.setUnaligned(rawSequence.substr(oligo.length()));
				if(qual.getName() != ""){
					qual.trimQScores(oligo.length(), -1);
				}
				success = 0;
				break;
			}
		}

		//if you found the barcode or if you don't want to allow for diffs
//		cout << success;
		if ((pdiffs == 0) || (success == 0)) { return success;  }
		
		else { //try aligning and see if you can find it
//			cout << endl;

			int maxLength = 0;

			Alignment* alignment;
			if (primers.size() > 0) {
				map<string,int>::iterator it=primers.begin();

				for(it;it!=primers.end();it++){
					if(it->first.length() > maxLength){
						maxLength = it->first.length();
					}
				}
				alignment = new NeedlemanOverlap(-1.0, 1.0, -1.0, (maxLength+pdiffs+1));  

			}else{ alignment = NULL; } 
			
			//can you find the barcode
			int minDiff = 1e6;
			int minCount = 1;
			int minGroup = -1;
			int minPos = 0;
			
			for(map<string,int>::iterator it=primers.begin();it!=primers.end();it++){
				string oligo = it->first;
//				int length = oligo.length();
				
				if(rawSequence.length() < maxLength){	
					success = pdiffs + 100;
					break;
				}
				
				//use needleman to align first barcode.length()+numdiffs of sequence to each barcode
				alignment->align(oligo, rawSequence.substr(0,oligo.length()+pdiffs));
				oligo = alignment->getSeqAAln();
				string temp = alignment->getSeqBAln();
		
				int alnLength = oligo.length();
				
				for(int i=oligo.length()-1;i>=0;i--){
					if(oligo[i] != '-'){	alnLength = i+1;	break;	}
				}
				oligo = oligo.substr(0,alnLength);
				temp = temp.substr(0,alnLength);
				
				int numDiff = countDiffs(oligo, temp);
				
//				cout << oligo << '\t' << temp << '\t' << numDiff << endl;				
				
				if(numDiff < minDiff){
					minDiff = numDiff;
					minCount = 1;
					minGroup = it->second;
					minPos = 0;
					for(int i=0;i<alnLength;i++){
						if(temp[i] != '-'){
							minPos++;
						}
					}
				}
				else if(numDiff == minDiff){
					minCount++;
				}

			}

			if(minDiff > pdiffs)	{	success = minDiff;		}	//no good matches
			else if(minCount > 1)	{	success = pdiffs + 10;	}	//can't tell the difference between multiple primers
			else{													//use the best match
				group = minGroup;
				seq.setUnaligned(rawSequence.substr(minPos));
				if(qual.getName() != ""){
					qual.trimQScores(minPos, -1);
				}
				success = minDiff;
			}
			
			if (alignment != NULL) {  delete alignment;  }
			
		}
		
		return success;

	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "stripForward");
		exit(1);
	}
}

//***************************************************************************************************************

bool TrimSeqsCommand::stripReverse(Sequence& seq, QualityScores& qual){
	try {
		string rawSequence = seq.getUnaligned();
		bool success = 0;	//guilty until proven innocent
		
		for(int i=0;i<numRPrimers;i++){
			string oligo = revPrimer[i];
			
			if(rawSequence.length() < oligo.length()){
				success = 0;
				break;
			}
			
			if(compareDNASeq(oligo, rawSequence.substr(rawSequence.length()-oligo.length(),oligo.length()))){
				seq.setUnaligned(rawSequence.substr(0,rawSequence.length()-oligo.length()));
				if(qual.getName() != ""){
					qual.trimQScores(-1, rawSequence.length()-oligo.length());
				}
				success = 1;
				break;
			}
		}	
		return success;
		
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "stripReverse");
		exit(1);
	}
}

//***************************************************************************************************************

bool TrimSeqsCommand::keepFirstTrim(Sequence& sequence, QualityScores& qscores){
	try {
		bool success = 1;
		if(qscores.getName() != ""){
			qscores.trimQScores(-1, keepFirst);
		}
		sequence.trim(keepFirst);
		return success;
	}
	catch(exception& e) {
		m->errorOut(e, "keepFirstTrim", "countDiffs");
		exit(1);
	}
	
}	

//***************************************************************************************************************

bool TrimSeqsCommand::removeLastTrim(Sequence& sequence, QualityScores& qscores){
	try {
		bool success = 0;
		
		int length = sequence.getNumBases() - removeLast;
		
		if(length > 0){
			if(qscores.getName() != ""){
				qscores.trimQScores(-1, length);
			}
			sequence.trim(length);
			success = 1;
		}
		else{
			success = 0;
		}

		return success;
	}
	catch(exception& e) {
		m->errorOut(e, "removeLastTrim", "countDiffs");
		exit(1);
	}
	
}	

//***************************************************************************************************************

bool TrimSeqsCommand::cullLength(Sequence& seq){
	try {
	
		int length = seq.getNumBases();
		bool success = 0;	//guilty until proven innocent
		
		if(length >= minLength && maxLength == 0)			{	success = 1;	}
		else if(length >= minLength && length <= maxLength)	{	success = 1;	}
		else												{	success = 0;	}
		
		return success;
	
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "cullLength");
		exit(1);
	}
	
}

//***************************************************************************************************************

bool TrimSeqsCommand::cullHomoP(Sequence& seq){
	try {
		int longHomoP = seq.getLongHomoPolymer();
		bool success = 0;	//guilty until proven innocent
		
		if(longHomoP <= maxHomoP){	success = 1;	}
		else					{	success = 0;	}
		
		return success;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "cullHomoP");
		exit(1);
	}
	
}

//***************************************************************************************************************

bool TrimSeqsCommand::cullAmbigs(Sequence& seq){
	try {
		int numNs = seq.getAmbigBases();
		bool success = 0;	//guilty until proven innocent
		
		if(numNs <= maxAmbig)	{	success = 1;	}
		else					{	success = 0;	}
		
		return success;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "cullAmbigs");
		exit(1);
	}
	
}

//***************************************************************************************************************

bool TrimSeqsCommand::compareDNASeq(string oligo, string seq){
	try {
		bool success = 1;
		int length = oligo.length();
		
		for(int i=0;i<length;i++){
			
			if(oligo[i] != seq[i]){
				if(oligo[i] == 'A' || oligo[i] == 'T' || oligo[i] == 'G' || oligo[i] == 'C')	{	success = 0; 	}
				else if((oligo[i] == 'N' || oligo[i] == 'I') && (seq[i] == 'N'))				{	success = 0;	}
				else if(oligo[i] == 'R' && (seq[i] != 'A' && seq[i] != 'G'))					{	success = 0;	}
				else if(oligo[i] == 'Y' && (seq[i] != 'C' && seq[i] != 'T'))					{	success = 0;	}
				else if(oligo[i] == 'M' && (seq[i] != 'C' && seq[i] != 'A'))					{	success = 0;	}
				else if(oligo[i] == 'K' && (seq[i] != 'T' && seq[i] != 'G'))					{	success = 0;	}
				else if(oligo[i] == 'W' && (seq[i] != 'T' && seq[i] != 'A'))					{	success = 0;	}
				else if(oligo[i] == 'S' && (seq[i] != 'C' && seq[i] != 'G'))					{	success = 0;	}
				else if(oligo[i] == 'B' && (seq[i] != 'C' && seq[i] != 'T' && seq[i] != 'G'))	{	success = 0;	}
				else if(oligo[i] == 'D' && (seq[i] != 'A' && seq[i] != 'T' && seq[i] != 'G'))	{	success = 0;	}
				else if(oligo[i] == 'H' && (seq[i] != 'A' && seq[i] != 'T' && seq[i] != 'C'))	{	success = 0;	}
				else if(oligo[i] == 'V' && (seq[i] != 'A' && seq[i] != 'C' && seq[i] != 'G'))	{	success = 0;	}			
				
				if(success == 0)	{	break;	 }
			}
			else{
				success = 1;
			}
		}
		
		return success;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "compareDNASeq");
		exit(1);
	}

}

//***************************************************************************************************************

int TrimSeqsCommand::countDiffs(string oligo, string seq){
	try {

		int length = oligo.length();
		int countDiffs = 0;
		
		for(int i=0;i<length;i++){
								
			if(oligo[i] != seq[i]){
				if(oligo[i] == 'A' || oligo[i] == 'T' || oligo[i] == 'G' || oligo[i] == 'C' || oligo[i] == '-' || oligo[i] == '.')	{	countDiffs++; 	}
				else if((oligo[i] == 'N' || oligo[i] == 'I') && (seq[i] == 'N'))				{	countDiffs++;	}
				else if(oligo[i] == 'R' && (seq[i] != 'A' && seq[i] != 'G'))					{	countDiffs++;	}
				else if(oligo[i] == 'Y' && (seq[i] != 'C' && seq[i] != 'T'))					{	countDiffs++;	}
				else if(oligo[i] == 'M' && (seq[i] != 'C' && seq[i] != 'A'))					{	countDiffs++;	}
				else if(oligo[i] == 'K' && (seq[i] != 'T' && seq[i] != 'G'))					{	countDiffs++;	}
				else if(oligo[i] == 'W' && (seq[i] != 'T' && seq[i] != 'A'))					{	countDiffs++;	}
				else if(oligo[i] == 'S' && (seq[i] != 'C' && seq[i] != 'G'))					{	countDiffs++;	}
				else if(oligo[i] == 'B' && (seq[i] != 'C' && seq[i] != 'T' && seq[i] != 'G'))	{	countDiffs++;	}
				else if(oligo[i] == 'D' && (seq[i] != 'A' && seq[i] != 'T' && seq[i] != 'G'))	{	countDiffs++;	}
				else if(oligo[i] == 'H' && (seq[i] != 'A' && seq[i] != 'T' && seq[i] != 'C'))	{	countDiffs++;	}
				else if(oligo[i] == 'V' && (seq[i] != 'A' && seq[i] != 'C' && seq[i] != 'G'))	{	countDiffs++;	}	
			}
			
		}
		
		return countDiffs;
	}
	catch(exception& e) {
		m->errorOut(e, "TrimSeqsCommand", "countDiffs");
		exit(1);
	}

}

//***************************************************************************************************************
