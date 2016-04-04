#include "Poco/Path.h"
#include "Poco/File.h"
#include <stdio.h>
using namespace Poco;
int main(int argc, char** argv)
{
	if(argc < 2)
		return 0;
	Path path(argv[1]);
	printf("%s is directory %s\n", path.toString().c_str(), path.isDirectory()?"true":"false");
	return 0;
}

