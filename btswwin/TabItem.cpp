#include "pch.h"
#include "TabItem.h"

CTabItem::CTabItem(UINT nIDTemplate)
    : CDialogEx(nIDTemplate)
{
}

BOOL CTabItem::Create(CWnd* pParent)
{
    return CDialogEx::Create(m_lpszTemplateName, pParent);
}
