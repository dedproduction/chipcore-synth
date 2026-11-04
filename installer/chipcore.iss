; CHIPCORE installer (Inno Setup 6) - only asks where to put the plugin
; Built by GitHub Actions: ISCC /DSourceDir=<dist folder> /DAppVersion=x.y.z installer\chipcore.iss

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\dist"
#endif

[Setup]
AppId={{8E2C4B1A-6F3D-4C7E-9A51-2D7B8C0F4E61}
AppName=CHIPCORE
AppVersion={#AppVersion}
AppPublisher=dedproduction
DefaultDirName={commoncf64}\VST3
DisableDirPage=no
UsePreviousAppDir=no
AppendDefaultDirName=no
DirExistsWarning=no
DisableWelcomePage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableFinishedPage=yes
CreateAppDir=yes
Uninstallable=no
ShowLanguageDialog=no
OutputDir=..\installer-out
OutputBaseFilename=CHIPCORE-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
CloseApplications=yes

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: "{#SourceDir}\CHIPCORE.vst3\*"; DestDir: "{app}\CHIPCORE.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
