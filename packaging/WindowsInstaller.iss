; Inno Setup script for the Besprited Windows installer.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Besprited"
#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif
#define MyAppPublisher "Veritaware"
#define MyAppURL "https://besprited.veritaware.com"
#define MyAppExeName "besprited.exe"
#define MyAppAssocName MyAppName + " File"
#define MyAppAssocExt ".ase"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
AppId={{E50A60CE-DF57-4FDE-AF8F-91D452A4C60D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
; Install for the current user only so no administrator elevation is required.
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=besprited-v{#MyAppVersion}-windows-x86_64
SetupIconFile=..\data\icons\ase.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
WizardImageFile=iss\wizardimg.bmp
WizardSmallImageFile=iss\wizardsmall.bmp

[Languages]
Name: "english";    MessagesFile: "compiler:Default.isl"
Name: "arabic";     MessagesFile: "compiler:Languages\Arabic.isl"
Name: "german";     MessagesFile: "compiler:Languages\German.isl"
Name: "spanish";    MessagesFile: "compiler:Languages\Spanish.isl"
Name: "french";     MessagesFile: "compiler:Languages\French.isl"
Name: "hindi";      MessagesFile: "iss\languages\Hindi.islu"
Name: "indonesian"; MessagesFile: "iss\languages\Indonesian.isl"
Name: "italian";    MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese";   MessagesFile: "compiler:Languages\Japanese.isl"
Name: "korean";     MessagesFile: "compiler:Languages\Korean.isl"
Name: "polish";     MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian";    MessagesFile: "compiler:Languages\Russian.isl"
Name: "chinese";    MessagesFile: "iss\languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\build\bin\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\bin\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
