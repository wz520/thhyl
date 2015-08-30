#ifndef _THKEYINFO_H_WOLEGEQU
#define _THKEYINFO_H_WOLEGEQU

// key state data structures and constants for replay files(.rpy)
// of Touhou STG series: TH6, TH7, TH8, TH9
// 
// Written by wz520
// Last Updated: 2012-02-21
//

// ----------------- THKEYSTATE DETAILS -----------------------
//
// keystate 是一个 16bit 数
// 其中低 9bit 用于记录某一帧的按键状态，高 7bit 用途不明（应该没用）
//
// keystate & 0x0001 != 0: shoot
// keystate & 0x0002 != 0: bomb
// keystate & 0x0004 != 0: slow
// keystate & 0x0008 != 0: pause
// keystate & 0x0010 != 0: up
// keystate & 0x0020 != 0: down
// keystate & 0x0040 != 0: left
// keystate & 0x0080 != 0: right
// keystate & 0x0100 != 0: skip dialog
//
// 暂停键（ESC 和 KeyConfig 中定义的 Pause 键）也是会被记录下来的
// 例：按ESC，游戏暂停，再按住ESC，但是别松开，游戏继续，继续之后按 ESC 的状态会被记录下来直到松开。
// 【注意】TH6 除外。TH6 中按 暂停键 不会被记录。'pause' 位总是 0 。
//

// ------------------------- 定义 THKEYSTATE ---------------------
// 这是个 union ，根据需要可以直接用 WORD w 访问整个 keystate，
//   也可以用 s 结构中的位域访问其中一个按键的状态。
typedef union tagTHKEYSTATE {
	WORD w;
	struct {
		WORD shoot  : 1;
		WORD bomb   : 1;
		WORD slow   : 1;
		WORD pause  : 1;
		WORD up     : 1;
		WORD down   : 1;
		WORD left   : 1;
		WORD right  : 1;
		WORD skip   : 1;
	}s;
}THKEYSTATE;

// macros for THKEYSTATE::w:
#define THKEYSTATE_SHOOT                        0x0001
#define THKEYSTATE_BOMB                         0x0002
#define THKEYSTATE_SLOW                         0x0004
#define THKEYSTATE_PAUSE                        0x0008
#define THKEYSTATE_UP                           0x0010
#define THKEYSTATE_DOWN                         0x0020
#define THKEYSTATE_RIGHT                        0x0040
#define THKEYSTATE_LEFT                         0x0080
#define THKEYSTATE_SKIP                         0x0100
#define th_getkeystate(keystate,key)            ((keystate) & (key))

/////////////////////////////////////////////////////////////////////////////
// KEYDATA 结构
//
// KEYDATA 结构数组跟在每一关的 TH?_STAGEINFO 结构（thdecode1.h）后面。
// 该结构主要包含 keystate ，记录玩家在游戏中某一帧（或某几帧）的按键状态。
// 每一作的 KEYDATA 数据结构稍有不同，相对 STAGEINFO 结构的偏移量也不同，
// 详见下面。
/////////////////////////////////////////////////////////////////////////////

// TH6 的 keydata 数据是被压缩过的。
// 算法类似 RLE 压缩，即连续相同的 keystate（按键状态） 只记录一次。并且
//   用一个值(frame)标出该 keystate 从哪一帧开始。
// 也就是按键状态一有变化就会产生新的数据，没变化就不会产生。
// 另外，每一关最后一组 keydata 的 frame 必定为 9999999(0x0098967F) 。
//   这是单关 keydata 数组的结束标志，同时也可以用作检查 rpy 文件
//   是否合法的校验码（不是 9999999 就不是合法的 TH06 RPY 神马的）。
//
// TH7 之后就没有压缩了，每一帧都有一组按键数据。但是因为使用了专门的
//   压缩算法将几乎整个 rpy 文件压缩了，所以文件大小和 TH6 差不多
typedef struct tagTH6_KEYDATA {
    DWORD      frame;      // key state starts from which frame
    THKEYSTATE keystate;   // see description above
    WORD       u1;         // seems to be padding
}TH6_KEYDATA;
// TH6 的 keydata 从 TH6_STAGEINFO 结构偏移 0x10 处开始。
// 但是该处的 frame 和 keystate 成员总是 0 。
//   该处确实是 keydata （在该处设内存断点后断下的位置与后面几组 keydata 相同），
//   但是无论玩家在该关开始时按什么键，该 keydata 的 frame 和 keystate 总是 0 。
// 所以这个没用的 keydata 应该被忽略。
// 但是由于 TH6 的数据是压缩的，这里我们并不忽略它。（方便以后的解压缩）
//
// PS: 每一关倒数第 2 个 keydata 的 frame 值即是该关的总 frame 数
static const int OFFSET_TH6KEYDATA = 0x10; 

typedef struct tagTH7_KEYDATA {
    THKEYSTATE keystate; // see description above
    WORD u1;             // seems to be padding
}TH7_KEYDATA;
// TH7 的 keydata 从 TH7_STAGEINFO 结构偏移 0x2C 处开始。
// 但是同样该处的 keystate 总是 0 。
// 类似 TH6 ，详见上面 OFFSET_TH6KEYDATA 的说明
// 这里我们忽略它，从第二个开始
static const int OFFSET_TH7KEYDATA = 0x2C + sizeof(TH7_KEYDATA)*1; // 忽略1个

typedef struct tagTH8_KEYDATA {
	THKEYSTATE keystate; // see description above
}TH8_KEYDATA;
// TH8 的 keydata 从 TH8_STAGEINFO 结构偏移 0x24 处开始。
// 但是同样该处的 keystate 总是 0 。
// 类似 TH6 ，详见上面 OFFSET_TH6KEYDATA 的说明
// 这里我们忽略它，从第二个开始
static const int OFFSET_TH8KEYDATA = 0x24 + sizeof(TH8_KEYDATA)*1; // 忽略1个

typedef TH8_KEYDATA TH9_KEYDATA; // same as TH8_KEYDATA
// TH9 的 keydata 从 TH9_STAGEINFO 结构偏移 0x20 处开始。
// 但是……没有但是了 :) 该偏移处的值正确记录了第 1 帧的按键状态。
static const int OFFSET_TH9KEYDATA = 0x20;

#endif // _THKEYINFO_H_WOLEGEQU
