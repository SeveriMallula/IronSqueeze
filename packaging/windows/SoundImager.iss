#ifndef BuildRoot
#define BuildRoot "..\..\build\SoundImager_artefacts\Release"
#endif
[Setup]
AppId={{E86E6D46-DA82-4D25-802C-45B889B59741}
AppName=Sound Imager
AppVersion=0.1.0
AppPublisher=Mallula Audio
DefaultDirName={autopf}\Mallula Audio\Sound Imager
DefaultGroupName=Mallula Audio
OutputDir=..\..\dist
OutputBaseFilename=SoundImager-0.1.0-Windows-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=Sound Imager
CloseApplications=yes
[Files]
Source: "{#BuildRoot}\VST3\Sound Imager.vst3\*"; DestDir: "{commoncf64}\VST3\Sound Imager.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BuildRoot}\Standalone\Sound Imager.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\Sound Imager"; Filename: "{app}\Sound Imager.exe"
