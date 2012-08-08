; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPerformDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "testcbot.h"
LastPage=0

ClassCount=8
Class1=CBotConsoleDlg
Class2=CChildFrame
Class3=CMainFrame
Class4=CTestCBotApp
Class5=CAboutDlg
Class6=CTestCBotDoc
Class7=CTestCBotView

ResourceCount=12
Resource1=IDD_CONSOLE
Resource2=IDR_TESTCBTYPE (French (France))
Resource3=IDD_ABOUTBOX (French (France))
Resource4=IDR_MAINFRAME (French (France))
Resource5=IDR_MAINFRAME
Resource6=IDR_TESTCBTYPE
Resource7=IDD_ABOUTBOX
Resource8=IDD_CONSOLE (French (Switzerland))
Class8=CPerformDlg
Resource9=IDD_DIALOG1
Resource10=IDD_DIALOG2
Resource11=IDD_DIALOG1 (French (Switzerland))
Resource12=IDD_DIALOG2 (French (France))

[CLS:CBotConsoleDlg]
Type=0
BaseClass=CDialog
HeaderFile=CBotConsoleDlg.h
ImplementationFile=CBotConsoleDlg.cpp
LastObject=IDC_EDIT1

[CLS:CChildFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CMDIFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=CMainFrame

[CLS:CTestCBotApp]
Type=0
BaseClass=CWinApp
HeaderFile=TestCBot.h
ImplementationFile=TestCBot.cpp
Filter=N
VirtualFilter=AC
LastObject=ID_TEST

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=TestCBot.cpp
ImplementationFile=TestCBot.cpp
LastObject=CAboutDlg

[CLS:CTestCBotDoc]
Type=0
BaseClass=CDocument
HeaderFile=TestCBotDoc.h
ImplementationFile=TestCBotDoc.cpp
LastObject=CTestCBotDoc
Filter=N
VirtualFilter=DC

[CLS:CTestCBotView]
Type=0
BaseClass=CView
HeaderFile=TestCBotView.h
ImplementationFile=TestCBotView.cpp
LastObject=CTestCBotView
Filter=C
VirtualFilter=VWC

[DLG:IDD_CONSOLE]
Type=1
Class=CBotConsoleDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT2,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDC_EDIT1,edit,1352734724

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=7
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[TB:IDR_MAINFRAME (French (France))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_RUN
Command9=ID_APP_ABOUT
CommandCount=9

[MNU:IDR_MAINFRAME (French (France))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_MRU_FILE1
Command4=ID_APP_EXIT
Command5=ID_VIEW_TOOLBAR
Command6=ID_VIEW_STATUS_BAR
Command7=ID_APP_ABOUT
CommandCount=7

[MNU:IDR_TESTCBTYPE (French (France))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_MRU_FILE1
Command7=ID_APP_EXIT
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_VIEW_TOOLBAR
Command13=ID_VIEW_STATUS_BAR
Command14=ID_WINDOW_NEW
Command15=ID_WINDOW_CASCADE
Command16=ID_WINDOW_TILE_HORZ
Command17=ID_WINDOW_ARRANGE
Command18=ID_APP_ABOUT
CommandCount=18

[ACL:IDR_MAINFRAME (French (France))]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVE
Command5=ID_EDIT_PASTE
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_RUN
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_RUN
Command12=ID_TEST
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_EDIT_CUT
Command16=ID_EDIT_UNDO
CommandCount=16

[DLG:IDD_ABOUTBOX (French (France))]
Type=1
Class=CAboutDlg
ControlCount=7
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[ACL:IDR_MAINFRAME]
Type=1
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVE
Command5=ID_EDIT_PASTE
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_RUN
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_RUN
Command12=ID_TEST
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_EDIT_CUT
Command16=ID_EDIT_UNDO
CommandCount=16

[TB:IDR_MAINFRAME]
Type=1
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_RUN
Command9=ID_APP_ABOUT
CommandCount=9

[MNU:IDR_MAINFRAME]
Type=1
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_MRU_FILE1
Command4=ID_APP_EXIT
Command5=ID_VIEW_TOOLBAR
Command6=ID_VIEW_STATUS_BAR
Command7=ID_APP_ABOUT
CommandCount=7

[MNU:IDR_TESTCBTYPE]
Type=1
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_MRU_FILE1
Command7=ID_APP_EXIT
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_VIEW_TOOLBAR
Command13=ID_VIEW_STATUS_BAR
Command14=ID_WINDOW_NEW
Command15=ID_WINDOW_CASCADE
Command16=ID_WINDOW_TILE_HORZ
Command17=ID_WINDOW_ARRANGE
Command18=ID_APP_ABOUT
CommandCount=18

[DLG:IDD_CONSOLE (French (Switzerland))]
Type=1
Class=CBotConsoleDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT2,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDC_EDIT1,edit,1352734724

[DLG:IDD_DIALOG1]
Type=1
Class=CPerformDlg
ControlCount=9
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT1,edit,1350633600
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT2,edit,1350631552
Control5=IDC_SPIN1,msctls_updown32,1342177312
Control6=IDC_COMBO1,combobox,1344339971
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_EDIT3,edit,1350633600

[CLS:CPerformDlg]
Type=0
HeaderFile=PerformDlg.h
ImplementationFile=PerformDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT3

[DLG:IDD_DIALOG2]
Type=1
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[DLG:IDD_DIALOG1 (French (Switzerland))]
Type=1
ControlCount=9
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT1,edit,1350633600
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT2,edit,1350631552
Control5=IDC_SPIN1,msctls_updown32,1342177312
Control6=IDC_COMBO1,combobox,1344339971
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_EDIT3,edit,1350633600

[DLG:IDD_DIALOG2 (French (France))]
Type=1
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

