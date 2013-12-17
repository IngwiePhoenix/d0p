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
#include <stdio.h>

#include "archive.h"
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
		string  *key; // random sequence, to seperate binary from header
		archive *archive; // libarchive instance, actual d0p object :)
		string  *targetName; // Path to the d0p file to read/write
		int		 verbosity;
		int		 ar_method;
		bool	 ar_exists;
		
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
	cout << "d0p " << D0P_VERSION << endl;
	cout << "By: Ingwie Phoenix <ingwie2000@googlemail.com>" << endl;
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


/*
	Possible command line syntax:

	d0p create package.d0p somefile.txt README.md source.c source.h
		Creates package.d0p with basic header.

	d0p create package.d0p package.d0p.yaml somefile.txt README.md source.c source.h
		the .yaml extension tells d0p that this is the actual header file. 
		The first time a yaml file appears with the same filename as the d0p file, then that is the header to use.
		But since .d0p.yaml is not very often, its pretty sure this is the only time youll need it.
		
	d0p create package.d0p --from-stdin somefile.txt README.md source.c source.h
		Use the YAML specified via stdin.

	d0p add package.d0p newFile.c
		Add a file
		
	d0p delete package.d0p newFile.c
		Delete a file, by its inner path in the archive.
		
	d0p create package.d0p package.yaml -C /home/Ingwie /home/Ingwie/somefile.txt /home/Ingwie/anotherfile.txt
		add somefile.txt and anotherfile.txt to the package.d0p, and treat /home/Ingwie as ./
		I.e.:
			$ d0p list package.d0p
			./somefile.txt
			./anotherfile.txt
			$ d0p create package.d0p package.yaml /home/Ingwie/somefile.txt /home/Ingwie/anotherfile.txt
			$ d0p list package.d0p
			/home/Ingwie/somefile.txt
			/home/Ingwie/anotherfile.txt
		
	d0p explain package.d0p
		Show the internal YAML, to show all the information that the package has.

	d0p xtract package.d0p
		Extract everything from the package. If a file(s) is specified, just that is extracted.
		
	d0p list package.d0p
		List the contents of the inner archive
		
	d0p wrap package.d0p archive.tar.gz archive.yaml
		Wrap the yaml around the archive.
		
	d0p unwrap package.d0p archive.tgz archive.yaml
		Extract the yaml and archive into seperated parts
		
	You can also use single letters as first arguments:
	d0p [a|c|d|e|l|u|x|w] [args...]
	
	If the first argument starts with two dashes instead, they will configure things
	
	d0p --version
	d0p --verbose
	d0p --quiet
	
	By default, d0p is semi-verbose.
	
	$ d0p unwrap package.d0p
	[d0p] Unwrapping into: package.yaml | package.tar.gz
	[d0p] Extracting header... - Done
	[d0p] Extracting Archive... - Done
	
	Note, that the lines are refreshed. This means that you don't have to stare at a big output. However, using --verbose, that'll be turned off.
	
	Example output of file extraction
	$ d0p xtract package.d0p
	[d0p] Obtaining information... - Done
	[d0p: 1/2] somefile.txt
	[d0p: 2/2] /var/log/somelog.log
	
	Example of pretty printing:
	$ d0p --quiet explain package.d0p
	name: drag0n
	category: core-update
	version: 0.5-dev
	owner: Ingwie Phoenix
	description: Meta package, in order to update everything.
	depends:
		- d0.cli
			- dialog
		- d0.php
		- d0.core
		- d0p
			- libd0p
		- d0.build
			- GNUstep
			- cmake
			- ninja
			- upx
			- make
			- autotools
			- d0.build-misc-libs
		- d0.gui
			- deskshell
				- libcef
				- nodejs
			- notify-me
	install dependencies: yes
	priority: "!"
	
	this above was an example from an actual drag0n package, which is ment to be installed via the package manager.

*/

#endif
