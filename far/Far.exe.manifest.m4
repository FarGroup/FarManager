m4_include(`farversion.m4')m4_dnl
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!-- Copyright © 1996-2000 Eugene Roshal, Copyright © 2000-M4_MACRO_GET(COPYRIGHTYEAR) Far Group -->
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">

<assemblyIdentity
	version="M4_MACRO_GET(VERSION_MAJOR).M4_MACRO_GET(VERSION_MINOR).M4_MACRO_GET(VERSION_BUILD).M4_MACRO_GET(VERSION_REVISION)"
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
		<supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"/> <!-- Windows Vista -->
		<supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/> <!-- Windows 7 -->
		<supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/> <!-- Windows 8 -->
		<supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/> <!-- Windows 8.1 -->
		<supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/> <!-- Windows 10 -->
	</application>
</compatibility>

<application xmlns="urn:schemas-microsoft-com:asm.v3">
	<windowsSettings>
		<dpiAware                xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true/pm</dpiAware>
		<printerDriverIsolation  xmlns="http://schemas.microsoft.com/SMI/2011/WindowsSettings">true</printerDriverIsolation>
		<longPathAware           xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">true</longPathAware>
		<dpiAwareness            xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2, PerMonitor</dpiAwareness>
		<heapType                xmlns="http://schemas.microsoft.com/SMI/2020/WindowsSettings">SegmentHeap</heapType>
	</windowsSettings>
</application>

</assembly>
