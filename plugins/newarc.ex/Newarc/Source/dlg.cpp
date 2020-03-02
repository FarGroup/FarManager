
///////

void OperationErrorList::AddError(const TCHAR* lpFileName)
{
	m_pErrors.add(StrDuplicate(lpFileName));
}

void OperationErrorList::Show()
{
	FarMenu menu(_T("Error list"));

	if ( m_pErrors.count() )
	{
		for (unsigned int i = 0; i < m_pErrors.count(); i++)
		{
			menu.Add(m_pErrors[i]);
		}

		menu.Run();
	}
}
