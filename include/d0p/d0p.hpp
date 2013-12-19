#ifndef HAVE_D0P_H
#define HAVE_D0P_H

#define D0P_VERSION "0.1"
#define VERBOSITY_NORMAL 0
#define VERBOSITY_QUIET -1
#define VERBOSITY_VERBOSE 1
#define AR_READING 0x1
#define AR_WRITING 0x2
#define dout cout << "[d0p] "

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <stdio.h>
#include <fcntl.h>

#include "archive.h"
#include "archive_entry.h"
#include "lzma.h"
#include "yaml-cpp/yaml.h"
#include "base64/base64.h"

using namespace std;

#define D0P_OK		0x1
#define D0P_FAIL	0x2

// clone of php::uniqid()
// return 13byte string, random, alphanumeric
string uniqid();

class d0p {
	private:
		string  *key; // random sequence, to separate binary from header
		archive *archive; // libarchive instance, actual d0p object :)
		string  *targetName; // Path to the d0p file to read/write
		int		 verbosity;
		int		 ar_method;
		bool	 ar_exists;
		
		// Very important array; it stores a list of everything we will use as input files.
		vector<string> list;
		
		// private methods
		int prepairWrite();
		void endWrite();
		int prepairRead();
		
	public:
		// structors
		d0p(string *, int); // target, and possibly files
		~d0p(); // clean everything away and do some garbage collection
		
		// header work: wrap and unwrap the archive.
		int wrap(YAML::Node *object); // Wrap object around this->archive, writing it as filename:this->targetName
		YAML::Node *unwrap(); // Extract the yaml headers and remove them from the file on-disk, re-generating the archive as this->outName
		
		// tar work
		int addFile(string fileName);
		int addFiles(char *flist[]);
		char * listFiles();
		int extractFile(string innerPath, string outerPath);
		
		// actual methods
		int createTarXZ(); // Create a plain .tar.xz archive.
		int createPackage(); // call addFile and wrap several times
							 // use -C switch in order to specify base-path that will later be treatened as "./".
							 // otherwise use the specified path (typical tar behavior)
		int explainPackage(); // pretty-print information about the package (unwrap+display)
		int listPackage(); // use libarchive to list content
		int extractPackage(); // extract files to specified path, treatening it as "./" otherwuse use cwd
};

// No-return function - bottom.
void d0p_log(const char *msg) { cout << "[d0p] " << msg << endl; }
void d0p_prog(const char *msg, int max, int cur) { cout << "[d0p: " << max << "/" << cur << "] " << msg << endl; }
void version() { cout << D0P_VERSION << endl; }
void help() {
	cout << "d0p " << D0P_VERSION << " | By: Ingwie Phoenix <ingwie2000@googlemail.com>" << endl;
	cout << endl;
	cout << "Usage: d0p [verb] [args]" << endl;
	cout << endl;
	cout << "Verbs include:" << endl;
	cout << "    create [-c rootDir] $newPackage.d0p --header=file file1 file2 file3 ..." << endl;
	cout << "        - Create a package" << endl;
	cout << "    create-flat [-c rootDir] $newPackage.tar.xz file1 file2 file3 ..." << endl;
	cout << "        - Create a tar.xz archive, used as the default by d0p. Use the offical XZ Utils to manipulate those." << endl;
	cout << "    add [-c rootDir] $targetPackage.d0p file1 file2 file3 ..." << endl;
	cout << "        - Add a file to a package" << endl;
	cout << "    delete $targetPackage.d0p innerPath1 innerPath2 innerPath3 ..." << endl;
	cout << "        - Delete a file within an archive by its inner path" << endl;
	cout << "    xtract [-c rootDir] $targetPackage.d0p" << endl;
	cout << "        - Extract all files of archive into current folder" << endl;
	cout << "          Optionally you can add -c to specify the 'destination root'. So if you want to unpack into /usr/local, run:" << endl;
	cout << "               $ d0p xtract -c /usr/local package.d0p" << endl;
	cout << "    list $targetPackage.d0p" << endl;
	cout << "        - List contents of packaged archive" << endl;
	cout << "    explain $targetPackage.d0p" << endl;
	cout << "        - Show the package header. As this will output valid YAML, you may pipe it into another programm:" << endl;
	cout << "               $ d0p explain package.d0p | myProgramm -" << endl;
	cout << "           Or redirect the output into a new file, to save the header." << endl;
	cout << "     wrap $newPackage.d0p --header=file --archive=archive.tar" << endl;
	cout << "        - Wrap an archive and header into a package." << endl;
	cout << "          Note, that d0p uses tar.xz by default. But since it utilizes libarchive, it can handle everything." << endl;
	cout << "          We encourage you to use create-flat to get a compatible archive." << endl;
	cout << "     unwrap $targetPackage.d0p [--out-header=file] [--out-archive=file]" << endl;
	cout << "        - Unwrap the header, and produce the two original files - archive and header." << endl;
	cout << "          If no out-header or out-archive is given, we'll use the package's basename and add .yaml and .tar.xz to it." << endl;
	cout << "     help" << endl;
	cout << "        - This help." << endl;
	cout << endl;
	cout << "If you supply --verbose or --quiet before any verb, d0p will become either very verbose, or very quiet." << endl;
	cout << "Based on: " << endl;
	cout << "  " << ARCHIVE_VERSION_STRING << endl;
	cout << "  liblzma (from XZ Utils) " << LZMA_VERSION_STRING << endl;
	cout << "  base64 AND yaml-cpp via CVS" << endl;
}
std::string get_working_path() {
   char temp[255];
   return ( getcwd(temp, 255) ? std::string( temp ) : std::string("") );
}
int file_exists(char *name);
int file_exists(char *name) {
	FILE *file = fopen(name, "r");
    if (file) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

#endif
