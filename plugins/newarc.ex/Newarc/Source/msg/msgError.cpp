bool msgError(const TCHAR* lpErrorString)
{
	FarMessage message(FMSG_WARNING);

	message.Add(_T("Error"));
	message.Add(lpErrorString);
	message.AddButton(_T("Ok"));

	return !message.Run();
}
