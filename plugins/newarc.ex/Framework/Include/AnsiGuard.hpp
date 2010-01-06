#pragma once

class AnsiGuard {

public:

	AnsiGuard()
	{
		SetFileApisToANSI();
	}

	~AnsiGuard()
	{
		SetFileApisToOEM();
	}
};