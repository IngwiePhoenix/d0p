#include "d0p.hpp"
#include <getopt.h>
using namespace std;

string uniqid() {
	static const char alphanum[] =
		"0123456789"
		"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
		int stringLength = sizeof(alphanum) - 1;
		string keyOut;

	for(int i=0; i<13; i++) {
		keyOut += alphanum[rand() % stringLength];
	}
	return keyOut;
}

// structors
d0p::d0p(string *targetName, char *list[]) {
	this->targetName = targetName;
	//this->list = list;
}
d0p::~d0p() {
	delete    this->key;
	//delete    this->archive;
	delete    this->targetName;
	//free(this->list);
}


int d0p::wrap(YAML::Node *object) { 
	return 0;
}

YAML::Node *d0p::unwrap() { YAML::Node *node; return node; }
		
// tar work
int d0p::addFile(string fileName) { return 0; }
int d0p::addFiles(char *flist[]) { return 0; }
char * d0p::listFiles() { return NULL; }
int d0p::extractFile(string innerPath, string outerPath) { return 0; }
		
// actual methods
int d0p::createPackage() { return 0; }
int d0p::explainPackage() { return 0; }
int d0p::listPackage() { return 0; }
int d0p::extractPackage() { return 0; }

int main(int argc, char *argv[]) {
	//char *inputList = new char[];
	string targetName;
	int verbosity=VERBOSITY_NORMAL;
	int i=1;
	
	// flag check
	if(strcmp(argv[1], "--verbose") == 0 || strcmp(argv[1], "--quiet") == 0) {
		d0p_log("Something got triggered.");
		i=2; // skip switch
		if(strcmp(argv[1],"--verbose") == 0) {
			verbosity=VERBOSITY_VERBOSE;
		} else if(strcmp(argv[1],"--quiet") == 0) {
			verbosity=VERBOSITY_QUIET;
		}
	}
	
	for(; i<argc; i++) {
		cout << "Option: " << argv[i] << endl;
	}
	
	return 0;
}
