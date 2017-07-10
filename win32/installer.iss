[Setup]
AppName=T3F_APP_NAME
AppVerName=T3F_APP_NAME vT3F_APP_VERSION
AppCopyright=T3F_APP_COPYRIGHT
AppPublisher=T3F_APP_PUBLISHER
AppPublisherURL=T3F_APP_URL
AppSupportURL=T3F_APP_URL
AppUpdatesURL=T3F_APP_URL
DefaultDirName={pf}\T3F_APP_NAME
DefaultGroupName=T3F_APP_NAME
LicenseFile=T3F_APP_LICENSE_FILE
InfoAfterFile=T3F_APP_README_FILE
OutputDir=T3F_APP_INSTALLER_DIR
OutputBaseFilename=T3F_APP_INSTALLER_NAME
Compression=lzma
SolidCompression=yes

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\bin\T3F_APP_EXECUTABLE"; Destdir: "{app}"
Source: "T3F_APP_LIBS_DIR\*.dll"; Destdir: "{app}"; Flags: skipifsourcedoesntexist
Source: "T3F_APP_USER_LIBS_DIR\*.dll"; Destdir: "{app}"; Flags: skipifsourcedoesntexist
Source: "T3F_APP_DATA_DIR\*.*"; DestDir: "{app}\data"; Flags: recursesubdirs
Source: "T3F_APP_DOCS_DIR\README"; DestDir: "{app}\docs"; DestName: readme.txt
Source: "T3F_APP_DOCS_DIR\copyright"; DestDir: "{app}\docs"; DestName: license.txt
Source: "T3F_APP_DOCS_DIR\changelog"; DestDir: "{app}\docs"; DestName: history.txt

[Icons]
Name: "{group}\T3F_APP_NAME"; Filename: "{app}\T3F_APP_EXECUTABLE"; WorkingDir: "{app}"
Name: "{group}\License"; Filename: "{app}\docs\license.txt"; WorkingDir: "{app}"
Name: "{group}\Readme"; Filename: "{app}\docs\readme.txt"; WorkingDir: "{app}"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"; WorkingDir: "{app}"
Name: "{userdesktop}\T3F_APP_NAME"; Filename: "{app}\T3F_APP_EXECUTABLE"; WorkingDir: "{app}"
