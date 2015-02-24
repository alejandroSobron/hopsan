; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Hopsan"
#define MyAppPublisher "Link�ping University"
#define MyAppURL "http://www.iei.liu.se/flumes/hopsan"
#define MyAppExeName "HopsanGUI.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{02E63B1C-D08C-4377-952C-A5E22EE620DC}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
ArchitecturesInstallIn64BitMode={#MyArchitecture}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=Hopsan-{#MyAppVersion}-win32-installer
SetupIconFile=HopsanGUI\hopsan.ico
Compression=lzma
SolidCompression=yes
WizardImageFile="installation_large.bmp"
WizardSmallImageFile="installation_small.bmp"
WizardImageStretch=yes
ChangesAssociations=yes
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon=HopsanGUI\hopsan.ico
DirExistsWarning=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
DirExists=The folder:%n%n%1%n%nalready exists. All previous contents will be removed! Would you like to install to that folder anyway?

[CustomMessages]
ProjectHomepage=Project &Homepage
UserGuide=User &Guide

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1
Name: "associatehmf"; Description: "Associate .hmf files with Hopsan"; Flags: dontinheritcheck
                                   
[InstallDelete]
Type: filesandordirs; Name: "{app}"

[Dirs]
;Create installation directories
Name: "{userdocs}\Hopsan"
Name: "{userdocs}\Hopsan\Models"
Name: "{userdocs}\Hopsan\Scripts"
                           
[Files]
;Hopsan files
Source: "{#MyFilesSource}\*";          DestDir: "{app}";     Flags: ignoreversion recursesubdirs createallsubdirs uninsremovereadonly;   Attribs: readonly
Source: "{#MyFilesSource}\Scripts\*";  Excludes: "---*---";  DestDir: "{userdocs}\Hopsan\Scripts";   Flags: ignoreversion recursesubdirs createallsubdirs uninsremovereadonly overwritereadonly;   Attribs: readonly
; NOTE: Don't use "Flags: ignoreversion" on any shared system files


; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{group}\{cm:UserGuide,{#MyAppName}}"; Filename: "{app}\doc\user\html\index.html"
Name: "{group}\{cm:ProjectHomepage,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall
Filename: "{app}\Hopsan-release-notes.txt"; Description: "View release notes"; Flags: nowait postinstall shellexec

[Registry]
Root: HKCR; Subkey: ".hmf"; ValueType: string; ValueName: ""; ValueData: "HopsanModelFile"; Flags: uninsdeletevalue; Tasks: associatehmf
Root: HKCR; Subkey: "HopsanModelFile"; ValueType: string; ValueName: ""; ValueData: "Hopsan Model File"; Flags: uninsdeletekey; Tasks: associatehmf
Root: HKCR; Subkey: "HopsanModelFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\HopsanGUI.exe,1"; Tasks: associatehmf
Root: HKCR; Subkey: "HopsanModelFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\HopsanGUI.exe"" ""%1"""; Tasks: associatehmf
