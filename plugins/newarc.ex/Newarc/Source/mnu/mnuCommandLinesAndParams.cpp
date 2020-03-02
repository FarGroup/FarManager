
void mnuCommandLinesAndParams(ArchiveManagerConfig* pCfg)
{
	Array<ArchiveFormat*> formats;
	pManager->GetFormats(formats);

	FarMenu menu(_M(MCommandLinesAndParamsArchiveFormat));

	for (unsigned int i = 0; i < formats.count(); i++)
	{
		ArchiveFormat* pFormat = formats[i];

		if ( pFormat->QueryCapability(AFF_SUPPORT_DEFAULT_COMMANDS) )
			menu.Add(pFormat->GetName(), 0, (void*)pFormat);
	}

	while ( true )
	{
		int nResult = menu.Run();

		if ( nResult != -1 )
		{
			ArchiveFormat* pFormat = (ArchiveFormat*)menu.GetData(nResult);
			dlgCommandLinesAndParams(pCfg, pFormat);
		}
		else
			break;
	}
}
