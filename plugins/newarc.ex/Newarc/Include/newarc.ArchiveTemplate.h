#pragma once
#include "newarc.h"

class ArchiveTemplate {

private:

	bool m_bInvalid;

	string m_strName;
	string m_strParams;

	string m_strConfig;

	GUID m_uidModule;
	GUID m_uidPlugin;
	GUID m_uidFormat;

	ArchiveFormat* m_pFormat;

public:

	const TCHAR* GetName() const
	{
		return m_strName;
	}

	void SetName(const TCHAR* lpName)
	{
		m_strName = lpName;
	}

	const TCHAR* GetParams() const
	{
		return m_strParams;
	}

	const TCHAR* GetConfig() const
	{
		return m_strConfig;
	}

	void SetConfig(const TCHAR* lpConfig)
	{
		m_strConfig = lpConfig;
	}

	void SetParams(const TCHAR* lpParams)
	{
		m_strParams = lpParams;
	}

	const GUID& GetModuleUID() const
	{
		return m_uidModule;
	}
	
	const GUID& GetPluginUID() const
	{
		return m_uidPlugin;
	}

	const GUID& GetFormatUID() const
	{
		return m_uidFormat;
	}

	bool IsValid() const
	{
		return !m_bInvalid;
	}

	ArchiveTemplate()
	{
		m_bInvalid = true;
	}

	void SetData(
			ArchiveModuleManager* pManager,
			const TCHAR* lpName,
			const TCHAR* lpParams,
			const TCHAR* lpConfig,
			const GUID& uidModule,
			const GUID& uidPlugin,
			const GUID& uidFormat
			)
	{
		m_strName = lpName;
		m_strParams = lpParams;
		m_strConfig = lpConfig;

		m_uidModule = uidModule;
		m_uidPlugin = uidPlugin;
		m_uidFormat = uidFormat;

		m_pFormat = pManager->GetFormat(uidModule, uidPlugin, uidFormat);

		m_bInvalid = (m_pFormat == NULL);
	}

	ArchiveTemplate(
			ArchiveModuleManager* pManager,
			const TCHAR* lpName,
			const TCHAR* lpParams,
			const TCHAR* lpConfig,
			const GUID& uidModule,
			const GUID& uidPlugin,
			const GUID& uidFormat
			)
	{
		SetData(pManager, lpName, lpParams, lpConfig, uidModule, uidPlugin, uidFormat);
	}

	~ArchiveTemplate()
	{
	}

	void SetFormat(ArchiveFormat* pFormat)
	{
		m_bInvalid = false;

		m_pFormat = pFormat;

		m_uidFormat = pFormat->GetUID();
		m_uidPlugin = pFormat->GetPlugin()->GetUID();
		m_uidModule = pFormat->GetModule()->GetUID();
	}

	ArchiveFormat* GetFormat() const
	{
		return m_pFormat;
	}

	bool HasFormat(ArchiveFormat* pFormat)
	{
		return ((m_uidFormat == pFormat->GetUID()) &&
				(m_uidPlugin == pFormat->GetPlugin()->GetUID()) &&
				(m_uidModule == pFormat->GetModule()->GetUID()));
	}

};
