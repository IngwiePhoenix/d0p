#include "d0p.hpp"
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
d0p(string target, char *list[]) {
	this->targetName = target;
	this->list = list;
}
~d0p() {
	delete    this->key;
	delete    this->archive;
	delete    this->targetName;
	delete    this->outName;
	delete [] this->list;
}


int d0p::wrap(YAML::Node *object) { 
	return 0;
}

YAML::Node *d0p::unwrap() { YAML::Node *node; return node; }
		
		// tar work
		int addFile(string fileName);
		int addFiles(...);
		int listFiles();
		int extractFile(string innerPath, string outerPath);
		
		// actual methods
		int createPackage(); // call addFile and wrap several times
							 // use -C switch in order to specify base-path that will later be treatened as "./".
							 // otherwise use the specified path (typical tar behavior)
		int explainPackage(); // pretty-print information about the package (unwrap+display)
		int listPackage(); // use libarchive to list content
		int extractPackage(); // extract files to specified path, treatening it as "./" otherwuse use cwd

int main() {
	cout << "o3o" << endl;
}
