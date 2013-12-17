#ifndef HAVE_D0P_H
#define HAVE_D0P_H

#include <iostream>
#include <string>
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
		string  *targetName; // Name of the d0p file to read/write
		string  *outName; // name of the resulting extracted archive. If none, constructor will replace .d0p by .tgz
		char    *list[];
		
	public:
		// structors
		d0p(string *, char * []); // target, and possibly files
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
