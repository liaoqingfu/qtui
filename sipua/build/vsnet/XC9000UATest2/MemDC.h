/*
 ********************************************************************************************************
 * Copyright (C) 2003-2009, Changsha Spon Electronic Co., Ltd
 ********************************************************************************************************
 * Filename     : MemDC.h
 * Author       : EaStar <eastar1110@gmail.com>
 * Description  : 
 ********************************************************************************************************
 * $Id:$ 
 ********************************************************************************************************
 */
#ifndef __MEM_DC_H__
#define __MEM_DC_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMemDCEx : public CDC
{
public:
    CMemDCEx(CDC* pDC) : CDC()
    {
        ASSERT(pDC != NULL);

        m_pDC = pDC;
        m_pOldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
              
        if (m_bMemDC != FALSE) {
            pDC->GetClipBox(&m_xcRect);
            CreateCompatibleDC(pDC);
            m_xcBitmap.CreateCompatibleBitmap(pDC, m_xcRect.Width(), m_xcRect.Height());
            m_pOldBitmap = SelectObject(&m_xcBitmap);
            SetWindowOrg(m_xcRect.left, m_xcRect.top);
        }
        else {
            m_bPrinting = pDC->m_bPrinting;
            m_hDC       = pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        }
    }
    
    ~CMemDCEx(VOID)
    {
        if (m_bMemDC != FALSE) {
			m_pDC->BitBlt(m_xcRect.left, m_xcRect.top, m_xcRect.Width(), m_xcRect.Height(), this, m_xcRect.left, m_xcRect.top, SRCCOPY);
            SelectObject(m_pOldBitmap);
        } 
		else {
            m_hDC = m_hAttribDC = NULL;
        }
    }

    CMemDCEx* operator->() { return this; }
    operator CMemDCEx*()   { return this; }

private:
    CBitmap  m_xcBitmap;
    CBitmap* m_pOldBitmap;
    CDC*     m_pDC;
    CRect    m_xcRect;
    BOOL     m_bMemDC;
};

#endif // __MEM_DC_H__
