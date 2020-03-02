#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

int wmain(int argc, wchar_t* argv[])
{
	if(argc != 4)
	{
		const auto NamePtr = wcsrchr(argv[0], L'\\');
		std::wcout << L"Usage:\n" << (NamePtr? NamePtr+1 : argv[0]) << L" input_template_file output_template_file new_lng_file" << std::endl;
		return -1;
	}

	const std::wstring InFeedName = argv[1], OutFeedName = argv[2], LngName = argv[3];
	
	std::wifstream Feed(InFeedName), Lng(LngName);

	std::wcout << L"Reading " << LngName << std::endl;

	std::wstring LngHeader;
	std::getline(Lng, LngHeader);

	std::list<std::wstring> LngLines;

	std::wstring Buffer;
	while(!Lng.eof())
	{
		std::getline(Lng, Buffer);
		if(!Buffer.compare(0, 1, L"\""))
		{
			LngLines.push_back(Buffer);
		}
	}

	std::wcout << L"Reading " << InFeedName << std::endl;

	std::list<std::wstring> FeedLines;

	while(!Feed.eof())
	{
		getline(Feed, Buffer);
		FeedLines.push_back(Buffer);
	}

	size_t ConstsCount = 0;
	for(auto i = FeedLines.begin(); i != FeedLines.end(); ++i)
	{
		// assume that all constants starts with 'M'.
		if(!i->compare(0, 1, L"M"))
		{
			++ConstsCount;
		}
	}

	if(ConstsCount != LngLines.size())
	{
		std::wcerr << L"Error: lines count mismatch: " << InFeedName << " - " << ConstsCount << L", " << LngName << L" - " << LngLines.size() << std::endl;
		return -1;
	}

	if(FeedLines.back().empty())
	{
		FeedLines.pop_back();
	}

	auto Ptr = FeedLines.begin();

	if(!Ptr->compare(0, 14, L"\xef\xbb\xbfm4_include("))
	{
		++Ptr;
	}

#define SKIP_EMPTY_LINES_AND_COMMENTS while(Ptr->empty() || !Ptr->compare(0, 1, L"#")) {++Ptr;}

	SKIP_EMPTY_LINES_AND_COMMENTS

	// skip header name
	++Ptr;

	SKIP_EMPTY_LINES_AND_COMMENTS

	std::wstringstream strStream(*Ptr);
	size_t Num;
	strStream >> Num;
	std::wcout << Num << L" languages found." << std::endl;

	auto NumPtr = Ptr;
	++Ptr;

	bool Update = false;
	size_t UpdateIndex = Num;

	for(size_t i = 0; i < Num; ++i, ++Ptr)
	{
		SKIP_EMPTY_LINES_AND_COMMENTS
		if(!Ptr->compare(0, LngName.length(), LngName))
		{
			Update = true;
			UpdateIndex = i;
			break;
		}
	}

	if(Update)
	{
		std::wcout << LngName << " already exist (id == " << UpdateIndex << L"). Updating." << std::endl;
	}
	else
	{
		std::wcout << L"Inserting new language (id == " << UpdateIndex << L") from " << LngName << std::endl;
		strStream.clear();
		strStream << Num+1;
		*NumPtr = strStream.str();

		std::wstring ShortLngName = LngHeader.substr(LngHeader.find(L'=', 0)+1);
		ShortLngName.resize(ShortLngName.find(L','), 0);

		std::wstring FullLngName = LngHeader.substr(LngHeader.find(L',', 0)+1);
		FeedLines.insert(Ptr, LngName+L" " + ShortLngName + L" \"" + FullLngName + L"\"");
	}

	for(auto i = LngLines.begin(); i != LngLines.end(); ++i)
	{
		// assume that all constants start with 'M'.
		while(Ptr->compare(0, 1, L"M"))
		{
			++Ptr;
		}
		++Ptr;

		for(size_t j = 0; j < UpdateIndex || !UpdateIndex; ++j)
		{
			while(Ptr != FeedLines.end() && Ptr->compare(0, 1, L"\"") && Ptr->compare(0, 5, L"upd:\""))
			{
				++Ptr;
			}
			if(!UpdateIndex)
			{
				break;
			}
			++Ptr;
		}

		if(Update)
		{
			const wchar_t* Str = Ptr->c_str();
			size_t l = Ptr->length();
			if(!Ptr->compare(0, 4, L"upd:"))
			{
				Str += 4;
				l -= 4;
			}
			if(i->compare(0, l, Str))
			{
				*Ptr = *i;
			}
		}
		else
		{
			FeedLines.insert(Ptr, *i);
		}
	}

	std::wcout << L"Writing to " << OutFeedName << std::endl;

	std::wofstream oFeed(OutFeedName);

	for(auto i = FeedLines.begin(); i != FeedLines.end(); ++i)
	{
		oFeed << *i << L'\n';
	}

	std::wcout << L"Done." << std::endl;

	return 0;
}
