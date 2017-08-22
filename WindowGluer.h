#ifndef __WINDOWGLUER_H__599C19FB_
#     define __WINDOWGLUER_H__599C19FB_

#include <windows.h>

// written by wz520

// 多窗口自动吸附+一起移动
// 
// 移动子窗口(follower)时，会自动吸附到主窗口(main)上。
// 移动主窗口时会自动与吸附着的子窗口一起移动。

class CWindowGluer
{
public:
	// hWndMain: 主窗口句柄
	// nFollowersCount: 准备添加的子窗口的数量，也限制了 addFolower() 的调用次数
	CWindowGluer( HWND hWndMain, int nFollowersCount );
	~CWindowGluer();

	// 增加一个子窗口。调用次数不能超过 nFollowersCount，否则返回 false
	bool addFollower(HWND hWndFollower);
	// 主窗口在自己的 WindowProc 中调用本方法，能实现与吸附着的子窗口一起移动的效果
	// 返回 true 表示处理了 uMsg 消息，调用者通常应该尽快 return 0。
	bool handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// 子窗口在自己的 WindowProc 中调用本方法，能实现靠近主窗口时吸附主窗口的效果
	// 返回 true 表示处理了 uMsg 消息，调用者通常应该尽快 return 0。
	bool handleFollowerMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 吸附力，值越大吸附力越强，默认值是 10
	// 目前不能分别为每个窗口指定不同的值（一般也没这个必要吧……）
	LONG m_lGluePower;
	
private:
	CWindowGluer& operator = (const CWindowGluer& another);
	CWindowGluer(const CWindowGluer& another);

	void init();
	void clear();

	int m_nFollowersCount;
	int m_nObjectCount;     // means (followers + main), so always == (m_nFollowersCount + 1)

	// [1],[2] is just a placeholder, may vary depend on m_nFollowersCount
	union GLUE_WINDOWS {
		struct {
			HWND main;
			HWND follower[1];
		};
		struct {
			HWND all[2];
		};
	}*m_pWindows;
	union GLUE_RECTS {
		struct {
			RECT main;
			RECT follower[1];
		};
		struct {
			RECT all[2];
		};
	}*m_pRects;

	bool *m_pIsGluedFollower;  // 判断某个子窗口是否需要和主窗口一起移动
};


#endif /* __WINDOWGLUER_H__599C19FB_ */
