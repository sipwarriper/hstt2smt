#include "fileencoder.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include "errors.h"
#include <iterator>
#include <sys/types.h>
#include <dirent.h>
#include <cstdlib>

using namespace std;


FileEncoder::FileEncoder(Encoding * encoding) : Encoder(encoding){
	tmpfilename = "aux.txt";
	defaulttmpdir = true;
}

FileEncoder::~FileEncoder(){

}

string FileEncoder::getTMPFileName() const{
	string s;
	if(defaulttmpdir){
#ifdef TMPFILESPATH
		DIR* dir = opendir(TMPFILESPATH);
		if (dir)
			closedir(dir);
		else if (ENOENT == errno)
		{
			const int dir_err = system((string("mkdir -p ") + TMPFILESPATH).c_str());
			if (-1 == dir_err){
				cerr << "Could not create directory " << TMPFILESPATH << endl;
				exit(1);
			}
		}
		else{
			cerr << "Could not acess nor create directory " << TMPFILESPATH << endl;
			exit(BADFILE_ERROR);
		}
		s=TMPFILESPATH"/";
#else
		s="";
#endif
	}
	else
		s="";

	s+=tmpfilename;
	return s;
}


void FileEncoder::setTmpFileName(const string & filename){
	this->tmpfilename = filename;
}

