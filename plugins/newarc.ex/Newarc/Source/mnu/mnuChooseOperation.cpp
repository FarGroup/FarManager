int mnuChooseOperation()
{
	FarMenu menu(_T("Архивные комманды"));

	menu.Add(_M(MSG_mnuCO_S_TEST_ARCHIVE));
	menu.Add(_M(MSG_mnuCO_S_ADD_ARCHIVE_COMMENT));
	menu.Add(_M(MSG_mnuCO_S_ADD_FILE_COMMENT));
	menu.Add(_M(MSG_mnuCO_S_CONVERT_TO_SFX));
	menu.Add(_M(MSG_mnuCO_S_RECOVER));
	menu.Add(_M(MSG_mnuCO_S_ADD_RECOVERY_RECORD));
	menu.Add(_M(MSG_mnuCO_S_LOCK));

	return menu.Run();
}
