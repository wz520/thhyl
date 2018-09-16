#ifndef __WINDOWGLUER_H__599C19FB_
#     define __WINDOWGLUER_H__599C19FB_

#include <windows.h>

// written by wz520

// �ര���Զ�����+һ���ƶ�
// 
// �ƶ��Ӵ���(follower)ʱ�����Զ�������������(main)�ϡ�
// �ƶ�������ʱ���Զ��������ŵ��Ӵ���һ���ƶ���

class CWindowGluer
{
public:
	// hWndMain: �����ھ��
	// nFollowersCount: ׼����ӵ��Ӵ��ڵ�������Ҳ������ addFolower() �ĵ��ô���
	CWindowGluer( HWND hWndMain, int nFollowersCount );
	~CWindowGluer();

	// ����һ���Ӵ��ڡ����ô������ܳ��� nFollowersCount�����򷵻� false
	bool addFollower(HWND hWndFollower);
	// ���������Լ��� WindowProc �е��ñ���������ʵ���������ŵ��Ӵ���һ���ƶ���Ч��
	// ���� true ��ʾ������ uMsg ��Ϣ��������ͨ��Ӧ�þ��� return 0��
	bool handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// �Ӵ������Լ��� WindowProc �е��ñ���������ʵ�ֿ���������ʱ���������ڵ�Ч��
	// ���� true ��ʾ������ uMsg ��Ϣ��������ͨ��Ӧ�þ��� return 0��
	bool handleFollowerMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// �����ڻ��Ӵ������Լ��ġ�PreTranslateMessage()���е��ñ���������ʵ�ְ� CTRL+F6 ������ӵ�����Ĵ���Ϊ�����л�Ϊ�����
	// ���� true ��ʾ������ uMsg ��Ϣ��������ͨ��Ӧ�þ��� return TRUE��
	bool handleSwitchMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ��������ֵԽ��������Խǿ��Ĭ��ֵ�� 10
	// Ŀǰ���ֱܷ�Ϊÿ������ָ����ͬ��ֵ��һ��Ҳû�����Ҫ�ɡ�����
	LONG m_lGluePower;

	// typedefs
	// [1],[2] are just placeholders, may vary depend on m_nFollowersCount
	union GLUE_WINDOWS {
		struct {
			HWND main;
			HWND follower[1];
		};
		struct {
			HWND all[2];
		};
	};
	union GLUE_RECTS {
		struct {
			RECT main;
			RECT follower[1];
		};
		struct {
			RECT all[2];
		};
	};

private:
	CWindowGluer& operator = (const CWindowGluer& another);
	CWindowGluer(const CWindowGluer& another);

	void init();
	void clear();

	int m_nFollowersCount;
	int m_nObjectCount;     // means (followers + main), so always == (m_nFollowersCount + 1)

	GLUE_WINDOWS *m_pWindows;
	GLUE_RECTS *m_pRects;

	bool *m_pIsGluedFollower;  // �ж�ĳ���Ӵ����Ƿ���Ҫ��������һ���ƶ�
};


#endif /* __WINDOWGLUER_H__599C19FB_ */
