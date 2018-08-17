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

// FPS 数据相关结构
typedef struct tagTH_FPSINFO {
    BYTE   *pointers[10];     // 指向各关 FPS 数据的指针
                              // 预留 10 个指针的空间，因为最多有 10 关（TH9）。(TH6 不使用)
    DWORD   sizes[10];        // 各关 FPS 数据的长度。
    double  slowrates[10];    // 根据FPS数据计算出来的各关掉帧率（参考值）
    DWORD   total_size;       // 所有关卡的 FPS 数据长度
    double  total_slowrate;   // 所有关卡的综合掉帧率（参考值）
}TH_FPSINFO;

// 目前主要用于回映录“保存 raw 数据”对话框里导出数据
typedef struct tagTH_STAGEPOINTER {
	BYTE  *p;        // 指针
	int    offset;   // 指针相对于 raw 数据起始处的偏移
	DWORD  size;     // 数据大小
}TH_STAGEPOINTER;


#define THRPYINFO_MAXSTAGES 16

// 将被 THRPYINFO 和 THRPYINFO2 继承
class THRPYINFO_BASE
{
public:
	TH_FPSINFO		fpsinfo;    // FPS 数据相关结构
	TH_STAGEPOINTER stagepointers[2][THRPYINFO_MAXSTAGES]; // 关卡指针信息，最多 THRPYINFO_MAXSTAGES 个，但是 TH9 有 2 个 PLAYER
	TH_STAGEPOINTER fpspointers[THRPYINFO_MAXSTAGES]; // 与 fpsinfo 不同，这里一个索引对应一个固定关卡，而 fpsinfo 在 THRPYINFO2 中则不一定
	LPCTSTR			stagenames[THRPYINFO_MAXSTAGES]; // 关卡名，最多 THRPYINFO_MAXSTAGES 个（妖精大战争）
	int				nStageNumberForHalf;	// 95,125,143 的关卡编号，初始 -1 表示未设置

	void setStageNames(LPCTSTR const newnames[], int start, int length);

protected:
	void initStageNames();
};

/*
 * 根据FPS信息计算掉帧率(SlowRateByFPS)。
 * 与直接写在录像文件里的掉帧率有误差。
 * 主要原因应该是游戏进行过程中每秒的帧数是浮点数，
 * 而存到RPY里的FPS信息都是取整（四舍五入）后的数值（回放时的FPS信息也都是整数）。所以有误差是必然的。
 * 既然有误差为什么还要计算呢？
 * 1. 花映囧不保存掉帧率（也许存了，但我不知道在哪儿...），回放时也不显示录像的FPS信息。
 *    但是实际上RPY文件里却是有FPS信息的。
 *    那么根据FPS信息，就可以显示花映囧的（大致）掉帧率了。
 * 2. 可以显示TH7-TH9每一关的（大致）掉帧率。（TH6不可，因为无FPS信息）
 */
double CalcSlowRate(BYTE* fpsData, DWORD fpsDataSize);


#endif /* __THDEFINES_H__5387D12E_ */
