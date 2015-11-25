#include "Poco/SHA1Engine.h"
#include <stdio.h>
int main(int argc, char** argv)
{
	Poco::SHA1Engine sha1;
	std::string name = "huangjian";
	sha1.update(name);
	const Poco::DigestEngine::Digest& digest = sha1.digest();
	std::string sha1digest(Poco::DigestEngine::digestToHex(digest));
	printf("%d %s\n",sha1digest.length(), sha1digest.c_str());
	return 0;
}

