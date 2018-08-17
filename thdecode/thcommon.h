#ifndef __THDEFINES_H__5387D12E_
#     define __THDEFINES_H__5387D12E_


// // #define tchar
// #if defined(_WIN32) || defined(WIN32)
//
// // # ifndef _MFC_VER
// // #  include <windows.h>
// // # endif
// # include <windef.h>
// # include <tchar.h>
//
// #else

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef int BOOL;

#ifndef _INC_TCHAR
# include <tchar.h>
#else
# ifdef UNICODE
#  ifndef _T
#  define _T(x)			L ## x
#  endif
# else
#  ifndef _T
#  define _T(x)			x
#  endif
# endif
#endif

#ifdef UNICODE
typedef const unsigned short*	   LPCTSTR;
#else
typedef const char*	   LPCTSTR;
#endif

// define DWORD
#if defined(_WIN32) || defined(WIN32)
typedef unsigned long  DWORD; // Using Windows standard
#elif defined(__UINT32_TYPE__)
typedef uint32_t  DWORD;
#elif defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ != 4
typedef unsigned int  DWORD;
#else
typedef unsigned long  DWORD;
#endif // _WIN32 || WIN32

// RPY Magic number enums
typedef enum tagRPYMGC {
	mgcnone = 0,      // is not a valid touhou rpy
	mgc6	= 'PR6T',
	mgc7	= 'PR7T',
	mgc8	= 'PR8T',
	mgc9	= 'PR9T',
    mgcalco = 'r1la',
    mgc95   = 'r59t',
    mgc10   = 'r01t',
    mgc11   = 'r11t',
    mgc12   = 'r21t',
    mgc125  = '521t',
    mgc128  = 'r821',
    mgc13   = 'r31t',
	mgc143  = '341t',
	mgc15   = 'r51t',
	mgc16   = 'r61t',
	mgc165  = '651t'
}RPYMGC;

// FPS ������ؽṹ
typedef struct tagTH_FPSINFO {
    BYTE   *pointers[10];     // ָ����� FPS ���ݵ�ָ��
                              // Ԥ�� 10 ��ָ��Ŀռ䣬��Ϊ����� 10 �أ�TH9����(TH6 ��ʹ��)
    DWORD   sizes[10];        // ���� FPS ���ݵĳ��ȡ�
    double  slowrates[10];    // ����FPS���ݼ�������ĸ��ص�֡�ʣ��ο�ֵ��
    DWORD   total_size;       // ���йؿ��� FPS ���ݳ���
    double  total_slowrate;   // ���йؿ����ۺϵ�֡�ʣ��ο�ֵ��
}TH_FPSINFO;

// Ŀǰ��Ҫ���ڻ�ӳ¼������ raw ���ݡ��Ի����ﵼ������
typedef struct tagTH_STAGEPOINTER {
	BYTE  *p;        // ָ��
	int    offset;   // ָ������� raw ������ʼ����ƫ��
	DWORD  size;     // ���ݴ�С
}TH_STAGEPOINTER;


#define THRPYINFO_MAXSTAGES 16

// ���� THRPYINFO �� THRPYINFO2 �̳�
class THRPYINFO_BASE
{
public:
	TH_FPSINFO		fpsinfo;    // FPS ������ؽṹ
	TH_STAGEPOINTER stagepointers[2][THRPYINFO_MAXSTAGES]; // �ؿ�ָ����Ϣ����� THRPYINFO_MAXSTAGES �������� TH9 �� 2 �� PLAYER
	TH_STAGEPOINTER fpspointers[THRPYINFO_MAXSTAGES]; // �� fpsinfo ��ͬ������һ��������Ӧһ���̶��ؿ����� fpsinfo �� THRPYINFO2 ����һ��
	LPCTSTR			stagenames[THRPYINFO_MAXSTAGES]; // �ؿ�������� THRPYINFO_MAXSTAGES ����������ս����
	int				nStageNumberForHalf;	// 95,125,143 �Ĺؿ���ţ���ʼ -1 ��ʾδ����

	void setStageNames(LPCTSTR const newnames[], int start, int length);

protected:
	void initStageNames();
};

/*
 * ����FPS��Ϣ�����֡��(SlowRateByFPS)��
 * ��ֱ��д��¼���ļ���ĵ�֡������
 * ��Ҫԭ��Ӧ������Ϸ���й�����ÿ���֡���Ǹ�������
 * ���浽RPY���FPS��Ϣ����ȡ�����������룩�����ֵ���ط�ʱ��FPS��ϢҲ����������������������Ǳ�Ȼ�ġ�
 * ��Ȼ�����Ϊʲô��Ҫ�����أ�
 * 1. ��ӳ�岻�����֡�ʣ�Ҳ����ˣ����Ҳ�֪�����Ķ�...�����ط�ʱҲ����ʾ¼���FPS��Ϣ��
 *    ����ʵ����RPY�ļ���ȴ����FPS��Ϣ�ġ�
 *    ��ô����FPS��Ϣ���Ϳ�����ʾ��ӳ��ģ����£���֡���ˡ�
 * 2. ������ʾTH7-TH9ÿһ�صģ����£���֡�ʡ���TH6���ɣ���Ϊ��FPS��Ϣ��
 */
double CalcSlowRate(BYTE* fpsData, DWORD fpsDataSize);


#endif /* __THDEFINES_H__5387D12E_ */
