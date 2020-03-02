#include "../dlg/dlgArchiveFilter.cpp"

void mnuConfigSelect(ArchiveManagerConfig* pCfg)
{
	FarMenu menu(_M(MPluginTitle));

	menu.Add(_M(MConfigCommonTitle));
	menu.Add(_M(MCommandLinesAndParamsTitleMenu));
	menu.Add(_T("Filters setup"));

	while ( true )
	{
		int nResult = menu.Run();

		if ( nResult == 0 )
			dlgConfigure(pCfg);
		else
		
		if ( nResult == 1 )
			mnuCommandLinesAndParams(pCfg);
		else

		if ( nResult == 2 )
		{
			if ( dlgArchiveFilter(pCfg, pCfg->GetFilter()) )
				pCfg->Save(SAVE_FILTER);
		}
		else
			break;
	}
		
}
