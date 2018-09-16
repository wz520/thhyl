#include "WindowGluer.h"

#define delete_array_then_null(v) ((delete[] (v)), (v)=NULL)
#define BETWEEN(n,min,max) ((n)>=(min) && (n)<=(max))


inline bool IsKeyDown(int vKey)
{
	return (GetAsyncKeyState(vKey) & 0x8000) ? true : false;
}



// utility functions

// 判断rect1和rect2是否紧挨着
static bool isRectNextTo(LPCRECT rect1, LPCRECT rect2)
{
	return rect1->left == rect2->right && max(rect1->top, rect2->top) <= min(rect1->bottom, rect2->bottom)
		|| rect1->top == rect2->bottom && max(rect1->left, rect2->left) <= min(rect1->right, rect2->right)
		|| rect1->right == rect2->left && max(rect1->top, rect2->top) <= min(rect1->bottom, rect2->bottom)
		|| rect1->bottom == rect2->top && max(rect1->left, rect2->left) <= min(rect1->right, rect2->right);
}


CWindowGluer::CWindowGluer( HWND hWndMain, int nFollowersCount )
	: m_lGluePower(10), m_nFollowersCount(nFollowersCount), m_nObjectCount(nFollowersCount+1)
{
	this->init();
	m_pWindows->main = hWndMain;
}

CWindowGluer::~CWindowGluer()
{
	this->clear();
}

void CWindowGluer::init()
{
	m_pWindows = (GLUE_WINDOWS*)(new HWND[m_nObjectCount]);
	m_pRects = (GLUE_RECTS*)(new RECT[m_nObjectCount]);
	m_pIsGluedFollower = new bool[m_nFollowersCount];
	int i;
	for ( i = 0; i < m_nObjectCount; ++i ) {
		m_pWindows->all[i] = NULL;
		SetRectEmpty(&m_pRects->all[i]);
	}
	for ( i = 0; i < m_nFollowersCount; ++i ) {
		m_pIsGluedFollower[i] = false;
	}
}

void CWindowGluer::clear()
{
	// new 的时候做过类型强制转换，delete 时应该再强制转换回去，否则可能导致无法正常析构对象
	delete[] ((HWND*)m_pWindows);
	m_pWindows = NULL;
	delete[] ((RECT*)m_pRects);
	m_pRects = NULL;
	delete_array_then_null(m_pIsGluedFollower);
}

bool CWindowGluer::addFollower(HWND hWndFollower)
{
	for ( int i = 0; i < m_nFollowersCount; ++i ) {
		if (m_pWindows->follower[i] == NULL) {
			m_pWindows->follower[i] = hWndFollower;
			return true;
		}
	}
	return false;
}

bool CWindowGluer::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_ENTERSIZEMOVE:
		{
			int i;
			// 记录所有窗口的 RECT
			for ( i = 0; i < m_nObjectCount; ++i ) {
				GetWindowRect(m_pWindows->all[i], &m_pRects->all[i]);
			}
			// 检查每个子窗口，设置“一起移动”的标志
			for (  i = 0; i < m_nFollowersCount; ++i ) {
				m_pIsGluedFollower[i] = isRectNextTo(&m_pRects->follower[i], &m_pRects->main);
			}
		}
		return true;
	case WM_MOVING:
		{
			LPRECT pRectThis = (LPRECT)lParam;
			RECT rectOffset = *pRectThis; // make a copy
			OffsetRect(&rectOffset, -m_pRects->main.left, -m_pRects->main.top);

			// move followers
			for ( int i = 0; i < m_nFollowersCount; ++i ) {
				if (m_pWindows->follower[i] != NULL && m_pIsGluedFollower[i]) {
					RECT rectFollower = m_pRects->follower[i]; // make a copy
					OffsetRect(&rectFollower, rectOffset.left, rectOffset.top);
					SetWindowPos(m_pWindows->follower[i], NULL,
							rectFollower.left, rectFollower.top, 0, 0,
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
		}
		return true;
	}

	return false;
}

bool CWindowGluer::handleFollowerMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 是否在 follower 列表中？不在的话就不作任何处理
	bool bIsFollower = false;
	for ( int i = 0; i < m_nFollowersCount; ++i ) {
		if (m_pWindows->follower[i] == hWnd) {
			bIsFollower = true;
			break;
		}
	}
	if ( !bIsFollower )
		return false;
	
	// 处理消息
	bool result = false;
	switch(uMsg) {
	case WM_MOVING:
		{
			LPRECT pRectThis = (LPRECT)lParam;
			RECT &rectMain = m_pRects->main;   // 为了书写方便，创建一个引用
			GetWindowRect(m_pWindows->main, &rectMain);

			const bool bOverlapX = max(rectMain.left, pRectThis->left) <= min(rectMain.right, pRectThis->right); // X轴重合
			const bool bOverlapY = max(rectMain.top, pRectThis->top) <= min(rectMain.bottom, pRectThis->bottom); // Y轴重合

			// follower around main's left
			LONG distance = rectMain.left - pRectThis->right;
			if ( abs(distance) <= m_lGluePower && bOverlapY ) {
				pRectThis->right = rectMain.left; // 吸附
				pRectThis->left += distance; // 调整对边的距离
				result = true;
			}
			// follower around main's top
			distance = rectMain.top - pRectThis->bottom;
			if ( abs(distance) <= m_lGluePower && bOverlapX ) {
				pRectThis->bottom = rectMain.top; // 吸附
				pRectThis->top += distance; // 调整对边的距离
				result = true;
			}
			// follower around main's right
			distance = pRectThis->left - rectMain.right;
			if ( abs(distance) <= m_lGluePower && bOverlapY ) {
				pRectThis->left = rectMain.right; // 吸附
				pRectThis->right -= distance; // 调整对边的距离
				result = true;
			}
			// follower around main's bottom
			distance = pRectThis->top - rectMain.bottom;
			if ( abs(distance) <= m_lGluePower && bOverlapX ) {
				pRectThis->top = rectMain.bottom; // 吸附
				pRectThis->bottom -= distance; // 调整对边的距离
				result = true;
			}
		}
		break;
	}

	return result;
}



bool CWindowGluer::handleSwitchMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool result = false;
	switch(uMsg) {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F6:
			if ( IsKeyDown(VK_CONTROL) ) {
				for ( int i = 0; i < m_nObjectCount; ++i ) {
					if (m_pWindows->all[i] == hWnd) {
						const int nIndexOfNextWindow = i >= m_nObjectCount - 1
							? 0
							: i+1;
						const HWND hWndOfNextWindow = m_pWindows->all[nIndexOfNextWindow];
						if ( IsWindow(hWndOfNextWindow) && IsWindowVisible(hWndOfNextWindow) ) {
							SetForegroundWindow(hWndOfNextWindow);
						}
						break;
					}
				}
				result = true;  // 只要按了正确的热键，就总是返回 true
			}
			break;
		}
		break;
	}

	return result;
}
