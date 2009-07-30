/*
 *  pintail.cpp
 *  Mothur
 *
 *  Created by Sarah Westcott on 7/9/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */

#include "pintail.h"
#include "ignoregaps.h"

//***************************************************************************************************************

Pintail::Pintail(string filename, string temp) {  fastafile = filename;  templateFile = temp;  }
//***************************************************************************************************************

Pintail::~Pintail() {
	try {
		for (int i = 0; i < querySeqs.size(); i++)		{  delete querySeqs[i];		}
		for (int i = 0; i < templateSeqs.size(); i++)	{  delete templateSeqs[i];	}
		
		if (processors != 1) {   for (int i = 0; i < bestfit.size(); i++)	{  delete bestfit[i];	}  }
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "~Pintail");
		exit(1);
	}
}	
//***************************************************************************************************************
void Pintail::print(ostream& out) {
	try {
		
		for (int i = 0; i < querySeqs.size(); i++) {
			
			int index = ceil(deviation[i]);
float quan = 2.64 * log10(deviation[i]) + 1.46;
cout << "dist = " << index << endl;
cout << "de = "  << DE[i] << endl;
cout << "mallard quantile = " << quan << endl;
cout << "my quantile = " << quantiles[index][4] << endl;
			
			//is your DE value higher than the 95%
			string chimera;
			if (DE[i] > quantiles[index][4])	{	chimera = "Yes";	}
			else								{	chimera = "No";		}
			
			out << querySeqs[i]->getName() << '\t' << "div: " << deviation[i] << "\tstDev: " << DE[i] << "\tchimera flag: " << chimera << endl;
			if (chimera == "Yes") {
				mothurOut(querySeqs[i]->getName() + "\tdiv: " + toString(deviation[i]) + "\tstDev: " + toString(DE[i]) + "\tchimera flag: " + chimera); mothurOutEndLine();
			}
			out << "Observed\t";
			
			for (int j = 0; j < obsDistance[i].size(); j++) {  out << obsDistance[i][j] << '\t';  }
			out << endl;
			
			out << "Expected\t";
			
			for (int m = 0; m < expectedDistance[i].size(); m++) {  out << expectedDistance[i][m] << '\t';  }
			out << endl;
			
		}
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "print");
		exit(1);
	}
}

//***************************************************************************************************************
void Pintail::getChimeras() {
	try {
		
		//read in query sequences and subject sequences
		mothurOut("Reading sequences and template file... "); cout.flush();
		querySeqs = readSeqs(fastafile);
		templateSeqs = readSeqs(templateFile);
		mothurOut("Done."); mothurOutEndLine();
		
		int numSeqs = querySeqs.size();
		
		obsDistance.resize(numSeqs);
		expectedDistance.resize(numSeqs);
		seqCoef.resize(numSeqs);
		DE.resize(numSeqs);
		Qav.resize(numSeqs);
		bestfit.resize(numSeqs);
		deviation.resize(numSeqs);
		trimmed.resize(numSeqs);
		windowSizes.resize(numSeqs, window);
		windowSizesTemplate.resize(templateSeqs.size(), window);
		windowsForeachQuery.resize(numSeqs);
		h.resize(numSeqs);
		quantiles.resize(100);  //one for every percent mismatch
		
		//break up file if needed
		int linesPerProcess = numSeqs / processors ;
		
		#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
			//find breakup of sequences for all times we will Parallelize
			if (processors == 1) {   lines.push_back(new linePair(0, numSeqs));  }
			else {
				//fill line pairs
				for (int i = 0; i < (processors-1); i++) {			
					lines.push_back(new linePair((i*linesPerProcess), ((i*linesPerProcess) + linesPerProcess)));
				}
				//this is necessary to get remainder of processors / numSeqs so you don't miss any lines at the end
				int i = processors - 1;
				lines.push_back(new linePair((i*linesPerProcess), numSeqs));
			}
			
			//find breakup of templatefile for quantiles
			if (processors == 1) {   templateLines.push_back(new linePair(0, templateSeqs.size()));  }
			else { 
				for (int i = 0; i < processors; i++) {
					templateLines.push_back(new linePair());
					templateLines[i]->start = int (sqrt(float(i)/float(processors)) * templateSeqs.size());
					templateLines[i]->end = int (sqrt(float(i+1)/float(processors)) * templateSeqs.size());
				}
			}
		#else
			lines.push_back(new linePair(0, numSeqs));
			templateLines.push_back(new linePair(0, templateSeqs.size()));
		#endif
		
		distcalculator = new ignoreGaps();
		decalc = new DeCalculator();
		
		decalc->setMask(seqMask);
		
		//find pairs
		if (processors == 1) { 
			mothurOut("Finding closest sequence in template to each sequence... "); cout.flush();
			bestfit = findPairs(lines[0]->start, lines[0]->end);
			mothurOut("Done."); mothurOutEndLine();
		}else {		createProcessesPairs();		}
		
		
		for (int j = 0; j < bestfit.size(); j++) { 
				//chops off beginning and end of sequences so they both start and end with a base
				ofstream out;
				string s = querySeqs[j]->getName();
				
				openOutputFile(s, out);
				out << ">" << querySeqs[j]->getName() << endl;
				out << querySeqs[j]->getAligned() << endl;
				out.close();
				
				string t =querySeqs[j]->getName() + ".ref";
				openOutputFile(t, out);
				out << ">" << bestfit[j]->getName() << endl;
				out << bestfit[j]->getAligned() << endl;
				out.close();
		}

		
		//find P
		mothurOut("Getting conservation... "); cout.flush();
		if (consfile == "") { 
			mothurOut("Calculating probability of conservation for your template sequences.  This can take a while...  I will output the frequency of the highest base in each position to a .freq file so that you can input them using the conservation parameter next time you run this command.  Providing the .freq file will improve speed.    "); cout.flush();
			probabilityProfile = decalc->calcFreq(templateSeqs, templateFile); 
			mothurOut("Done."); mothurOutEndLine();
		}else				{   probabilityProfile = readFreq();			  }

		//make P into Q
		for (int i = 0; i < probabilityProfile.size(); i++)  {	probabilityProfile[i] = 1 - probabilityProfile[i];   }  //cout << i << '\t' << probabilityProfile[i] << endl;
		mothurOut("Done."); mothurOutEndLine();
		
		//mask querys
		for (int i = 0; i < querySeqs.size(); i++) {
		//cout << querySeqs[i]->getName() << " before mask = " << querySeqs[i]->getAligned() << endl << endl;
			decalc->runMask(querySeqs[i]);
		//cout << querySeqs[i]->getName() << " after mask = " << querySeqs[i]->getAligned() << endl << endl;
		}
		
		//mask templates
		for (int i = 0; i < templateSeqs.size(); i++) {
			decalc->runMask(templateSeqs[i]);
		}
		
//for (int i = 0; i < lines.size(); i++) { cout << "line pair " << i << " = " << lines[i]->start << '\t' << lines[i]->end << endl;  }
				
		if (processors == 1) { 
	
			for (int j = 0; j < bestfit.size(); j++) { 
			cout << querySeqs[j]->getName() << " after mask = " << querySeqs[j]->getAligned() << endl << endl;
			cout << bestfit[j]->getName() << " after mask = " << bestfit[j]->getAligned() << endl << endl;
				decalc->trimSeqs(querySeqs[j], bestfit[j], trimmed[j]);  
			}
			
			mothurOut("Finding window breaks... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				it = trimmed[i].begin();
cout << i << '\t' << "trimmed = " << it->first << '\t' << it->second << endl;
				vector<int> win = decalc->findWindows(querySeqs[i], it->first, it->second, windowSizes[i], increment);
				windowsForeachQuery[i] = win;
			}
			mothurOut("Done."); mothurOutEndLine();
		
		}else {		createProcessesSpots();		}

		
		if (processors == 1) { 
						
			mothurOut("Calculating observed distance... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
	cout << querySeqs[i]->getName() << '\t' << bestfit[i]->getName() << " windows = " << windowsForeachQuery[i].size() << " size = " << windowSizes[i] << endl;
				vector<float> obsi = decalc->calcObserved(querySeqs[i], bestfit[i], windowsForeachQuery[i], windowSizes[i]);
	
	for (int j = 0; j < obsi.size(); j++) {
		cout << obsi[j] << '\t';
	}
	cout << endl;
				obsDistance[i] = obsi;
			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			mothurOut("Finding variability... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				vector<float> q = decalc->findQav(windowsForeachQuery[i], windowSizes[i], probabilityProfile);

				Qav[i] = q;
cout << i+1 << endl;
for (int j = 0; j < Qav[i].size(); j++) {
	cout << Qav[i][j] << '\t';
}
cout << endl << endl;

			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			mothurOut("Calculating alpha... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				float alpha = decalc->getCoef(obsDistance[i], Qav[i]);
cout << i+1 << "\tcoef = " << alpha << endl;
				seqCoef[i] = alpha;
			}
			mothurOut("Done."); mothurOutEndLine();
		
		
			mothurOut("Calculating expected distance... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				vector<float> exp = decalc->calcExpected(Qav[i], seqCoef[i]);
				expectedDistance[i] = exp;
			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			mothurOut("Finding deviation... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				float de = decalc->calcDE(obsDistance[i], expectedDistance[i]);
				DE[i] = de;
cout << querySeqs[i]->getName() << '\t' << "de value = " << de << endl;				
				it = trimmed[i].begin();
				float dist = decalc->calcDist(querySeqs[i], bestfit[i], it->first, it->second); 
cout << querySeqs[i]->getName() << '\t' << "dist value = " << dist << endl;
				deviation[i] = dist;
			}
			mothurOut("Done."); mothurOutEndLine();
			
		} 
		else {		createProcesses();		}
		
		
		//quantiles are used to determine whether the de values found indicate a chimera
		//if you have to calculate them, its time intensive because you are finding the de and deviation values for each 
		//combination of sequences in the template
		if (quanfile != "") {  quantiles =  readQuantiles();  }
		else {
			
			mothurOut("Calculating quantiles for your template.  This can take a while...  I will output the quantiles to a .quan file that you can input them using the quantiles parameter next time you run this command.  Providing the .quan file will dramatically improve speed.    "); cout.flush();
			if (processors == 1) { 
				quantiles = decalc->getQuantiles(templateSeqs, windowSizesTemplate, window, probabilityProfile, increment, 0, templateSeqs.size());
			}else {		createProcessesQuan();		}
			
			
			decalc->removeObviousOutliers(quantiles);
			
			ofstream out4;
			string o = getRootName(templateFile) + "quan";
			
			openOutputFile(o, out4);
			
			//adjust quantiles
			for (int i = 0; i < quantiles.size(); i++) {
				if (quantiles[i].size() == 0) {
					//in case this is not a distance found in your template files
					for (int g = 0; g < 6; g++) {
						quantiles[i].push_back(0.0);
					}
				}else{
					
					sort(quantiles[i].begin(), quantiles[i].end());
					
					vector<float> temp;
					//save 10%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.10)]);
					//save 25%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.25)]);
					//save 50%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.5)]);
					//save 75%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.75)]);
					//save 95%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.95)]);
					//save 99%
					temp.push_back(quantiles[i][int(quantiles[i].size() * 0.99)]);
					
					quantiles[i] = temp;
				}
				
				//output quan value
				out4 << i+1 << '\t';				
				for (int u = 0; u < quantiles[i].size(); u++) {   out4 << quantiles[i][u] << '\t'; }
				out4 << endl;

			}
			
			mothurOut("Done."); mothurOutEndLine();
		}
	
		//free memory
		for (int i = 0; i < lines.size(); i++)					{	delete lines[i];				}
		for (int i = 0; i < templateLines.size(); i++)			{	delete templateLines[i];		}
			
		delete distcalculator;
		delete decalc;
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "getChimeras");
		exit(1);
	}
}

//***************************************************************************************************************

vector<float> Pintail::readFreq() {
	try {
	
		ifstream in;
		openInputFile(consfile, in);
		
		vector<float> prob;
		set<int> h = decalc->getPos();  //positions of bases in masking sequence
		
		//read in probabilities and store in vector
		int pos; float num; 
		
		while(!in.eof()){
			
			in >> pos >> num;
			
			if (h.count(pos) > 0) {
				float Pi;
				Pi =  (num - 0.25) / 0.75; 
			
				//cannot have probability less than 0.
				if (Pi < 0) { Pi = 0.0; }

				//do you want this spot
				prob.push_back(Pi);  
			}
			
			gobble(in);
		}
		
		in.close();
		return prob;
		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "readFreq");
		exit(1);
	}
}

//***************************************************************************************************************

vector< vector<float> > Pintail::readQuantiles() {
	try {
	
		ifstream in;
		openInputFile(quanfile, in);
		
		vector< vector<float> > quan;
	
		int num; float ten, twentyfive, fifty, seventyfive, ninetyfive, ninetynine; 
		
		while(!in.eof()){
			
			in >> num >> ten >> twentyfive >> fifty >> seventyfive >> ninetyfive >> ninetynine; 
			
			vector <float> temp;
			
			temp.push_back(ten); 
			temp.push_back(twentyfive);
			temp.push_back(fifty);
			temp.push_back(seventyfive);
			temp.push_back(ninetyfive);
			temp.push_back(ninetynine);
			
			quan.push_back(temp);  
			
			gobble(in);
		}
		
		in.close();
		return quan;
		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "readQuantiles");
		exit(1);
	}
}
//***************************************************************************************************************
//calculate the distances from each query sequence to all sequences in the template to find the closest sequence
vector<Sequence*> Pintail::findPairs(int start, int end) {
	try {
		
		vector<Sequence*> seqsMatches;  
		
		for(int i = start; i < end; i++){
		
			float smallest = 10000.0;
			Sequence query = *(querySeqs[i]);
			Sequence* match;
			
			for(int j = 0; j < templateSeqs.size(); j++){
				
				Sequence temp = *(templateSeqs[j]);
				
				distcalculator->calcDist(query, temp);
				float dist = distcalculator->getDist();
				
				if (dist < smallest) { 
					match = templateSeqs[j];
					smallest = dist;
				}
			}
			
			seqsMatches.push_back(match);
		}
		
		return seqsMatches;
	
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "findPairs");
		exit(1);
	}
}

/**************************************************************************************************/

void Pintail::createProcessesSpots() {
	try {
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
		int process = 0;
		vector<int> processIDS;
		
		//loop through and create all the processes you want
		while (process != processors) {
			int pid = fork();
			
			if (pid > 0) {
				processIDS.push_back(pid);  
				process++;
			}else if (pid == 0){
				
				for (int j = lines[process]->start; j < lines[process]->end; j++) {
				
					//chops off beginning and end of sequences so they both start and end with a base
					map<int, int> trim;
					decalc->trimSeqs(querySeqs[j], bestfit[j], trim); 
					trimmed[j] = trim;
					
				}

				mothurOut("Finding window breaks for sequences " + toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					it = trimmed[i].begin();
					windowsForeachQuery[i] = decalc->findWindows(querySeqs[i], it->first, it->second, windowSizes[i], increment);
				}
				mothurOut("Done finding window breaks for sequences " + toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				
				//write out data to file so parent can read it
				ofstream out;
				string s = toString(getpid()) + ".temp";
				openOutputFile(s, out);
				
				//output windowsForeachQuery
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << windowsForeachQuery[i].size() << '\t';
					for (int j = 0; j < windowsForeachQuery[i].size(); j++) {
						out << windowsForeachQuery[i][j] << '\t';
					}
					out << endl;
				}
			
				//output windowSizes
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << windowSizes[i] << '\t';
				}
				out << endl;
				
				//output trimmed values
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					it = trimmed[i].begin();
					
					out << it->first << '\t' << it->second << endl;
				}
				out.close();
				
				exit(0);
			}else { mothurOut("unable to spawn the necessary processes."); mothurOutEndLine(); exit(0); }
		}
		
		//force parent to wait until all the processes are done
		for (int i=0;i<processors;i++) { 
			int temp = processIDS[i];
			wait(&temp);
		}
		
		//get data created by processes
		for (int i=0;i<processors;i++) { 
			ifstream in;
			string s = toString(processIDS[i]) + ".temp";
			openInputFile(s, in);
			
			int size = lines[i]->end - lines[i]->start;
					
			int count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				int num;
				in >> num;
			
				vector<int> win;  int w;
				for (int j = 0; j < num; j++) {
					in >> w;
					win.push_back(w);
				}
			
				windowsForeachQuery[count] = win;
				count++;
				gobble(in);
			}
		
			gobble(in);
			count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				int num;
				in >> num;
				
				windowSizes[count] = num;
				count++;
			}
			
			gobble(in);
			
			count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				int front, back;
				in >> front >> back;
				
				map<int, int> t;
				
				t[front] = back;
				
				trimmed[count] = t;
				count++;
				
				gobble(in);
			}

			
			in.close();
			remove(s.c_str());
		}
			
	
#else
		for (int j = 0; j < bestfit.size(); j++) {
			//chops off beginning and end of sequences so they both start and end with a base
			decalc->trimSeqs(querySeqs[j], bestfit[j], trimmed[j]);  
		}

		for (int i = lines[0]->start; i < lines[0]->end; i++) {
				it = trimmed[i].begin();
				map<int, int> win = decalc->findWindows(querySeqs[i], it->first, it->second, windowSizes[i], increment);
				windows[i] = win;
		}

#endif		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "createProcessesSpots");
		exit(1);
	}
}
/**************************************************************************************************/

void Pintail::createProcessesPairs() {
	try {
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
		int process = 0;
		vector<int> processIDS;
		
		//loop through and create all the processes you want
		while (process != processors) {
			int pid = fork();
			
			if (pid > 0) {
				processIDS.push_back(pid);  
				process++;
			}else if (pid == 0){
				
				mothurOut("Finding pairs for sequences " + toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				bestfit = findPairs(lines[process]->start, lines[process]->end);
				mothurOut("Done finding pairs for sequences " +  toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				
				//write out data to file so parent can read it
				ofstream out;
				string s = toString(getpid()) + ".temp";
				openOutputFile(s, out);
				
				//output range and size
				out << bestfit.size() << endl;
				
				//output pairs
				for (int i = 0; i < bestfit.size(); i++) {
					out << ">" << bestfit[i]->getName() << endl << bestfit[i]->getAligned() << endl;
				}
				out.close();
				
				exit(0);
			}else { mothurOut("unable to spawn the necessary processes."); mothurOutEndLine(); exit(0); }
		}
		
		//force parent to wait until all the processes are done
		for (int i=0;i<processors;i++) { 
			int temp = processIDS[i];
			wait(&temp);
		}
		
		//get data created by processes
		for (int i=0;i<processors;i++) { 
			ifstream in;
			string s = toString(processIDS[i]) + ".temp";
			openInputFile(s, in);
			
			int size;
			in >> size;  gobble(in);
				
			//get pairs
			int count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				Sequence* temp = new Sequence(in);
				bestfit[count] = temp;
			
				count++;
				gobble(in);
			}
			
			in.close();
			remove(s.c_str());
		}
			
	
#else
		bestfit = findPairs(lines[0]->start, lines[0]->end);
#endif		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "createProcessesPairs");
		exit(1);
	}
}
/**************************************************************************************************/

void Pintail::createProcesses() {
	try {
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
		int process = 0;
		vector<int> processIDS;
		
		//loop through and create all the processes you want
		while (process != processors) {
			int pid = fork();
			
			if (pid > 0) {
				processIDS.push_back(pid);  
				process++;
			}else if (pid == 0){
				
				mothurOut("Calculating observed, expected and de values for sequences " + toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					
					vector<float> obsi = decalc->calcObserved(querySeqs[i], bestfit[i], windowsForeachQuery[i], windowSizes[i]);
					obsDistance[i] = obsi;
				
					//calc Qav
					vector<float> q = decalc->findQav(windowsForeachQuery[i], windowSizes[i], probabilityProfile);
					
					//get alpha
					float alpha = decalc->getCoef(obsDistance[i], q);
					
					//find expected
					vector<float> exp = decalc->calcExpected(q, alpha);
					expectedDistance[i] = exp;
					
					//get de and deviation
					float dei = decalc->calcDE(obsi, exp);
					DE[i] = dei;
					
					it = trimmed[i].begin();
					float dist = decalc->calcDist(querySeqs[i], bestfit[i], it->first, it->second); 
					deviation[i] = dist;
				}
				mothurOut("Done calculating observed, expected and de values for sequences " + toString(lines[process]->start) + " to " + toString(lines[process]->end)); mothurOutEndLine();
				
				//write out data to file so parent can read it
				ofstream out;
				string s = toString(getpid()) + ".temp";
				openOutputFile(s, out);
				
				int size = lines[process]->end - lines[process]->start;
				out << size << endl;
								
				//output observed distances
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << obsDistance[i].size() << '\t';
					for (int j = 0; j < obsDistance[i].size(); j++) {
						out << obsDistance[i][j] << '\t';
					}
					out << endl;
				}
				
				
				//output expected distances
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << expectedDistance[i].size() << '\t';
					for (int j = 0; j < expectedDistance[i].size(); j++) {
						out << expectedDistance[i][j] << '\t';
					}
					out << endl;
				}

			
				//output de values
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << DE[i] << '\t';
				}
				out << endl;	
				
				//output de values
				for (int i = lines[process]->start; i < lines[process]->end; i++) {
					out << deviation[i] << '\t';
				}
				out << endl;
				
				out.close();

				exit(0);
			}else { mothurOut("unable to spawn the necessary processes."); mothurOutEndLine(); exit(0); }
		}
		
		//force parent to wait until all the processes are done
		for (int i=0;i<processors;i++) { 
			int temp = processIDS[i];
			wait(&temp);
		}
		
		//get data created by processes
		for (int i=0;i<processors;i++) { 
			ifstream in;
			string s = toString(processIDS[i]) + ".temp";
			openInputFile(s, in);
			
			int size;
			in >> size;  gobble(in);
			
			//get observed distances
			int count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				int num;
				in >> num;
			
				vector<float> obs;  float w;
				for (int j = 0; j < num; j++) {
					in >> w;
					obs.push_back(w);
				}
			
				obsDistance[count] = obs;
				count++;
				gobble(in);
			}
			
			gobble(in);
			
			//get expected distances
			count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				int num;
				in >> num;
			
				vector<float> exp;  float w;
				for (int j = 0; j < num; j++) {
					in >> w;
					exp.push_back(w);
				}
			
				expectedDistance[count] = exp;
				count++;
				gobble(in);
			}

			gobble(in);
			
			count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				float num;
				in >> num;
				
				DE[count] = num;
				count++;
			}
			
			gobble(in);
			
			count = lines[i]->start;
			for (int m = 0; m < size; m++) {
				float num;
				in >> num;
				
				deviation[count] = num;
				count++;
			}

			in.close();
			remove(s.c_str());
		}

				
#else
			mothurOut("Calculating observed distance... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				vector<float> obsi = decalc->calcObserved(querySeqs[i], bestfit[i], windows[i], windowSizes[i]);
				obsDistance[i] = obsi;
			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			
			mothurOut("Finding variability... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				vector<float> q = decalc->findQav(windows[i], windowSizes[i], probabilityProfile, h[i]);
				Qav[i] = q;
			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			
			mothurOut("Calculating alpha... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				float alpha = decalc->getCoef(obsDistance[i], Qav[i]);
				seqCoef.push_back(alpha);
			}
			mothurOut("Done."); mothurOutEndLine();
		
		
		
			mothurOut("Calculating expected distance... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				vector<float> exp = decalc->calcExpected(Qav[i], seqCoef[i]);
				expectedDistance[i] = exp;
			}
			mothurOut("Done."); mothurOutEndLine();
			
			
			
			mothurOut("Finding deviation... "); cout.flush();
			for (int i = lines[0]->start; i < lines[0]->end; i++) {
				float de = decalc->calcDE(obsDistance[i], expectedDistance[i]);
				DE[i] = de;
				
				it = trimmed[i].begin();
				float dist = decalc->calcDist(querySeqs[i], bestfit[i], it->first, it->second); 
				deviation[i] = dist;
			}
			mothurOut("Done."); mothurOutEndLine();

#endif		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "createProcesses");
		exit(1);
	}
}


/**************************************************************************************************/

void Pintail::createProcessesQuan() {
	try {
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux)
		int process = 0;
		vector<int> processIDS;
				
		//loop through and create all the processes you want
		while (process != processors) {
			int pid = fork();
			
			if (pid > 0) {
				processIDS.push_back(pid);  
				process++;
			}else if (pid == 0){
				
				quantiles = decalc->getQuantiles(templateSeqs, windowSizesTemplate, window, probabilityProfile, increment, templateLines[process]->start, templateLines[process]->end);
				
				//write out data to file so parent can read it
				ofstream out;
				string s = toString(getpid()) + ".temp";
				openOutputFile(s, out);
				
								
				//output observed distances
				for (int i = 0; i < quantiles.size(); i++) {
					out << quantiles[i].size() << '\t';
					for (int j = 0; j < quantiles[i].size(); j++) {
						out << quantiles[i][j] << '\t';
					}
					out << endl;
				}
				
				out.close();
				
				exit(0);
			}else { mothurOut("unable to spawn the necessary processes."); mothurOutEndLine(); exit(0); }
		}
		
		//force parent to wait until all the processes are done
		for (int i=0;i<processors;i++) { 
			int temp = processIDS[i];
			wait(&temp);
		}
		
		//get data created by processes
		for (int i=0;i<processors;i++) { 
			ifstream in;
			string s = toString(processIDS[i]) + ".temp";
			openInputFile(s, in);
			
			vector< vector<float> > quan; quan.resize(100);
			
			//get quantiles
			for (int m = 0; m < quan.size(); m++) {
				int num;
				in >> num;
			
				vector<float> q;  float w;
				for (int j = 0; j < num; j++) {
					in >> w;
					q.push_back(w);
				}
			
				quan[m] = q;
				gobble(in);
			}
			
			
			//save quan in quantiles
			for (int i = 0; i < quan.size(); i++) {
				//put all values of q[i] into quan[i]
				quantiles[i].insert(quantiles[i].begin(), quan[i].begin(), quan[i].end());
			}
					
			in.close();
			remove(s.c_str());
		}
		
#else
		quantiles = decalc->getQuantiles(templateSeqs, windowSizesTemplate, window, probabilityProfile, increment, 0, templateSeqs.size());
#endif		
	}
	catch(exception& e) {
		errorOut(e, "Pintail", "createProcessesQuan");
		exit(1);
	}
}


//***************************************************************************************************************

