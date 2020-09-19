#ifndef UUIDS_FAR_DIALOGS_HPP_760BACF0_E0D8_4C67_A732_5C075A1CC176
#define UUIDS_FAR_DIALOGS_HPP_760BACF0_E0D8_4C67_A732_5C075A1CC176
#pragma once

/*
uuids.far.dialogs.hpp

UUIDs of common dialogs
*/
/*
Copyright © 2010 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Internal:

// Platform:

// Common:
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

namespace uuids::far::dialogs
{
	constexpr inline auto
		FindFileId                       = "8C9EAD29-910F-4B24-A669-EDAFBA6ED964"_uuid,
		FindFileResultId                 = "536754EB-C2D1-4626-933F-A25D1E1D110A"_uuid,
		CopyOverwriteId                  = "9FBCB7E1-ACA2-475D-B40D-0F7365B632FF"_uuid,
		FileOpenCreateId                 = "1D07CEE2-8F4F-480A-BE93-069B4FF59A2B"_uuid,
		FileSaveAsId                     = "9162F965-78B8-4476-98AC-D699E5B6AFE7"_uuid,
		EditorSavedROId                  = "3F9311F5-3CA3-4169-A41C-89C76B3A8C1D"_uuid,
		EditorSaveF6DeletedId            = "85532BD5-1583-456D-A810-41AB345995A9"_uuid,
		EditorSaveExitDeletedId          = "2D71DCCE-F0B8-4E29-A3A9-1F6D8C1128C2"_uuid,
		EditorAskOverwriteId             = "4109C8B3-760D-4011-B1D5-14C36763B23E"_uuid,
		EditorOpenRSHId                  = "D8AA706F-DA7E-4BBF-AB78-6B7BDB49E006"_uuid,
		EditAskSaveExtId                 = "40A699F1-BBDD-4E21-A137-97FFF798B0C8"_uuid,
		EditAskSaveId                    = "F776FEC0-50F7-4E7E-BDA6-2A63F84A957B"_uuid,
		MakeFolderId                     = "FAD00DBE-3FFF-4095-9232-E1CC70C67737"_uuid,
		FileAttrDlgId                    = "80695D20-1085-44D6-8061-F3C41AB5569C"_uuid,
		CopyReadOnlyId                   = "879A8DE6-3108-4BEB-80DE-6F264991CE98"_uuid,
		CopyFilesId                      = "FCEF11C4-5490-451D-8B4A-62FA03F52759"_uuid,
		CopyCurrentOnlyFileId            = "502D00DF-EE31-41CF-9028-442D2E352990"_uuid,
		MoveFilesId                      = "431A2F37-AC01-4ECD-BB6F-8CDE584E5A03"_uuid,
		MoveCurrentOnlyFileId            = "89664EF4-BB8C-4932-A8C0-59CAFD937ABA"_uuid,
		HardSymLinkId                    = "5EB266F4-980D-46AF-B3D2-2C50E64BCA81"_uuid,
		PluginsMenuId                    = "937F0B1C-7690-4F85-8469-AA935517F202"_uuid,
		EditorReloadId                   = "AFDAD388-494C-41E8-BAC6-BBE9115E1CC0"_uuid,
		FarAskQuitId                     = "72E6E6D8-0BC6-4265-B9C4-C8DB712136AF"_uuid,
		AdvancedConfigId                 = "A204FF09-07FA-478C-98C9-E56F61377BDE"_uuid,
		FolderShortcutsId                = "4CD742BC-295F-4AFA-A158-7AA05A16BEA1"_uuid,
		FolderShortcutsDlgId             = "DC8D98AC-475C-4F37-AB1D-45765EF06269"_uuid,
		FolderShortcutsMoreId            = "601DD149-92FA-4601-B489-74C981BC8E38"_uuid,
		ScreensSwitchId                  = "72EB948A-5F1D-4481-9A91-A4BFD869D127"_uuid,
		SelectSortModeId                 = "B8B6E1DA-4221-47D2-AB2E-9EC67D0DC1E3"_uuid,
		HistoryCmdId                     = "880968A6-6258-43E0-9BDC-F2B8678EC278"_uuid,
		HistoryFolderId                  = "FC3384A8-6608-4C9B-8D6B-EE105F4C5A54"_uuid,
		HistoryEditViewId                = "E770E044-23A8-4F4D-B268-0E602B98CCF9"_uuid,
		PanelViewModesId                 = "B56D5C08-0336-418B-A2A7-CF0C80F93ACC"_uuid,
		PanelViewModesEditId             = "98B75500-4A97-4299-BFAD-C3E349BF3674"_uuid,
		CodePagesMenuId                  = "78A4A4E3-C2F0-40BD-9AA7-EAAC11836631"_uuid,
		EditorReplaceId                  = "8BCCDFFD-3B34-49F8-87CD-F4D885B75873"_uuid,
		EditorSearchId                   = "5D3CBA90-F32D-433C-B016-9BB4AF96FACC"_uuid,
		HelpSearchId                     = "F63B558F-9185-46BA-8701-D143B8F62658"_uuid,
		FiltersMenuId                    = "5B87B32E-494A-4982-AF55-DAFFCD251383"_uuid,
		FiltersConfigId                  = "EDDB9286-3B08-4593-8F7F-E5925A3A0FF8"_uuid,
		HighlightMenuId                  = "D0422DF0-AAF5-46E0-B98B-1776B427E70D"_uuid,
		HighlightConfigId                = "51B6E342-B499-464D-978C-029F18ECCE59"_uuid,
		PluginsConfigMenuId              = "B4C242E7-AA8E-4449-B0C3-BD8D9FA11AED"_uuid,
		ChangeDiskMenuId                 = "252CE4A3-C415-4B19-956B-83E2FDD85960"_uuid,
		FileAssocMenuId                  = "F6D2437C-FEDC-4075-AA56-275666FC8979"_uuid,
		SelectAssocMenuId                = "D2BCB5A5-6B82-4EB5-B321-1AE7607A6236"_uuid,
		FileAssocModifyId                = "6F245B1A-47D9-41A6-AF3F-FA2C8DBEEBD0"_uuid,
		EditorSwitchUnicodeCPDisabledId  = "15568DC5-4D6B-4C60-B43D-2040EE39871A"_uuid,
		GetNameAndPasswordId             = "CD2AC546-9E4F-4445-A258-AB5F7A7800E0"_uuid,
		SelectFromEditHistoryId          = "4406C688-209F-4378-8B7B-465BF16205FF"_uuid,
		EditorReloadModalId              = "D6F557E8-7E89-4895-BD75-4D3F2C30E382"_uuid,
		EditorCanNotEditDirectoryId      = "CCA2C4D0-8705-4FA1-9B10-C9E3C8F37A65"_uuid,
		EditorFileLongId                 = "E3AFCD2D-BDE5-4E92-82B6-87C6A7B78FB6"_uuid,
		EditorFileGetSizeErrorId         = "6AD4B317-C1ED-44C8-A76A-9146CA8AF984"_uuid,
		DisconnectDriveId                = "A1BDBEB1-2911-41FF-BC08-EEBC44040B50"_uuid,
		ChangeDriveModeId                = "F87F9351-6A80-4872-BEEE-96EF80C809FB"_uuid,
		SUBSTDisconnectDriveId           = "75554EEB-A3A7-45FD-9795-4A85887A75A0"_uuid,
		VHDDisconnectDriveId             = "629A8CA6-25C6-498C-B3DD-0E18D1CC0BCD"_uuid,
		EditorFindAllListId              = "9BD3E306-EFB8-4113-8405-E7BADE8F0A59"_uuid,
		BadEditorCodePageId              = "4811039D-03A3-4F15-8D7A-8EBC4BCC97F9"_uuid,
		UserMenuUserInputId              = "D2750B57-D3E6-42F4-8137-231C50DDC6E4"_uuid,
		DescribeFileId                   = "D8AF7A38-8357-44A5-A44B-A595CF707549"_uuid,
		SelectDialogId                   = "29C03C36-9C50-4F78-AB99-F5DC1A9C67CD"_uuid,
		UnSelectDialogId                 = "34614DDB-2A22-4EA9-BD4A-2DC075643F1B"_uuid,
		SUBSTDisconnectDriveError1Id     = "FF18299E-1881-42FA-AF7E-AC05D99F269C"_uuid,
		SUBSTDisconnectDriveError2Id     = "43B0FFC2-70BE-4289-91E6-FE9A3D54311B"_uuid,
		EjectHotPlugMediaErrorId         = "D6DC3621-877E-4BE2-80CC-BDB2864CE038"_uuid,
		RemoteDisconnectDriveError2Id    = "F06953B8-25AA-4FC0-9899-422FC1D49F7A"_uuid,
		RemoteDisconnectDriveError1Id    = "C9439386-9544-49BF-954B-6BEEDE7F1BD0"_uuid,
		VHDDisconnectDriveErrorId        = "B890E6B0-05A9-4ED8-A4C3-BBC4D29DA3BE"_uuid,
		ChangeDriveCannotReadDiskErrorId = "F3D46DC3-380B-4264-8BF8-10B05B897A5E"_uuid,
		ApplyCommandId                   = "044EF83E-8146-41B2-97F0-404C2F4C7B69"_uuid,
		DeleteFileFolderId               = "6EF09401-6FE1-495A-8539-61B0F761408E"_uuid,
		DeleteRecycleId                  = "85A5F779-A881-4B0B-ACEE-6D05653AE0EB"_uuid,
		DeleteWipeId                     = "9C054039-5C7E-4B04-96CD-3585228C916F"_uuid,
		DeleteLinkId                     = "B1099BC3-14BD-4B22-87AC-44770D4189A3"_uuid,
		DeleteFolderId                   = "4E714029-11BF-476F-9B17-9E47AA0DA8EA"_uuid,
		WipeFolderId                     = "E23BB390-036E-4A30-A9E6-DC621617C7F5"_uuid,
		DeleteFolderRecycleId            = "A318CBDC-DBA9-49E9-A248-E6A9FF8EC849"_uuid,
		DeleteAskWipeROId                = "6792A975-57C5-4110-8129-2D8045120964"_uuid,
		DeleteAskDeleteROId              = "8D4E84B3-08F6-47DF-8C40-7130CD31D0E6"_uuid,
		WipeHardLinkId                   = "5297DDFE-0A37-4465-85EF-CBF9006D65C6"_uuid,
		RecycleFolderConfirmDeleteLinkId = "26A7AB9F-51F5-40F7-9061-1AE6E2FBD00A"_uuid,
		CannotRecycleFileId              = "52CEB5A5-06FA-43DD-B37C-239C02652C99"_uuid,
		CannotRecycleFolderId            = "BBD9B7AE-9F6B-4444-89BF-C6124A5A83A4"_uuid,
		AskInsertMenuOrCommandId         = "57209AD5-51F6-4257-BAB6-837462BBCE74"_uuid,
		EditUserMenuId                   = "73BC6E3E-4CC3-4FE3-8709-545FF72B49B4"_uuid,
		PluginInformationId              = "FC4FD19A-43D2-4987-AC31-0F7A94901692"_uuid,
		ViewerSearchId                   = "03B6C098-A3D6-4DFB-AED4-EB32D711D9AA"_uuid;
}

// TODO: Use fully qualified names everywhere
inline namespace uuids_inline
{
	using namespace uuids::far::dialogs;
}

#endif // UUIDS_FAR_DIALOGS_HPP_760BACF0_E0D8_4C67_A732_5C075A1CC176
