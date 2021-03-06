## 较旧的更新历史

1.70(2014-06-03): 

- 增加：支持 `弹幕天邪鬼`。可显示主道具、副道具的名称及可用次数，还有主道具的能力值，以及 BOSS 名、 SC 名（需要 th143sc.lst）。
- 增加：`文花帖` 增加 BOSS 名及 SC 名显示，需要 th95sc.lst 。
- 增加：`文花帖DS` 增加 BOSS 名及 SC 名显示，需要 th125sc.lst 。
- 增加：`风神录`、`地灵殿`、`星莲船`、`妖精大战争`、`神灵庙`、`辉针城` 增加各关自机初始坐标显示。
- 增加：在主窗口中按住Ctrl键的同时滚动鼠标滚轮可以快速调整录像信息框的字体大小。
- 增加：在主窗口中，当输入焦点不位于“录像信息”框时，按住鼠标左键的同时滚动鼠标滚轮可以调整主窗口的不透明度。
- 增加：`文花帖` 也可显示 [※ ShotSlow] 和 [※ NoDirectInput] 。
- 增加：本来 [PlayTime](#PlayTime) 和 [SlowRateByFPS](#SlowRateByFPS) 的显示只支持 `红魔乡` ~ `花映冢`，现支持全作。
- BUG修正：使用“重命名/移动”功能后，切换到别的程序再切换回来，文件会被自动关闭。如果在选项中勾选“文件不存在时自动退出”，则程序会自动退出。
- 其他细节的修正。

1.66(2013-09-09): 

- 增加：辉针城各关数据中增加 [Non2.0Bonus] 项，非 2.0 道具回收奖励的次数。详见帮助说明。
- 更改：右上角“关闭文件”按钮的图案（原来的“X”不大好，像是删除文件），并将快捷键从 F9 改为 CTRL+L（本版开始F9另有他用）
- 增加：“选项”中增加“文件不存在时自动退出”：若打开的录像不再有效，则自动退出回映录。
- 增加：“打开文件”快捷键增加 CTRL+O。
- 增加："剪切"、"复制"按钮和菜单项。类似资源管理器的剪切/复制功能，配合“粘贴”功能可以进行移动/复制打开的录像文件。快捷键分别为 F9，F10。
- 增加：快捷键 F11 可以 最大化/还原 主窗口和注释编辑窗口。
- 其他细节的修正。

1.65(2013-08-18): 

- 增加：支持 辉针城正式版 的各关数据显示及 raw 数据保存。
- 增加：软件包中加入 th14sc.lst 文件，作用类似 th13sc.lst，用以显示 辉针城正式版 SpellCard Practice 录像的符卡名称，参见帮助中关于 th13sc.lst 的条目。
- 增加：主界面中增加“关闭文件”按钮，快捷键 F9。

1.62(2013-07-04):

- 辉针城体验版各关数据增加 [1UPCount] 项，累计的奖命次数。详见帮助。
- 原来在 Vista/Win7 下设置文件关联需要管理员权限，现在不需要了。
- 现在若不勾选“选项”对话框中的“双击鼠标左键退出程序”，双击时会最大化/还原窗口。
- “选项”对话框中增加选项“位于非标题栏也可拖动”。
- 把主窗口的按钮调大了，在大字体环境下看起来能舒服点。
- 其他细节的修正。

1.61(2013-05-30):

- BUG修正：（1.60版起）拖入大于32M的文件会导致无限询问是否打开。
- BUG修正：（1.60版起）从别的窗口切换回主窗口时录像信息框都会滚到开头几行，导致查看信息不便。
- 添加： 选项中增加“打开复制后的目标文件”。
- 添加： 为“选项”对话框的一些选项添加了帮助信息（鼠标移动到选项上可见。其实 1.60 版中主窗口的部分控件已经有帮助信息了）。
- 其他细节的修正。

1.60(2013-05-27):

- 增加：支持 辉针城体验版 的各关数据显示及 raw 数据保存。
- 增加：支持 黄昏酒场 的各关数据显示（也就只有“Clear Score”一项而已）及 raw 数据保存。
- 增加：风神录 增加 連擊槽 项。详见帮助说明。
- 增加：监视录像文件变化。每当从别的窗口切换回主窗口后会尝试重新载入录像文件。
- 增加：“打开录像” 菜单项增加快捷键 CTRL+B 。
- 调整了主界面部分控件的外观和位置。
- 其他细节的修正。

1.50(2012-02-21): 

- 增加/改进：[PlayTime] 和 [TotalPlayTime] 使用按键状态数据来计算（本来使用 FPS 数据），这样更精确（精确到帧），并且会显示每一关的总帧数，更重要的是可以支持没有 FPS 数据的红魔乡。
- 增加：红魔乡 显示蓝点数 [Point] 。和游戏中显示的不同，是从游戏开始累计的，类似后续作品。
- 增加：显示录像玩家在游戏前开启的部分游戏选项，详见 “※ 其他信息”。
- 增加：花映囧显示两个玩家的 [Charge Type] 。
- 增加：花映囧显示 Match 模式的对战场所 [Place] 。非 Match 模式不会显示。
- 增加：主窗口标题中显示打开的录像文件名（像 “文件名 - 东方回映录” 这样）
- 增加：“选项”窗口中增加一选项用于将花映冢各关信息中的“Cirno”显示成“⑨”。
- BUG修正: 地灵殿 魔理沙A（魔爱组） [Power] 显示错误。
- 其他细节的修正。

1.40(2011-09-27):

- YoooooYooooo梦 录像可显示是否是中文版。
- 对于 呻吟喵 Spell Practice（符卡练习）录像，会显示 SpellCard 名称和编号。（不过显示名称需要 th13sc.lst ）
- 支持将 .rpy 解码后的原始数据另存（右键菜单->保存 raw 数据，或者按快捷键 Ctrl+R）。不过目前还不支持黄昏酒场。
- 为一些菜单项添加了快捷键。
- 记忆窗口位置和大小。
- 现在使用快捷键 F1~F4 或 右键菜单->编码 可以快速更改 录像信息 和 注释 的 编码，不需要进 选项 对话框了（不过用这种方法只能将 录像信息编码 和 注释编码 改成同一种编码）。
- Bug修正：手动在“录像文件”编辑框中随意输入一个文件路径，即使不按回车，之后的一些操作也会错误地针对这个刚输入的路径。由于未作检查，如果路径不合法，会引起各种错误。
- 其他各种乱涂乱改 =v=

1.34(2011-08-18):

- Bug修正：神灵庙正式版“最大得点”小数部分显示错误。

1.33(2011-08-14):

- 支持 神灵喵 1.00a 正式版的各关数据显示。
- 神灵庙的各关数据中新增显示 Player碎片 和 Bomb碎片 的定额（下次 奖命/奖B 所需碎片数）。
- 原来主界面的“帮助”按钮现在用于显示右键菜单，打开帮助文件功能被移到了右键菜单中。
- 其他细节的修正。

1.32(2011-07-02): 

- 风神录、地灵殿、星莲船的各关信息中，“分数”的个位会显示续关次数（最大 9 次），这样就与游戏中的显示完全一致了。
- 主窗口可以改变大小。
- 打开录像对话框可以一次打开多个文件。与拖入操作一样，除第一个文件外，其他文件用新进程打开。
- Bug修正：星莲船的各关信息中如果某一关的初始 Bomb数 是 9 并且带一个以上 Bomb碎片，会显示为 "error" 。
- 其他细节的修正。

1.31(2011-04-16): 

- 增加 神灵喵 体验版 各关数据的显示（可能未来的正式版也照样可以用）。关于灵界槽的显示值请参看各关数据的说明。
- 再次调整了一下各关数据的显示格式。
- 修正了一个BUG：拖入 TH08+ 的录像后，再拖入一个无法打开的文件，会允许编辑注释。
- 其他细节的修正。

1.30(2011-04-01): 

- 增加 TH10, TH11, TH12, TH128 各关数据的显示。
- 略微调整了一下各关数据的显示。
- 其他细节的修正。

1.20(2011-03-19): 

- 增加 SlowRateByFPS 项。详见 SlowRateByFPS 的说明。
- TH07, TH08 增加 SCB Count 项。Spell Card Bonus 取得次数。
- TH08 增加 时刻 项。游戏中的时刻。
- 支持显示 花映囧 的各关信息，包括各关的使用角色和分数。
- 可以自定义是否显示 PlayTime 和 SlowRateByFPS，不过默认不显示，详见说明。
- 为了方便显示花映囧的信息，主窗口变胖了。
- 以前版本的主窗口中的“复制到剪贴板”按钮，无论录像信息框是否有选中的文字，都只会复制全部信息。现在如果没有选区，则复制全部信息；有则只复制选中的。
- 完善了说明文件。
- 修正各种潜在的BUG，比如如果在命令提示符下敲 thhyl 启动程序，cfg 文件名会变成 thcfg 而非 thhyl.cfg ……
- 其他细节的修正。

1.11(2010-08-15):

- Add support for TH128.

1.1(2010-08-04):

- Add 1upPoint, PlayTime for TH06-TH08, 妖率 for TH08.

1.0(2010-04):

- Initial version.
