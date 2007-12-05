// MenuXP.cpp: implementation of the CMenuXP class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MenuXP.h"
#include "emule.h"
#include "CIF.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CM_DISABLEDBLEND ILD_BLEND25
#define CM_ICONWIDTH		16
#define CM_ICONHEIGHT		16
//#define ID_SEPARATOR       0


//IMPLEMENT_DYNAMIC( CMenuXP, CMenu )
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMenuXP MenuXP;

CMenuXP::CMenuXP()
{
		cif.CreateFonts();
		m_nCheckIcon	= 0;
		m_bEnable		= TRUE;
		m_bUnhook		= FALSE;
}

CMenuXP::~CMenuXP()
{
		SetWatermark( NULL );
		if ( m_bUnhook ) EnableHook( FALSE );
}

/****************************************************************************
                          
������:
       MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)

��������:
      	��ò˵������ϸ�ߴ�,ͬʱ�����˷ָ����Ͳ˵���ĳ��Ϳ�

�����������õĺ����嵥:
      		
���ñ������ĺ����嵥:
      
����:
     LPMEASUREITEMSTRUCT lpMeasureItemStruct ָ��MEASUREITEMSTRUCT�ṹ��ָ��

	typedef struct tagMEASUREITEMSTRUCT {   // mis 
				UINT  CtlType;      // type of control 
				UINT  CtlID;        // combo box, list box, or button identifier 
				UINT  itemID;       // menu item, variable-height list box, or combo box identifier
				UINT  itemWidth;    // width of menu item, in pixels 
				UINT  itemHeight;   // height of single item in list box menu, in pixels 
				DWORD itemData;     // application-defined 32-bit value 
	} MEASUREITEMSTRUCT;

����ֵ:
	    void
����:
  
****************************************************************************/
void CMenuXP::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
		if(lpMeasureItemStruct->itemID == ID_SEPARATOR)    //#define ID_SEPARATOR     0
		{
				lpMeasureItemStruct->itemWidth	= 16;    //�ָ�������
				lpMeasureItemStruct->itemHeight	= 2;     //�ָ����߶�
		}
		else
        {
				CString		strText;
				CDC         dc;

				m_pStrings.Lookup(static_cast<DWORD>(lpMeasureItemStruct->itemData), strText);

				dc.Attach(GetDC(0));

				CFont* pOld = (CFont*)dc.SelectObject( &cif.m_fntUP );
				CSize sz = dc.GetTextExtent( strText );
				dc.SelectObject( pOld );
		
				ReleaseDC( 0, dc.Detach() );
		
				lpMeasureItemStruct->itemWidth	= sz.cx + 32;
				lpMeasureItemStruct->itemHeight	= 23;
		}

}

/****************************************************************************
                          
������:
       DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)

��������:
      	���Ʋ˵���

�����������õĺ����嵥:
      		
���ñ������ĺ����嵥:
      
����:
     LPDRAWITEMSTRUCT lpDrawItemStruct ָ��DRAWITEMSTRUCT�ṹ��ָ��

	typedef struct tagDRAWITEMSTRUCT {  // dis 
				UINT  CtlType; 
				UINT  CtlID; 
				UINT  itemID; 
				UINT  itemAction; 
				UINT  itemState; 
				HWND  hwndItem; 
				HDC   hDC; 
				RECT  rcItem; 
				DWORD itemData; 
	} DRAWITEMSTRUCT; 


����ֵ:
	    void
����:
  
****************************************************************************/
void CMenuXP::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	
		CRect rcItem, rcText;
		CString strText;
		int nIcon = -1;
		CDC dc;
		
	    //�˵��ļ���״̬
		BOOL	bSelected	= lpDrawItemStruct->itemState & ODS_SELECTED;//ѡ��
		BOOL	bChecked	= lpDrawItemStruct->itemState & ODS_CHECKED;//��ѡ
		BOOL	bDisabled	= lpDrawItemStruct->itemState & ODS_GRAYED;//������
		BOOL	bKeyboard	= FALSE;//����
		BOOL	bEdge		= TRUE;
	
		dc.Attach( lpDrawItemStruct->hDC );//�õ�Ҫ���ƵĲ˵�

		if (CWnd* pWnd = dc.GetWindow())
		{
				CRect rcScreen( &lpDrawItemStruct->rcItem );
				CPoint ptCursor;
		
				GetCursorPos( &ptCursor );
				pWnd->ClientToScreen( &rcScreen );
		}
		
		rcItem.CopyRect( &lpDrawItemStruct->rcItem );
		rcItem.OffsetRect( -rcItem.left, -rcItem.top );

		rcText.CopyRect( &rcItem );
		rcText.left += 32;//�˵������ı���λ��
		rcText.right -= 2;

		CDC* pDC = cif.GetBuffer( dc, rcItem.Size() );

		if ( m_bmWatermark.m_hObject != NULL )
		{
				DrawWatermark( pDC, &rcItem, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top );
		}
		else
		{
				pDC->FillSolidRect( rcItem.left, rcItem.top, 24, rcItem.Height(), cif.m_crMargin );
				pDC->FillSolidRect( rcItem.left + 24, rcItem.top, rcItem.Width() - 24, rcItem.Height(), cif.m_crBackNormal );
		}
		//�жϷָ���(���˵������ݲ�����Ӧ�ַ�������Ϊ�Ƿָ���)
		if ( m_pStrings.Lookup( static_cast<DWORD>(lpDrawItemStruct->itemData), strText ) == FALSE )
		{
				int nMiddle = rcText.top + 1;//�ָ�����λ��: + ˮƽ�����ƶ�; - ˮƽ�����ƶ�
		
				pDC->FillSolidRect( rcText.left, nMiddle, rcText.Width() + 2, 1, cif.m_crDisabled );//���Ʒָ���
		
				dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );
				dc.Detach();
		
				return;
		}

		if ( bSelected )//ѡ�к��Ƿ����
		{
				if ( ! bDisabled )
				{       //���Ʊ߿�
						pDC->Draw3dRect(
								rcItem.left + 1, //������ߵ�λ��
								rcItem.top + 1,
								rcItem.Width() - 2, 
								rcItem.Height() - 1,
								cif.m_crBorder,
								cif.m_crBorder
						);
						//��䱳��ɫ
						pDC->FillSolidRect(
								rcItem.left + 2,
								rcItem.top + 2,
								rcItem.Width() - 4,
								rcItem.Height() - 2 - bEdge,
								cif.m_crBackSel
						);
			
						pDC->SetBkColor( cif.m_crBackSel );
				}
				else if ( bKeyboard )
				{
						pDC->Draw3dRect( rcItem.left + 1, rcItem.top + 1,
						rcItem.Width() - 2, rcItem.Height() - 1 - bEdge,
						cif.m_crBorder, cif.m_crBorder );
						pDC->FillSolidRect( rcItem.left + 2, rcItem.top + 2,
						rcItem.Width() - 4, rcItem.Height() - 3 - bEdge,
						cif.m_crBackNormal );
			
						pDC->SetBkColor( cif.m_crBackNormal );
				}
		}
		else
		{
				pDC->SetBkColor( cif.m_crBackNormal );
		}
	
		if ( bChecked )
		{
				pDC->Draw3dRect( rcItem.left + 2, rcItem.top + 2, 20, rcItem.Height() - 2 - bEdge, cif.m_crBorder, cif.m_crBorder );
				pDC->FillSolidRect( rcItem.left + 3, rcItem.top + 3, 18, rcItem.Height() - 4 - bEdge, ( bSelected && !bDisabled ) ? cif.m_crBackCheckSel : cif.m_crBackCheck );
		}
	
		//���Ʋ˵�ͼ��

		nIcon = cif.ImageForID( (DWORD)lpDrawItemStruct->itemID );//=0

		/*if ( bChecked && nIcon < 0 ) nIcon = m_nCheckIcon;*///ԭ���ĸ�ʽ, ���޸ĳ����¸�ʽ
		
		if ( bChecked && nIcon < 0 ) 
		{
			nIcon = m_nCheckIcon;
		}
	
		if ( nIcon >= 0 )
		{
			CPoint pt( rcItem.left + 4, rcItem.top + 4 );//ͼ��λ��
		
			if ( bDisabled )//���˵������ʱ
			{
				ImageList_DrawEx( cif.m_GrayImageList.m_hImageList, cif.GrayImageForID((DWORD)lpDrawItemStruct->itemID), pDC->GetSafeHdc(), pt.x, pt.y, 0, 0, CLR_NONE, cif.m_crDisabled, CM_DISABLEDBLEND );
			}
			else
			{
				if ( bChecked )//���˵����Ǹ�ѡʱ
				{
					cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_NORMAL );
				}
				else
				{
					if ( bSelected )//���˵���ѡ��ʱ
					{
						pt.Offset( 1, 1 );
						pDC->SetTextColor( cif.m_crShadow );
						cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_MASK );//�Ȼ���ͼ�����Ӱ
						pt.Offset( -2, -2 );//�����Ϸ��ƶ�2������
						cif.m_ImageList.Draw( pDC, nIcon, pt, ILD_NORMAL );//�ٻ���ͼ��
					}
					else
					{
						ImageList_DrawEx( cif.m_ImageList.m_hImageList, nIcon, pDC->GetSafeHdc(),pt.x, pt.y, 0, 0, CLR_NONE, cif.m_crMargin, ILD_BLEND25 );
					}
				}
			}
		}
	
		CFont* pOld = (CFont*)pDC->SelectObject(( lpDrawItemStruct->itemState & ODS_DEFAULT ) && ! bDisabled ? &cif.m_fntBold : &cif.m_fntUP );
	
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( bDisabled ? cif.m_crDisabled :( bSelected ? cif.m_crCmdTextSel : cif.m_crCmdText ) );
		DrawMenuText( pDC, &rcText, strText );
	
		pDC->SelectObject( pOld );
	
		dc.BitBlt( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
		rcItem.Width(), rcItem.Height(), pDC, 0, 0, SRCCOPY );

		dc.Detach();
}

/****************************************************************************
                          
������:
       DrawMenuText(CDC *pDC, CRect *pRect, const CString &strText)

��������:
      	���Ʋ˵����ı�

�����������õĺ����嵥:
      		
���ñ������ĺ����嵥:
					  CMenuXP::DrawItem
      
����:
     CDC				*pDC 
	 CRect				*pRect
	 const CString		&strText

����ֵ:
	    void
����:
  
****************************************************************************/

void CMenuXP::DrawMenuText(CDC *pDC, CRect *pRect, const CString &strText)
{
int nPos = strText.Find( '\t' );// '/t'������
	
	if ( nPos >= 0 )
	{
		pRect->right -= 8;
		pDC->DrawText( strText.Left( nPos ), pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );//�����ı������У������
		pDC->DrawText( strText.Mid( nPos + 1 ), pRect, DT_SINGLELINE|DT_VCENTER|DT_RIGHT );//�����ı����Ҷ���
		pRect->right += 8;
	}
	else
	{
		pDC->DrawText( strText, pRect, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
	}
}

BOOL CMenuXP::AddMenu(CMenu *pMenu, BOOL bChild)
{
	
	if (  !m_bEnable )
	{
			return FALSE;
	}

	for ( int i = 0 ; i < (int)pMenu->GetMenuItemCount() ; i++ )//�����˵���
	{
		TCHAR szBuffer[128];
		MENUITEMINFO mii;//�˵�����Ϣ��
		//���˵�����Ϣ��
		ZeroMemory( &mii, sizeof(mii) ); //����㣨��ʼ���˵�����Ϣ����
		mii.cbSize		= sizeof(mii);//�˵�����Ϣ����С
		
		//�˵������루�˴�Ϊ����Ҫ����dwItemData��wID��fType and dwTypeData��hSubMenu�
		mii.fMask		= MIIM_DATA|MIIM_ID|MIIM_TYPE|MIIM_SUBMENU;
		
		mii.dwTypeData	= szBuffer;//�˵����ı�
		mii.cch			= 128;//�˵����ı�����
		
		//�õ���װ�Ĳ˵�����Ϣ
		GetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
		
		if ( mii.fType & (MF_OWNERDRAW|MF_SEPARATOR) )
		{
			mii.fType |= MF_OWNERDRAW;
			if ( mii.fType & MF_SEPARATOR ) mii.dwItemData = 0;
			SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
			continue;
		}
		
		mii.fType		|= MF_OWNERDRAW;
		mii.dwItemData	 = ( (DWORD)pMenu->GetSafeHmenu() << 16 ) | ( mii.wID & 0xFFFF );
		
		CString strText = szBuffer;
		m_pStrings.SetAt(static_cast<DWORD>(mii.dwItemData), strText );
		
		if ( bChild ) SetMenuItemInfo( pMenu->GetSafeHmenu(), i, MF_BYPOSITION, &mii );
		
		if ( mii.hSubMenu != NULL ) AddMenu( pMenu->GetSubMenu( i ), TRUE );
	}
	return TRUE;
}

void CMenuXP::DrawWatermark(CDC *pDC, CRect *pRect, int nOffX, int nOffY)
{
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += m_czWatermark.cy )
	{
		if ( nY + m_czWatermark.cy < pRect->top ) continue;
		
		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += m_czWatermark.cx )
		{
			if ( nX + m_czWatermark.cx < pRect->left ) continue;
			
			pDC->BitBlt( nX, nY, m_czWatermark.cx, m_czWatermark.cy, &m_dcWatermark, 0, 0, SRCCOPY );
		}
	}
}

void CMenuXP::SetWatermark(HBITMAP hBitmap)
{
if ( m_bmWatermark.m_hObject != NULL )
	{
		m_dcWatermark.SelectObject( CBitmap::FromHandle( m_hOldMark ) );
		m_bmWatermark.DeleteObject();
		m_dcWatermark.DeleteDC();
	}
	
	if ( hBitmap != NULL )
	{
		CDC dc;		  
		dc.Attach( GetDC( 0 ) );
		m_dcWatermark.CreateCompatibleDC( &dc );
		ReleaseDC( 0, dc.Detach() );
		
		m_bmWatermark.Attach( hBitmap );
		m_hOldMark = (HBITMAP)m_dcWatermark.SelectObject( &m_bmWatermark )->GetSafeHandle();
		
		BITMAP pInfo;
		m_bmWatermark.GetBitmap( &pInfo );
		m_czWatermark.cx = pInfo.bmWidth;
		m_czWatermark.cy = pInfo.bmHeight;
	}
}

LPCTSTR CMenuXP::wpnOldProc		= _T("BLINKCD_MenuOldWndProc");
BOOL	CMenuXP::m_bPrinted		= TRUE;
HHOOK	CMenuXP::m_hMsgHook		= NULL;
int		CMenuXP::m_nEdgeLeft	= 0;
int		CMenuXP::m_nEdgeTop		= 0;
int		CMenuXP::m_nEdgeSize	= 0;

void CMenuXP::RegisterEdge(int nLeft, int nTop, int nLength)
{
		m_nEdgeLeft	= nLeft;
		m_nEdgeTop	= nTop;
		m_nEdgeSize	= nLength;
}

LRESULT CMenuXP::MenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		WNDPROC pWndProc = (WNDPROC)::GetProp( hWnd, wpnOldProc );
	
	switch ( uMsg )
	{
	case WM_NCCALCSIZE:
		{
			NCCALCSIZE_PARAMS* pCalc = (NCCALCSIZE_PARAMS*)lParam;
			pCalc->rgrc[0].left ++;
			pCalc->rgrc[0].top ++;
			pCalc->rgrc[0].right --;
			pCalc->rgrc[0].bottom --;
		}
		return 0;
		
	case WM_WINDOWPOSCHANGING:
		if ( WINDOWPOS* pWndPos = (WINDOWPOS*)lParam )
		{
			DWORD nStyle	= GetWindowLong( hWnd, GWL_STYLE );
			DWORD nExStyle	= GetWindowLong( hWnd, GWL_EXSTYLE );
			CRect rc( 0, 0, 32, 32 );
			
			AdjustWindowRectEx( &rc, nStyle, FALSE, nExStyle );
			
			pWndPos->cx -= ( rc.Width() - 34 );
			pWndPos->cy -= ( rc.Height() - 34 ) - 1;
			
			if ( pWndPos->x != m_nEdgeLeft || pWndPos->y != m_nEdgeTop )
				pWndPos->x ++;
		}
		break;
		
	case WM_PRINT:
		if ( ( lParam & PRF_CHECKVISIBLE ) && ! IsWindowVisible( hWnd ) ) return 0;
		if ( lParam & PRF_NONCLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CRect rc;
			
			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );
			
			pDC->Draw3dRect( &rc, cif.m_crDisabled, cif.m_crDisabled );
			if ( bEdge ) pDC->FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, cif.m_crBackNormal );
		}
		if ( lParam & PRF_CLIENT )
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CDC* pDC = CDC::FromHandle( (HDC)wParam );
			CBitmap bmBuf, *pbmOld;
			CDC dcBuf;
			CRect rc;
			
			pWnd->GetClientRect( &rc );
			dcBuf.CreateCompatibleDC( pDC );
			bmBuf.CreateCompatibleBitmap( pDC, rc.Width(), rc.Height() );
			pbmOld = (CBitmap*)dcBuf.SelectObject( &bmBuf );
			
			m_bPrinted = TRUE;
			dcBuf.FillSolidRect( &rc, GetSysColor( COLOR_MENU ) );
			SendMessage( hWnd, WM_PRINTCLIENT, (WPARAM)dcBuf.GetSafeHdc(), 0 );
			
			pDC->BitBlt( 1, 1, rc.Width(), rc.Height(), &dcBuf, 0, 0, SRCCOPY );
			dcBuf.SelectObject( pbmOld );
		}
		return 0;
		
	case WM_NCPAINT:
		{
			CWnd* pWnd = CWnd::FromHandle( hWnd );
			CWindowDC dc( pWnd );
			CRect rc;
			
			pWnd->GetWindowRect( &rc );
			BOOL bEdge = ( rc.left == m_nEdgeLeft && rc.top == m_nEdgeTop );
			rc.OffsetRect( -rc.left, -rc.top );
			
			dc.Draw3dRect( &rc,cif.m_crDisabled, cif.m_crDisabled );
			if ( bEdge ) dc.FillSolidRect( rc.left + 1, rc.top, min( rc.Width(), m_nEdgeSize ) - 2, 1, cif.m_crBackNormal );
		}
		return 0;
		
	case WM_PAINT:
		m_bPrinted = FALSE;
		break;
		
	case WM_NCDESTROY:
		::RemoveProp( hWnd, wpnOldProc );
		break;
	}
	
	return CallWindowProc( pWndProc, hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK CMenuXP::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* pCWP = (CWPSTRUCT*)lParam;
	
	while ( nCode == HC_ACTION )
	{
		if ( pCWP->message != WM_CREATE && pCWP->message != 0x01E2 ) break;
		
		TCHAR szClassName[16];
		int nClassName = GetClassName( pCWP->hwnd, szClassName, 16 );
		if ( nClassName != 6 || _tcscmp( szClassName, _T("#32768") ) != 0 ) break;
		
		if ( ::GetProp( pCWP->hwnd, wpnOldProc ) != NULL ) break;
		
		HWND hWndFore = GetForegroundWindow();
		if ( hWndFore != NULL && CWnd::FromHandlePermanent( hWndFore ) == NULL ) break;
		
		WNDPROC pWndProc = (WNDPROC)::GetWindowLong( pCWP->hwnd, GWL_WNDPROC );
		if ( pWndProc == NULL ) break;
		ASSERT( pWndProc != MenuProc );
		
		if ( ! SetProp( pCWP->hwnd, wpnOldProc, pWndProc ) ) break;
		
		if ( ! SetWindowLong( pCWP->hwnd, GWL_WNDPROC, (DWORD)MenuProc ) )
		{
			::RemoveProp( pCWP->hwnd, wpnOldProc );
			break;
		}
		
		break;
	}
	
	return CallNextHookEx( CMenuXP::m_hMsgHook, nCode, wParam, lParam );

}

void CMenuXP::EnableHook(BOOL bEnable)
{
if ( bEnable == ( m_hMsgHook != NULL ) ) return;
	
	if ( bEnable )
	{
		m_hMsgHook = SetWindowsHookEx( 
			WH_CALLWNDPROC,				//ϵͳ����Ϣ���͵�ָ������֮ǰ��"����"
			MsgHook,					//���Ӵ�������
			AfxGetInstanceHandle(),		//���Ӵ�����������ģ��ľ��
			GetCurrentThreadId()        //ָ�������ӵ��߳�
		);
	}
	else
	{
		UnhookWindowsHookEx( m_hMsgHook );
		m_hMsgHook = NULL;
	}
}

void CMenuXP::EnableHook()
{
	ASSERT( m_hMsgHook == NULL );
	ASSERT( m_bUnhook == FALSE );
	m_bEnable		= TRUE;
	
	m_bUnhook = TRUE;
	EnableHook( TRUE );
}

UINT CMenuXP::TrackPopupMenu(CMenu* pszMenu, const CPoint& point, UINT nDefaultID, UINT nFlags, CWnd* pWnd)
{
	CMenu* pPopup = pszMenu;
	if ( pPopup == NULL ) return 0;
	
	if ( nDefaultID != 0 )
	{
		MENUITEMINFO pInfo;
		pInfo.cbSize	= sizeof(pInfo);
		pInfo.fMask		= MIIM_STATE;
		GetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
		pInfo.fState	|= MFS_DEFAULT;
		SetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
	}
	
	return pPopup->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|nFlags, point.x, point.y, pWnd);
}