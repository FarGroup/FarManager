void _fastcall TMacroView::ReadConfig()
{
	if (Reg->OpenKey(PluginRootKey))
	{
		Conf.AddDescription=Reg->ReadInteger(_T("AddDescription"));
		Conf.AutomaticSave=Reg->ReadInteger(_T("AutomaticSave"));
		Conf.UseHighlight=Reg->ReadInteger(_T("UseHighlight"));
		Conf.StartDependentSort=Reg->ReadInteger(_T("StartDependentSort"));
		Conf.LongGroupNames=Reg->ReadInteger(_T("LongGroupNames"));
		Conf.MenuCycle=Reg->ReadInteger(_T("MenuCycle"));
		Conf.DblClick=Reg->ReadInteger(_T("DblClick"));
		Conf.GroupDivider=Reg->ReadInteger(_T("GroupDivider"));
		Conf.SaveOnStart=Reg->ReadInteger(_T("SaveOnStart"));
		Conf.ViewShell=Reg->ReadInteger(_T("ViewShell"));
		Conf.ViewViewer=Reg->ReadInteger(_T("ViewViewer"));
		Conf.ViewEditor=Reg->ReadInteger(_T("ViewEditor"));

		if (Conf.AddDescription<0)
		{
			Reg->WriteInteger(_T("AddDescription"),1);
			Conf.AddDescription=1;
		}

		if (Conf.AutomaticSave<0)
			Conf.AutomaticSave=0;

		if (Conf.UseHighlight<0)
		{
			Reg->WriteInteger(_T("UseHighlight"),1);
			Conf.UseHighlight=1;
		}

		if (Conf.StartDependentSort<0)
			Conf.StartDependentSort=0;

		if (Conf.LongGroupNames<0)
		{
			Reg->WriteInteger(_T("LongGroupNames"),1);
			Conf.LongGroupNames=1;
		}

		if (Conf.MenuCycle<0)
		{
			Reg->WriteInteger(_T("MenuCycle"),1);
			Conf.MenuCycle=1;
		}

		if (Conf.DblClick<0)
		{
			Reg->WriteInteger(_T("DblClick"),1);
			Conf.DblClick=1;
		}

		if (Conf.GroupDivider<0)
		{
			Reg->WriteInteger(_T("GroupDivider"),1);
			Conf.GroupDivider=1;
		}

		if (Conf.SaveOnStart<0)
		{
			Reg->WriteInteger(_T("SaveOnStart"),1);
			Conf.SaveOnStart=1;
		}

		if (Conf.ViewShell<0)
			Conf.ViewShell=0;

		if (Conf.ViewViewer<0)
			Conf.ViewViewer=0;

		if (Conf.ViewEditor<0)
			Conf.ViewEditor=0;

		Reg->CloseKey();
	}
	else
	{
		Conf.AddDescription=1;
		Conf.AutomaticSave=0;
		Conf.UseHighlight=1;
		Conf.StartDependentSort=0;
		Conf.LongGroupNames=1;
		Conf.MenuCycle=1;
		Conf.DblClick=1;
		Conf.GroupDivider=1;
		Conf.SaveOnStart=1;
		Conf.ViewShell=0;
		Conf.ViewViewer=0;
		Conf.ViewEditor=0;
	}
}

BOOL _fastcall TMacroView::Configure()
{
	BOOL Result = FALSE;
	int OutCode;
	TCHAR *ItemsErrorWrite[]=
	{
		GetMsg(MMacroError),GetMsg(MMacroErrorWrite),
		S,GetMsg(MMacroOk),
	};
	struct InitDialogItem InitItems[]=
	{
		/* 0  */ {DI_DOUBLEBOX,3,1,70,18,0,0,0,0,(TCHAR *)MMacroConfig},
		/* 1  */ {DI_CHECKBOX,5,2,0,0,1,0,0,0,(TCHAR *)MMacroAddDescription},
		/* 2  */ {DI_CHECKBOX,5,3,0,0,0,0,0,0,(TCHAR *)MMacroAutomaticSave},
		/* 3  */ {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MMacroUseHighlight},
		/* 4  */ {DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)MMacroStartDependentSort},
		/* 5  */ {DI_CHECKBOX,5,6,0,0,0,0,0,0,(TCHAR *)MMacroLongGroupNames},
		/* 6  */ {DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)MMacroMenuCycle},
		/* 7  */ {DI_CHECKBOX,5,8,0,0,0,0,0,0,(TCHAR *)MMacroDblClick},
		/* 8  */ {DI_CHECKBOX,5,9,0,0,0,0,0,0,(TCHAR *)MMacroGroupDivider},
		/* 9  */ {DI_TEXT,5,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/* 10 */ {DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)MMacroSaveOnStart},
		/* 11 */ {DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/* 12 */ {DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)MMacroViewShell},
		/* 13 */ {DI_CHECKBOX,5,14,0,0,0,0,0,0,(TCHAR *)MMacroViewViewer},
		/* 14 */ {DI_CHECKBOX,5,15,0,0,0,0,0,0,(TCHAR *)MMacroViewEditor},
		/* 15 */ {DI_TEXT,5,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/* 16 */ {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MMacroSave},
		/* 17 */ {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MMacroCancel},
	};
	int size=ARRAYSIZE(InitItems);
	FarDialogItem *DialogItems=new FarDialogItem[size];
	InitDialogItems(InitItems,DialogItems,size);
	Reg->CreateKey(PluginRootKey);
	ReadConfig();
	DialogItems[1].Param.Selected=Conf.AddDescription;
	DialogItems[2].Param.Selected=Conf.AutomaticSave;
	DialogItems[3].Param.Selected=Conf.UseHighlight;
	DialogItems[4].Param.Selected=Conf.StartDependentSort;
	DialogItems[5].Param.Selected=Conf.LongGroupNames;
	DialogItems[6].Param.Selected=Conf.MenuCycle;
	DialogItems[7].Param.Selected=Conf.DblClick;
	DialogItems[8].Param.Selected=Conf.GroupDivider;
	DialogItems[10].Param.Selected=Conf.SaveOnStart;
	DialogItems[12].Param.Selected=Conf.ViewShell;
	DialogItems[13].Param.Selected=Conf.ViewViewer;
	DialogItems[14].Param.Selected=Conf.ViewEditor;
#ifndef UNICODE
	OutCode=Info.Dialog(Info.ModuleNumber,-1,-1,74,20,_T("Config"),DialogItems,size);
#else
	HANDLE hDlg=Info.DialogInit(Info.ModuleNumber,-1,-1,74,20,_T("Config"),DialogItems,size,0,0,NULL,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		OutCode=Info.DialogRun(hDlg);
#endif

	if (OutCode==16) // кнопка [Сохранить]
	{
		Reg->OpenKey(PluginRootKey);

		if ((Reg->WriteInteger(_T("AddDescription"),GetCheck(1))) &&
		        (Reg->WriteInteger(_T("AutomaticSave"),GetCheck(2))) &&
		        (Reg->WriteInteger(_T("UseHighlight"),GetCheck(3))) &&
		        (Reg->WriteInteger(_T("StartDependentSort"),GetCheck(4))) &&
		        (Reg->WriteInteger(_T("LongGroupNames"),GetCheck(5))) &&
		        (Reg->WriteInteger(_T("MenuCycle"),GetCheck(6))) &&
		        (Reg->WriteInteger(_T("DblClick"),GetCheck(7))) &&
		        (Reg->WriteInteger(_T("GroupDivider"),GetCheck(8))) &&
		        (Reg->WriteInteger(_T("SaveOnStart"),GetCheck(10))) &&
		        (Reg->WriteInteger(_T("ViewShell"),GetCheck(12))) &&
		        (Reg->WriteInteger(_T("ViewViewer"),GetCheck(13))) &&
		        (Reg->WriteInteger(_T("ViewEditor"),GetCheck(14))))
		{
			Conf.AddDescription=GetCheck(1);
			Conf.AutomaticSave=GetCheck(2);
			Conf.UseHighlight=GetCheck(3);
			Conf.StartDependentSort=GetCheck(4);
			Conf.LongGroupNames=GetCheck(5);
			Conf.MenuCycle=GetCheck(6);
			Conf.DblClick=GetCheck(7);
			Conf.GroupDivider=GetCheck(8);
			Conf.SaveOnStart=GetCheck(10);
			Conf.ViewShell=GetCheck(12);
			Conf.ViewViewer=GetCheck(13);
			Conf.ViewEditor=GetCheck(14);
			Result=TRUE;
		}
		else
		{
			lstrcpy(ItemsErrorWrite[2],PluginRootKey);
			QuoteText(ItemsErrorWrite[2]);
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorWrite,
			             ARRAYSIZE(ItemsErrorWrite),1);
			Result=FALSE;
		}
	}

#ifdef UNICODE
	Info.DialogFree(hDlg);
}

#endif
delete[] DialogItems;
Reg->CloseKey();
return Result;
}
