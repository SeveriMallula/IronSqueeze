#ifndef BuildRoot
#define BuildRoot "..\..\build\IronSqueeze_artefacts\Release"
#endif
[Setup]
AppId={{59838C51-786F-475E-B408-715A32458DEA}
AppName=Iron Squeeze
AppVersion=0.2.1
AppPublisher=Mallula Audio
DefaultDirName={autopf}\Mallula Audio\Iron Squeeze
DefaultGroupName=Mallula Audio
OutputDir=..\..\dist
OutputBaseFilename=IronSqueeze-0.2.1-Windows-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=Iron Squeeze
CloseApplications=yes
[Files]
Source: "{#BuildRoot}\VST3\Iron Squeeze.vst3\*"; DestDir: "{commoncf64}\VST3\Iron Squeeze.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BuildRoot}\Standalone\Iron Squeeze.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\Iron Squeeze"; Filename: "{app}\Iron Squeeze.exe"
