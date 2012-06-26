// TstCBotDoc.cpp : implementation of the CTstCBotDoc class
//

#include "stdafx.h"
#include "TstCBot.h"

#include "TstCBotDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc

IMPLEMENT_DYNCREATE(CTstCBotDoc, CDocument)

BEGIN_MESSAGE_MAP(CTstCBotDoc, CDocument)
    //{{AFX_MSG_MAP(CTstCBotDoc)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc construction/destruction

CTstCBotDoc::CTstCBotDoc()
{
    // TODO: add one-time construction code here

}

CTstCBotDoc::~CTstCBotDoc()
{
}

BOOL CTstCBotDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)

    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc serialization

void CTstCBotDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc diagnostics

#ifdef _DEBUG
void CTstCBotDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CTstCBotDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTstCBotDoc commands

