; Feather Wallet Installer for Windows
; Copyright (c) 2021-2021, The Monero Project

#define AppName "Feather Wallet"
#define AppVersion "1.0.2"
#define AppPublisher "Feather Wallet"
#define AppURL "https://featherwallet.org"
#define AppExeName "feather.exe"

[Setup]
AppId={{E3C599C7-4DF1-49F2-9C35-918A288677A4}
AppName={#AppName}
AppVersion={#AppVersion}
;AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputBaseFilename=FeatherWalletSetup
SetupIconFile=appicon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
DisableReadyPage=yes

WizardSmallImageFile=compiler:WizClassicSmallImage.bmp
WizardImageFile=compiler:WizClassicImage.bmp

UninstallDisplayIcon={app}\{#AppExeName}

[Messages]
SetupWindowTitle=Setup - Feather Wallet {#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "bin\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "tor\*"; DestDir: "{app}\tor"; Flags: ignoreversion

;Source: "C:\Users\dev\Desktop\feather setup\finishbanner.bmp"; Flags: dontcopy
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Code]
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectTasks then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
end;

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
