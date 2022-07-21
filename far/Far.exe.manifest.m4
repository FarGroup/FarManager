﻿m4_include(`farversion.m4')m4_dnl
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!-- Copyright © 1996-2000 Eugene Roshal, Copyright © 2000-COPYRIGHTYEAR Far Group -->
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3" >
<assemblyIdentity
	version="VERSION_MAJOR.VERSION_MINOR.VERSION_BUILD.VERSION_REVISION"
	processorArchitecture="*"
	name="Far Manager"
	type="win32"
/>
<description>File and archive manager</description>
<dependency>
	<dependentAssembly>
		<assemblyIdentity
			type="win32"
			name="Microsoft.Windows.Common-Controls"
			version="6.0.0.0"
			processorArchitecture="*"
			publicKeyToken="6595b64144ccf1df"
			language="*"
		/>
	</dependentAssembly>
</dependency>
<!-- Identify the application security requirements -->
<trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
	<security>
		<requestedPrivileges>
			<requestedExecutionLevel
				level="asInvoker"
				uiAccess="false"
			/>
		</requestedPrivileges>
	</security>
</trustInfo>
<compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
	<application>
		<!--The ID below indicates application support for Windows Vista -->
		<supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"/>
		<!--The ID below indicates application support for Windows 7 -->
		<supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
		<!--The ID below indicates application support for Windows 8 -->
		<supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
		<!--The ID below indicates application support for Windows 8.1 -->
		<supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
		<!--The ID below indicates application support for Windows 10 -->
		<supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
	</application>
</compatibility>
<asmv3:application>
	<asmv3:windowsSettings>
		<dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true</dpiAware>
		<longPathAware xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">true</longPathAware>
		<heapType xmlns="http://schemas.microsoft.com/SMI/2020/WindowsSettings">SegmentHeap</heapType>
	</asmv3:windowsSettings>
</asmv3:application>
</assembly>
