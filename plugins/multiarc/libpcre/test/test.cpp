#define PCRE_STATIC
#include "pcre++.h"
#include <iostream>
#include <string>
using namespace std;
using namespace PCRE;

#pragma comment(lib, "pcre.lib")

int main(int argc, char **argv)
{
	RegExp re(
		"^(?P<date>\\d\\d\\.\\d\\d\\.(\\d\\d)?\\d\\d)\\s+(?P<time>\\d\\d:\\d\\d)\\s+(?P<size>(\\d+\\xff?)+)\\s+(?P<name>[^\\s].*[^\\s])\\s*$",
		"ix");

	string input;
	while(getline(cin, input))
	{
		if(Match m = re.match(input.c_str()))
		{
			if(m["name"])
				cout << "---------------------" << endl << "name: " << m["name"] << endl;
			if(m["date"])
				cout << "date: " << m["date"] << endl;
			if(m["time"])
				cout << "time: " << m["time"] << endl;
			if(m["size"])
				cout << "size: " << m["size"] << endl;
		}
	}
	
	return 0;
}
