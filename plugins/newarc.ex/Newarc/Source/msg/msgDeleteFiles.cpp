bool msgDeleteFiles()
{
	FarMessage message(0);

	message.Add(_M(MDeleteFilesTitle));
	message.Add(_M(MDeleteFilesPrompt));
	message.AddButton(_M(MDeleteFilesDelete));
	message.AddButton(_M(MDeleteFilesCancel));

	return !message.Run();
}
