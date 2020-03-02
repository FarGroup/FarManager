#pragma once

class AnsiGuard {

	bool m_bWasOem;

public:

	AnsiGuard()
	{
		m_bWasOem = !AreFileApisANSI();

		if ( m_bWasOem )
			SetFileApisToANSI();
	}

	~AnsiGuard()
	{
		if ( m_bWasOem )
			SetFileApisToOEM();
	}
};

class OemGuard {

	bool m_bWasAnsi;

public:

	OemGuard()
	{
		m_bWasAnsi = AreFileApisANSI();

		if ( m_bWasAnsi )
			SetFileApisToOEM();
	}

	~OemGuard()
	{
		if ( m_bWasAnsi )
			SetFileApisToANSI();
	}
};