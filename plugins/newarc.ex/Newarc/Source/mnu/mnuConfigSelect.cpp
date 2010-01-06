#include "../dlg/dlgArchiveFilter.cpp"

void mnuConfigSelect()
{
	FarMenu menu(_M(MPluginTitle));

	menu.Add(_M(MConfigCommonTitle));
	menu.Add(_M(MCommandLinesAndParamsTitleMenu));
	menu.Add(_T("Filters setup"));

	while ( true )
	{
		int nResult = menu.Run();

		if ( nResult == 0 )
			dlgConfigure ();
		else
		
		if ( nResult == 1 )
			mnuCommandLinesAndParams ();
		else

		if ( nResult == 2 )
		{
			if ( dlgArchiveFilter(pManager->GetFilter()) )
			{
				string strFilters = Info.ModuleName;
				CutToSlash(strFilters);

				strFilters += _T("filters.ini");

				pManager->GetFilter()->Save(strFilters);
			}
		}
		else
		    break;
	}
		
}
