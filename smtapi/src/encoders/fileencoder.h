#ifndef FILEENCODER_DEFINITION
#define FILEENCODER_DEFINITION


#include <stdlib.h>
#include "encoder.h"
#include "encoding.h"
#include <cstring>

using namespace std;
using namespace smtapi;

/*
 * Virtual class from which all file encoders inherit
 */
class FileEncoder : public Encoder {

private:
	string tmpfilename;
	bool defaulttmpdir;

protected:
	string getTMPFileName() const;

public:

	//Default constructor
	FileEncoder(Encoding * encoding);

	//Destructor
	virtual ~FileEncoder();

	virtual void createFile(ostream & os, SMTFormula * f) const = 0;

	void setTmpFileName(const string & filename);



};

#endif

