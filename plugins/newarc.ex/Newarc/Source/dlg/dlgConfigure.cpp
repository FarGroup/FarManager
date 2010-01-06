void dlgConfigure ()
{
	FarDialog D (-1, -1, 78, 9);

	D.DoubleBox (3, 1, 74, 7, _M(MConfigCommonTitle));

	D.RadioButton (5, 2, false, _M(MConfigCommonAlwaysShowOutput));
	D.RadioButton (5, 3, false, _M(MConfigCommonDontShowOutputWhenViewingEditing));
	D.RadioButton (5, 4, false, _M(MConfigCommonNeverShowOutput));

	D.Separator (5);

	D.Button (-1, 6, _M(MSG_cmn_B_OK));
	D.DefaultButton ();

	D.Button (-1, 6, _M(MSG_cmn_B_CANCEL));

	if ( D.Run() == D.FirstButton() )
	{
		//do something
	}
}
