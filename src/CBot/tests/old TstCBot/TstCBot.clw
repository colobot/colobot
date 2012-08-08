; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CTstCBotView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "TstCBot.h"
LastPage=0

ClassCount=7
Class1=CTstCBotApp
Class2=CTstCBotDoc
Class3=CTstCBotView
Class4=CMainFrame

ResourceCount=6
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDD_ABOUTBOX (French (France))
Resource4=IDD_CONSOLE
Class6=CBotErrorDlg
Resource5=IDD_DIALOG1 (French (Switzerland))
Class7=CBotConsoleDlg
Resource6=IDR_MAINFRAME (French (France))

[CLS:CTstCBotApp]
Type=0
HeaderFile=TstCBot.h
ImplementationFile=TstCBot.cpp
Filter=N

[CLS:CTstCBotDoc]
Type=0
HeaderFile=TstCBotDoc.h
ImplementationFile=TstCBotDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CTstCBotDoc

[CLS:CTstCBotView]
Type=0
HeaderFile=TstCBotView.h
ImplementationFile=TstCBotView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=CTstCBotView

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame



[CLS:CAboutDlg]
Type=0
HeaderFile=TstCBot.cpp
ImplementationFile=TstCBot.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_STATUS_BAR
Command12=ID_CP1
Command13=ID_EXE
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_CP1
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVE
Command5=ID_EXE
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EXE
Command9=ID_CP1
Command10=ID_EXE
CommandCount=10

[MNU:IDR_MAINFRAME (French (France))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_STATUS_BAR
Command12=ID_CP1
Command13=ID_EXE
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME (French (France))]
Type=1
Class=?
Command1=ID_CP1
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVE
Command5=ID_EXE
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EXE
Command9=ID_CP1
Command10=ID_EXE
CommandCount=10

[DLG:IDD_ABOUTBOX (French (France))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[CLS:CBotErrorDlg]
Type=0
HeaderFile=BotErrorDlg.h
ImplementationFile=BotErrorDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CBotErrorDlg

[DLG:IDD_DIALOG1 (French (Switzerland))]
Type=1
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDC_EDIT1,edit,1352728708
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC1,static,1342308352

[DLG:IDD_CONSOLE]
Type=1
Class=CBotConsoleDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT2,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDC_EDIT1,edit,1352734724

[CLS:CBotConsoleDlg]
Type=0
HeaderFile=BotConsoleDlg.h
ImplementationFile=BotConsoleDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDOK

