#include "Poco/Path.h"
#include "Poco/File.h"
#include <stdio.h>
using namespace Poco;
int main(int argc, char** argv)
{
	if(argc < 2)
		return 0;
	Path path(argv[1]);
	path.append("SC0000000001");
	printf("%s\n", path.toString().c_str());
	printf("is dir %s\n", path.isDirectory()?"true":"false");
	File file1("./file1");
	file1.copyTo("./file2");
	return 0;
}

