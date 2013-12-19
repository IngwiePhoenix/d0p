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
d0p::d0p(string *targetName, int vbs) {
	this->targetName = targetName;
	this->verbosity = vbs;
	
	// c-conform
	char *cTargetName = (char*)targetName->c_str();
	
	// Now, relly init the target.
	if(vbs == VERBOSITY_VERBOSE) dout << "Target: " << targetName->c_str() << endl;
	if(file_exists(cTargetName)) {
		if(vbs == VERBOSITY_VERBOSE) dout << "Target exists." << endl;
		this->ar_exists=true;
	} else {
		if(vbs == VERBOSITY_VERBOSE) dout << "Target does not exist, allocate in memory." << endl;
		this->ar_exists=false;
	}
}
d0p::~d0p() {
	delete    this->key;
	delete    this->targetName;
	if(this->ar_method == AR_READING)
		archive_read_free(this->archive);
	else if(this->ar_method == AR_WRITING)
		archive_write_free(this->archive);
}

int d0p::prepairWrite() {
	// If we create, we need to open our archive in write mode.
	this->archive = archive_write_new();
	archive_write_add_filter_xz(this->archive); // strong.
	return archive_write_open_filename(this->archive, (char*)targetName->c_str());
}
void d0p::endWrite() {
	archive_write_close(this->archive);
	archive_write_free(this->archive);
}


int d0p::wrap(YAML::Node *object) { 
	return 0;
}

YAML::Node *d0p::unwrap() { YAML::Node *node; return node; }
		
// tar work
int d0p::addFile(string fileName) {
	cout << "File: " << fileName << endl;
	this->list.push_back(fileName);
	return 0;
}
int d0p::addFiles(char *flist[]) { return 0; }
char * d0p::listFiles() { return NULL; }
int d0p::extractFile(string innerPath, string outerPath) { return 0; }
		
// actual methods
int d0p::createTarXZ() {
	if(this->verbosity == VERBOSITY_VERBOSE) dout << "Creating target tar.xz" << endl;
	archive_entry *entry;
  	struct stat st;
  	char buff[8192]; // 8 byte
  	int len;
  	FILE *fd;
  	
  	// Now, open an archive
  	this->prepairWrite();
  	
	for(std::vector<int>::size_type i = 0; i != this->list.size(); i++) {
		// string -> char*
		char *filename = (char*)this->list[i].c_str();
    	stat(filename, &st);
    	entry = archive_entry_new();
    	
    	archive_entry_set_pathname(entry, filename);
    	archive_entry_set_size(entry, st.st_size);
    	
    	cerr << "File " << filename << " has size " << st.st_size << endl;
    	
    	archive_entry_set_filetype(entry, AE_IFREG);
    	archive_entry_set_perm(entry, st.st_mode);
    	
    	archive_write_header(this->archive, entry);

    	// stream from file to archive
    	fd = fopen(filename, "r");
    	if (fd==NULL) {fputs("File error",stderr); exit (1);}
    	
    	// stream char to archive
    	while(!feof(fd)) {
    		size_t bytes_read = fread(&buff, sizeof(buff), 1, fd);
    		cerr << "Read some bytes:" << bytes_read << endl;
    		archive_write_data(this->archive, &buff, sizeof(buff)); // <-------
    	}
    	fclose(fd);
    	
    	archive_entry_free(entry);
	}
	
	// free and dealloc it
	this->endWrite();
	
	return D0P_OK;
}
int d0p::explainPackage() { return 0; }
int d0p::listPackage() { return 0; }
int d0p::extractPackage() { return 0; }

int main(int argc, char *argv[]) {
	string targetName;
	int verbosity=VERBOSITY_NORMAL;
	int i=1; // 0 is the program itself - and thus useless if called via $PATH
	d0p *$d0p;
	
	#define stillHasArgs (i<argc)
		
	// flag check
	if(stillHasArgs && (strcmp(argv[1], "--verbose") == 0 || strcmp(argv[1], "--quiet") == 0)) {
		i=2; // skip switch
		if(strcmp(argv[1],"--verbose") == 0) {
			d0p_log("Going verbose");
			verbosity=VERBOSITY_VERBOSE;
		} else if(strcmp(argv[1],"--quiet") == 0) {
			verbosity=VERBOSITY_QUIET;
		}
	}

	// Parse arguments
	if(stillHasArgs){
		char *verb = argv[i];
		i++;
		if(strcmp(verb,"help") == 0) { help(); exit(0); }
		if(strcmp(verb,"version") == 0) { version(); exit(0); }
		if(strcmp(verb,"create-flat") == 0 && i+1<argc) {
			string t = string(argv[i]);
			$d0p = new d0p(&t,verbosity);
			string rootPath;
			i++;
			if(!stillHasArgs) {
				d0p_log("No file(s) specified");
				exit(1);
			}
			if(strcmp(argv[i],"-c") == 0) {
				rootPath = argv[i+1];
				i=i+2;
			} else {
				rootPath = get_working_path();
			}
			chdir(rootPath.c_str());
			for(; i<argc; i++) $d0p->addFile(argv[i]);
			return $d0p->createTarXZ();
		}
	} else help();
	
	free($d0p);
	return 0;
}
