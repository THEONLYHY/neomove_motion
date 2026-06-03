#pragma once

#define NEOMOVEAPIRETURN extern "C" __declspec(dllexport) int __stdcall

//函数返回值错误码
#define NM_RETURN_OK                            0       //函数执行OK
#define NM_RETURN_ERROR_NOTOPEN                 -901    //控制器未打开
#define NM_RETURN_ERROR_ALREADYOPEN             -902    //控制器已经打开
#define NM_RETURN_ERROR_LIBNOTOPEN              -903    //运动库未连接
#define NM_RETURN_ERROR_HOMETYPEERROR           -904    //回零模式错误
#define NM_RETURN_ERROR_MOTIONLIB_TIMEOUT       -905    //运动库连接超时
#define NM_RETURN_ERROR_NOTSUPPORT              -906    //功能不支持
#define NM_RETURN_ERROR_TRIGGER_ENCODERINDEX    -907    //触发模块编码器序号错误
#define NM_RETURN_ERROR_TRIGGER_COMPARE         -908    //触发模块比较器序号错误
#define NM_RETURN_ERROR_TRIGGER_OUTPUT          -909    //触发模块输出序号错误
#define NM_RETURN_ERROR_TORQUEMODE              -910    //力矩模式错误
#define NM_RETURN_ERROR_ISHOMING                -911    //正在回零
#define NM_RETURN_ERROR_DOWNLOADPROJECT_FAIL    -993    //下载工程文件失败
#define NM_RETURN_ERROR_CONTROLLER_THREADTIMEOUT    -994    //控制器通讯线程阻塞
#define NM_RETURN_ERROR_CONTROLLER_INDEX        -995    //控制器序号错误
#define NM_RETURN_ERROR_CONTROLLER_COMMANDLENGTH    -996    //控制器指令长度错误
#define NM_RETURN_ERROR_CONTROLLER_TIMEOUT      -997    //控制器通讯超时
#define NM_RETURN_ERROR_CONTROLLER_DISCONNECT   -998    //控制器未连接
#define NM_RETURN_ERROR_WRONGCOMMAND            -999    //指令错误
#define NM_RETURN_ERROR_TIMEOUT                 -1000   //指令应答超时
#define NM_RETURN_ERROR_DATACOUNT               -1002   //指令交互数据数量错误
#define NM_RETURN_ERROR_NODATA                  -1003   //指令交互无数据
#define NM_RETURN_ERROR_PLANFAIL                -1004   //速度规划错误
#define NM_RETURN_ERROR_GROUPENABLE             -1005   //Group已启用
#define NM_RETURN_ERROR_GROUPDISABLE            -1006   //Group未启用
#define NM_RETURN_ERROR_AXISINDEX               -1007   //轴号错误
#define NM_RETURN_ERROR_AXISBUSY                -1008   //轴正忙
#define NM_RETURN_ERROR_VELOCITYTYPE            -1009   //速度模式错误
#define NM_RETURN_ERROR_CHANNELNUMBER           -1010   //通道号错误
#define NM_RETURN_ERROR_GANTRYSLAVEOVER         -1011   //龙门从轴数量满
#define NM_RETURN_ERROR_GROUPBUSY               -1012   //Group正忙
#define NM_RETURN_ERROR_MOVEMODE                -1013   //运动模式错误
#define NM_RETURN_ERROR_OUTOFRANGE              -1014   //超出范围
#define NM_RETURN_ERROR_MOVEDIR                 -1015   //运动方向错误
#define NM_RETURN_ERROR_FUNCTIONNOTSUPPORT      -1016   //功能不支持
#define NM_RETURN_ERROR_ROBOTINDEX              -1017   //机械手序号错误
#define NM_RETURN_ERROR_ROBOTOUTOFRANGE         -1018   //机械手位置超出范围
#define NM_RETURN_ERROR_ROBOTDISABLE            -1019   //机械手未启用
#define NM_RETURN_ERROR_LIBALARM                -1020   //运动库报警
#define NM_RETURN_ERROR_AUTOMODE                -1021   //非自动模式
#define NM_RETURN_ERROR_COORDINATEDISABLE       -1022   //坐标系未设置
#define NM_RETURN_ERROR_NOLATCH                 -1023   //没有捕获位置
#define NM_RETURN_ERROR_LATCHMODE               -1024   //捕获模式错误
#define NM_RETURN_ERROR_LOAD                    1       //实时服务进程启动失败
#define NM_RETURN_ERROR_SYS                     2       //未和控制台连接成功
#define NM_RETURN_ERROR_RUNTIME                 3       //实时服务共享内存打开失败
#define NM_RETURN_ERROR_CONSOLE                 4       //和控制台连接失败，检查NeoMove Monitor是否已经打开
#define NM_RETURN_ERROR_CRC                     5       //文件校验出错。文件可能损坏，重新安装
#define NM_RETURN_ERROR_TEXT                    7       //多语言文件加载错误
#define NM_RETURN_ERROR_LIC                     8       //授权权限不够，在NeoMove Monitor中可查看授权列表信息
#define NM_RETURN_ERROR_CHN                     9       //字符串中存在中文字符,日志文件/实时程序文件中不能含有中文字符
#define NM_RETURN_ERROR_RTOS                    10      //实时系统没有启动
#define NM_RETURN_ERROR_SYS_ALIGN               11      //系统对齐错误
#define NM_RETURN_ERROR_EXIT                    12      //控制台已经退出或者重启，请重新连接
#define NM_RETURN_ERROR_NODE_ENA                13      //主站节点未启用
#define NM_RETURN_ERROR_NOTIME                  14      //加载实时程序失败
#define NM_RETURN_ERROR_NODE_BCD                15      //指定节点未运行
#define NM_RETURN_ERROR_PARA                    16      //输入参数非法
#define NM_RETURN_ERROR_FUN                     17      //不支持的功能
#define NM_RETURN_ERROR_DYNC_MALLOC             18      //内存分配失败
#define NM_RETURN_ERROR_COLD_ERR                19      //系统冷复位后需重启应用程序
#define NM_RETURN_ERROR_INVALID_DOUBLE          20      //非法的浮点数
#define NM_RETURN_ERROR_API_VER                 21      //使用了不用版本的
#define NM_RETURN_ERROR_EBUS                    100     //总线未初始化完成，在NeoMove Monitor中查看总线日志及当前报警
#define NM_RETURN_ERROR_SDO_OV                  101     //SDO列表溢出
#define NM_RETURN_ERROR_SDO_U                   102     //SDO站号非法
#define NM_RETURN_ERROR_SDO_OD                  103     //SDO对象字典不存在或者不可读写
#define NM_RETURN_ERROR_BUSY                    200     //轴正在使用中
#define NM_RETURN_ERROR_READY                   201     //驱动器未使能
#define NM_RETURN_ERROR_LIMIT_H                 202     //已经到硬限位
#define NM_RETURN_ERROR_LIMIT_S                 203     //已经到软限位
#define NM_RETURN_ERROR_AXIS_INDEX              204     //轴序号不存在
#define NM_RETURN_ERROR_MODE_MISMATCH           206     //运动模式不匹配
#define NM_RETURN_ERROR_CIA_CSP                 207     //此功能需要轴切换成CSP模式
#define NM_RETURN_ERROR_CONTI_DATA              300     //连续运动缓冲区内无数据
#define NM_RETURN_ERROR_CONTI_OV                301     //单轴连续运动缓冲区溢出
#define NM_RETURN_ERROR_FOLLOW_MASTER           302     //主轴模式已启用，禁用此功能
#define NM_RETURN_ERROR_CAM_MPOS                303     //凸轮主轴位置要求单调增加
#define NM_RETURN_ERROR_NODE_UNI                304     //联动轴不在同一个主站内
#define NM_RETURN_ERROR_CRD_INIT                400     //坐标系已初始化
#define NM_RETURN_ERROR_CRD_DEINIT              401     //坐标系未初始化
#define NM_RETURN_ERROR_CRD_REPEAT              402     //坐标系轴序号重复
#define NM_RETURN_ERROR_CRD_ACTIVE              403     //坐标系运行中
#define NM_RETURN_ERROR_CRD_AXIS                405     //坐标系内的轴有报警
#define NM_RETURN_ERROR_CRD_WARN                406     //坐标系有报警
#define NM_RETURN_ERROR_CRD_SINGLE              407     //坐标系内的轴处于单轴运行模式
#define NM_RETURN_ERROR_CRD_CSP                 408     //坐标系绑定的轴需要在CSP模式下
#define NM_RETURN_ERROR_CRD_REPEAT2             409     //坐标系外轴和内部轴重合
#define NM_RETURN_ERROR_CRD_PAUSE               410     //坐标系暂停中
#define NM_RETURN_ERROR_CRD_INC                 411     //有相对指令不支持暂停模式
#define NM_RETURN_ERROR_CRD_STOPPING            412     //坐标系正在减速停止
#define NM_RETURN_ERROR_CRD_GANTRY              413     //龙门从轴不能加在轴列表中
#define NM_RETURN_ERROR_PITCH_ENA               500     //在螺距补偿禁用状态下更新配置
#define NM_RETURN_ERROR_PITCH_REPEAT            501     //螺距补偿和2D补偿不能同时启用
#define NM_RETURN_ERROR_PITCH_DATA              502     //补偿值大于了间距的20%
#define NM_RETURN_ERROR_PITCH_DRV               503     //禁止在伺服Ready下启用/禁用机械补偿
#define NM_RETURN_ERROR_COMPENSATION            504     //补偿功能已启用，禁用此功能
#define NM_RETURN_ERROR_UMOTION_OV              750     //自定义算法缓冲区溢出

//控制器类型定义
#define NM_CONTROLLERTYPE_R2_M100		0       //R2-M100，RTEX总线运动控制器
#define NM_CONTROLLERTYPE_R2_M300		1       //R2-M300，RTEX总线嵌入式运动控制器
#define NM_CONTROLLERTYPE_G2_M100		2       //G2-M100，脉冲式网络型运动控制器
#define NM_CONTROLLERTYPE_E2_M200A		4       //E2-M200A，EtherCAT总线运动控制板卡
#define NM_CONTROLLERTYPE_E2_M300		3       //E2-M300，EtherCAT总线运动控制器
#define NM_CONTROLLERTYPE_E2_M100       5       //E2-M100, EtherCAT总线运动控制器（独立网络型）

//EtherCAT轴模式
#define NM_E2M300_AXISMODE_TQ           4       //力矩模式
#define NM_E2M300_AXISMODE_HM           6       //回零模式
#define NM_E2M300_AXISMODE_CSP          8       //CSP模式

//回零模式
#define NM_HOMEMODE_ETHERCAT_1          1       //EtherCAT模式1，负限位+探针
#define NM_HOMEMODE_ETHERCAT_2          2       //EtherCAT模式2，正限位+探针
#define NM_HOMEMODE_ETHERCAT_3          3       //EtherCAT模式3，原点+探针
#define NM_HOMEMODE_ETHERCAT_4          4       //EtherCAT模式4，原点+探针
#define NM_HOMEMODE_ETHERCAT_5          5       //EtherCAT模式5，原点+探针
#define NM_HOMEMODE_ETHERCAT_6          6       //EtherCAT模式6，原点+探针
#define NM_HOMEMODE_ETHERCAT_7          7       //EtherCAT模式7，正限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_8          8       //EtherCAT模式8，正限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_9          9       //EtherCAT模式9，正限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_10         10      //EtherCAT模式10，正限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_11         11      //EtherCAT模式11，负限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_12         12      //EtherCAT模式12，负限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_13         13      //EtherCAT模式13，负限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_14         14      //EtherCAT模式14，负限位+原点+探针
#define NM_HOMEMODE_ETHERCAT_17         17      //EtherCAT模式17，找负限位
#define NM_HOMEMODE_ETHERCAT_18         18      //EtherCAT模式18，找正限位
#define NM_HOMEMODE_ETHERCAT_19         19      //EtherCAT模式19，找原点
#define NM_HOMEMODE_ETHERCAT_20         20      //EtherCAT模式20，找原点
#define NM_HOMEMODE_ETHERCAT_21         21      //EtherCAT模式21，找原点
#define NM_HOMEMODE_ETHERCAT_22         22      //EtherCAT模式22，找原点
#define NM_HOMEMODE_ETHERCAT_23         23      //EtherCAT模式23，正限位+原点
#define NM_HOMEMODE_ETHERCAT_24         24      //EtherCAT模式24，正限位+原点
#define NM_HOMEMODE_ETHERCAT_25         25      //EtherCAT模式25，正限位+原点
#define NM_HOMEMODE_ETHERCAT_26         26      //EtherCAT模式26，正限位+原点
#define NM_HOMEMODE_ETHERCAT_27         27      //EtherCAT模式27，负限位+原点
#define NM_HOMEMODE_ETHERCAT_28         28      //EtherCAT模式28，负限位+原点
#define NM_HOMEMODE_ETHERCAT_29         29      //EtherCAT模式29，负限位+原点
#define NM_HOMEMODE_ETHERCAT_30         30      //EtherCAT模式30，负限位+原点
#define NM_HOMEMODE_ETHERCAT_33         33      //EtherCAT模式33，负方向找探针
#define NM_HOMEMODE_ETHERCAT_34         34      //EtherCAT模式34，正方向找探针
#define NM_HOMEMODE_ETHERCAT_35         35      //EtherCAT模式35，当前位置

//插补段类型
#define NM_SEGMENTTYPE_STARTPOINT       0       //起点
#define NM_SEGMENTTYPE_LINE             1       //直线
#define NM_SEGMENTTYPE_FASTLINE         2       //快速直线
#define NM_SEGMENTTYPE_ARCCW            3       //顺时针圆弧
#define NM_SEGMENTTYPE_ARCCCW           4       //逆时针圆弧
#define NM_SEGMENTTYPE_HELICALCW        5       //顺时针螺旋
#define NM_SEGMENTTYPE_HELICALCCW       6       //逆时针螺旋
#define NM_SEGMENTTYPE_SETOUTPUT        7       //数字输出
#define NM_SEGMENTTYPE_SLEEP            8       //延时

//总线重置模式
#define NM_BUSRESET_WARM                0       //热重置
#define NM_BUSRESET_COLD                1       //冷重置

//示波器数据类型
#define NM_OSCITEMTYPE_AXISINFO         0       //轴信息
#define NM_OSCITEMTYPE_AXISINFO2        1       //双轴信息
#define NM_OSCITEMTYPE_PDO              2       //PDO

//示波器轴信息类型
#define NM_OSCAXISINFO_CMDPOS           0       //指令位置
#define NM_OSCAXISINFO_ACTPOS           1       //实际位置
#define NM_OSCAXISINFO_CMDVEL           2       //指令速度
#define NM_OSCAXISINFO_ACTVEL           3       //实际速度
#define NM_OSCAXISINFO_FOLERROR         4       //跟随误差

//示波器双轴信息类型
#define NM_OSCAXISINFO2_GANTRYFOLERROR  0       //龙门跟随误差

//从站类型
#define NM_SLAVETYPE_UNKNOWN            0       //未知类型
#define NM_SLAVETYPE_STEPMOTOR          1       //步进电机
#define NM_SLAVETYPE_SERVOMOTOR         2       //伺服电机
#define NM_SLAVETYPE_R2_S600            3       //R2-S600，RTEX总线步进驱动器从站
#define NM_SLAVETYPE_R2_S244            4       //R2-S244，RTEX总线IO从站

//运动库速度类型
#define NM_VELOCITYTYPE_S               0       //S曲线
#define NM_VELOCITYTYPE_T               1       //T曲线
#define NM_VELOCITYTYPE_S5              2       //S5曲线

//EtherCAT主站状态
#define NM_MASTERSTATUS_STOP            0       //停止
#define NM_MASTERSTATUS_RUN             1       //停止
#define NM_MASTERSTATUS_TRANSITION      2       //过渡

//位置捕获模式
#define NM_LatchPositionMode_POSLIMIT_RISINGEDGE        0       //正限位上升沿
#define NM_LatchPositionMode_POSLIMIT_FALLINGEDGE       1       //正限位下降沿
#define NM_LatchPositionMode_NEGLIMIT_RISINGEDGE        2       //负限位上升沿
#define NM_LatchPositionMode_NEGLIMIT_FALLINGEDGE       3       //负限位下降沿
#define NM_LatchPositionMode_HOME_RISINGEDGE            4       //原点上升沿
#define NM_LatchPositionMode_HOME_FALLINGEDGE           5       //原点下降沿

/// <summary>
/// 控制器版本信息
/// </summary>
typedef struct NM_Version
{
	/// <summary>
	/// 运动库版本
	/// </summary>
	int motionLibVersion;

	/// <summary>
	/// 核心库版本
	/// </summary>
	int coreVersion;

	/// <summary>
	/// 控制器版本
	/// </summary>
	int controllerVersion;

	/// <summary>
	/// dll版本
	/// </summary>
	int dllVersion;

    /// <summary>
    /// 网络库版本
    /// </summary>
    int netDllVersion;
}NM_VERSION;

/// <summary>
/// 单轴状态
/// </summary>
typedef struct NM_AxisStatus
{
public:
    /// <summary>
    /// 伺服上电，1：已上电，0：未上电
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int servoOn;

    /// <summary>
    /// 伺服离线，1：离线，0：在线
    /// 支持控制器：R2-M100，R2-M300
    /// </summary>
    unsigned int servoOffline;

    /// <summary>
    /// 驱动器报警，1：有报警，0：无报警
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int ampAlarm;

    /// <summary>
    /// 轴报警，1：有报警，0：无报警
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisAlarm;

    /// <summary>
    /// 跟随误差报警，1：有报警，0：无报警
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int followingErrorAlarm;

    /// <summary>
    /// 运动指令完成，1：完成，0：未完成
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int motionComplete;

    /// <summary>
    /// 正在加速，1：正在加速，0：未加速
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int accFlag;

    /// <summary>
    /// 轴到位，1：已到位，0：未到位
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int inPos;

    /// <summary>
    /// 正限位开关，1：已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int positiveLS;

    /// <summary>
    /// 负限位开关，1：已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int negativeLS;

    /// <summary>
    /// 正软限位开关，1：已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int positiveSoftLimit;

    /// <summary>
    /// 负软限位开关，1:已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int negativeSoftLimit;

    /// <summary>
    /// 原点开关，1：已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int homeSwitch;

    /// <summary>
    /// 回零完成，1：已触发，0：未触发
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int homeDone;

    /// <summary>
    /// 回零中，1：已触发，0：未触发
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int homing;

    /// <summary>
    /// 力矩限制，1：已触发，0：未触发
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int torqueLimit;

    /// <summary>
    /// 驱动器报警代码
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int ampAlarmCode;

    /// <summary>
    /// 主轴，1：是主轴，0：不是主轴
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int master;

    /// <summary>
    /// 从轴，1：是从轴，0：不是从轴
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int slave;

    /// <summary>
    /// 当前轴的运动模式（位置模式，速度模式，力矩模式，插补，PVT等）
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int moveMode;

    /// <summary>
    /// 龙门跟随误差超差，1：已超差，0：未超差
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int gantryFollowingError;

    /// <summary>
    /// 指令位置
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double posCmd;

    /// <summary>
    /// 实际位置
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double actualPos;

    /// <summary>
    /// 指令速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double velocityCmd;

    /// <summary>
    /// 实际速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double actualVelocity;

    /// <summary>
    /// 指令力矩
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double torqueCmd;

    /// <summary>
    /// 实际力矩
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double actualTorque;

    /// <summary>
    /// 实际跟随误差
    /// 支持控制器：E2-M300
    /// </summary>
    double actualFollowingError;

    /// <summary>
    /// 实际同步误差
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double syncOffset;
}NM_AXISSTATUS;

/// <summary>
/// 轴参数
/// </summary>
typedef struct NM_AxisProfile
{
public:
    /// <summary>
    /// 最大速度
    /// 支持控制器：E2-M300
    /// </summary>
    double maxVel;

    /// <summary>
    /// 最大加速度
    /// 支持控制器：E2-M300
    /// </summary>
    double maxAcc;
}NM_AXISPROFILE;

/// <summary>
/// 回零参数
/// </summary>
typedef struct NM_HomeParam
{
public:
    /// <summary>
    /// 回零结束后是否运动到结束位置，1：运动，0：不运动
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int isGotoFinishPos;

    /// <summary>
    /// Double Pass
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int doublePass;

    /// <summary>
    /// 回零方向
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int homeDirection;

    /// <summary>
    /// 是否是从轴，1：是从轴，0：不是从轴
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int slaveAxis;

    /// <summary>
    /// 回零类型
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int homeType;

    /// <summary>
    /// 回零完成后当前位置是否清零，1：清零，0：不清零
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int keepPosition;

    /// <summary>
    /// 回零慢速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double homingVelocitySlow;

    /// <summary>
    /// 回零快速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double homingVelocityFast;

    /// <summary>
    /// 回零加速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double homingAcc;

    /// <summary>
    /// 回零减速度
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    double homingDec;

    /// <summary>
    /// 原点位置
    /// 支持控制器：R2-M100，R2-M300，G2-M100
    /// </summary>
    double homePosition;

    /// <summary>
    /// 回零完成后的移动距离
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double finishPosition;

    /// <summary>
    /// 找探针的最大移动距离
    /// 支持控制器：E2-M300
    /// </summary>
    double maxProbeMove;

    /// <summary>
    /// 找限位/原点开关的最大移动距离
    /// 支持控制器：E2-M300
    /// </summary>
    double maxSwitchMove;
}NM_HOMEPARAM;

/// <summary>
/// 位置模式运动指令
/// </summary>
typedef struct NM_PositionCommand
{
public:
    /// <summary>
    /// 轴号，从0开始
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int axisIndex;

    /// <summary>
    /// 绝对模式下表示目标位置，相对模式下表示相对运动距离
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double target;

    /// <summary>
    /// 运动速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double velocity;

    /// <summary>
    /// 运动加速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double acc;

    /// <summary>
    /// 运动减速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double dec;

    /// <summary>
    /// 加/减速平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double smoothTime;

    /// <summary>
    /// 四阶平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double snapTime;

    /// <summary>
    /// 五阶平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double crackleTime;
}NM_POSITIONCOMMAND;

/// <summary>
/// Jog模式运动指令
/// </summary>
typedef struct NM_JogCommand
{
public:
    /// <summary>
    /// 轴号，从0开始
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int axisIndex;

    /// <summary>
    /// 运动速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double velocity;

    /// <summary>
    /// 运动加速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double acc;

    /// <summary>
    /// 运动减速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double dec;

    /// <summary>
    /// 加/减速平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double smoothTime;
}NM_JOGCOMMAND;

/// <summary>
/// 力矩模式运动指令
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_TorqueCommand
{
public:
    /// <summary>
    /// 轴号，从0开始
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisIndex;

    /// <summary>
    /// 小轴轴号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int torqueAxisIndex;

    /// <summary>
    /// 力矩运动模式：0，常规模式；1，大小轴模式
    /// 支持控制器：E2-M300
    /// </summary>
    int mode;

    /// <summary>
    /// 力矩限制值
    /// 支持控制器：E2-M300
    /// </summary>
    int torque;

    /// <summary>
    /// 速度限制值
    /// 支持控制器：E2-M300
    /// </summary>
    double velocityLimit;

    /// <summary>
    /// 力矩轴启动位置
    /// 支持控制器：E2-M300
    /// </summary>
    double torqueModeStartPos;

}NM_TORQUECOMMAND;

/// <summary>
/// 轴跟随参数
/// </summary>
typedef struct NM_FollowingConfig
{
public:
    /// <summary>
    /// 主轴轴号
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int master;

    /// <summary>
    /// 从轴轴号
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int slave;

    /// <summary>
    /// 横梁轴轴号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int beamAxis;

    /// <summary>
    /// 主轴加速度前馈PDO编号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int master_GIndex_AccFeedforward;

    /// <summary>
    /// 主轴速度前馈PDO编号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int master_GIndex_VelFeedforward;

    /// <summary>
    /// 从轴加速度前馈PDO编号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int slave_GIndex_AccFeedforward;

    /// <summary>
    /// 从轴速度前馈PDO编号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int slave_GIndex_VelFeedforward;

    /// <summary>
    /// 主从轴跟随误差限值
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double followingErrorLimit;

    /// <summary>
    /// 横梁轴最大加速度
    /// 支持控制器：E2-M300
    /// </summary>
    double maxBeamAxisAcc;

    /// <summary>
    /// 加速度前馈值范围
    /// </summary>
    double accFeedforwardRange;

    /// <summary>
    /// 速度前馈比例
    /// </summary>
    double velFeedforwardScale;
}NM_FOLLOWINGCONFIG;

/// <summary>
/// 前瞻插补通道参数
/// </summary>
typedef struct NM_PathIntplLookaheadConfiguration
{
public:
    /// <summary>
    /// 插补组轴数
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    int axisCount;

    /// <summary>
    /// 用户坐标系号
    /// 支持控制器：E2-M300
    /// </summary>
    int userCoordinate;

    /// <summary>
    /// 世界坐标系模式
    /// </summary>
    int worldCoodinateMode;

    /// <summary>
    /// 插补合成速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double compositeVel;

    /// <summary>
    /// 快速运动速度
    /// 支持控制器：E2-M300
    /// </summary>
    double fastVel;

    /// <summary>
    /// 插补合成加速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double compositeAcc;

    /// <summary>
    /// 插补合成减速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double compositeDec;

    /// <summary>
    /// 插补加减速平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double smoothTime;

    /// <summary>
    /// 插补加减速四阶平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double snapTime;

    /// <summary>
    /// 插补加减速五阶平滑时间
    /// 支持控制器：E2-M300
    /// </summary>
    double crackleTime;

    /// <summary>
    /// 插补路径拐角减速阈值
    /// 支持控制器：E2-M300
    /// </summary>
    double thresholdAngle;

    /// <summary>
    /// 插补组轴列表，最多8轴
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int axisList[8];
}NM_PATHINTPLLOOKAHEADCONFIGURATION;

/// <summary>
/// 前瞻插补指令段
/// </summary>
typedef struct NM_PathIntplLookaheadCommandPoint
{
public:
    /// <summary>
    /// 是否使用段速度设定值，0：不使用，1：使用
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int setSegmentVel;

    /// <summary>
    /// 段类型
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    int segmentType;

    /// <summary>
    /// 段速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double segmentVel;

    /// <summary>
    /// 段结束速度
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double segmentEndVel;

    /// <summary>
    /// 段位置，最多支持8轴
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double position[8];

    /// <summary>
    /// 辅助位置，最多8轴
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    double auxPoint[8];
}NM_PATHINTPLLOOKAHEADCOMMANDPOINT;

/// <summary>
/// 前瞻插补通道状态
/// </summary>
typedef struct NM_PathIntplLookaheadStatus
{
public:
    /// <summary>
    /// 运动状态
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    unsigned int state;

    /// <summary>
    /// 插补组是否启用，1：已启用，0：未启用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int enable;

    /// <summary>
    /// 插补缓存中的段数
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    int bufferCount;

    /// <summary>
    /// 插补当前运行的段号
    /// 支持控制器：E2-M300
    /// </summary>
    int segmentIndex;
}NM_PATHINTPLLOOKAHEADSTATUS;

/// <summary>
/// PVT运动通道参数
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_PVTConfiguration
{
    /// <summary>
    /// PVT组轴数
    /// 支持控制器：E2-M300
    /// </summary>
    int axisCount;

    /// <summary>
    /// PVT组轴列表，最多8轴
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisList[8];

    /// <summary>
    /// 轴最大速度，最多8轴
    /// 支持控制器：E2-M300
    /// </summary>
    double maxVel[8];

    /// <summary>
    /// 轴最大加速度，最多8轴
    /// 支持控制器：E2-M300
    /// </summary>
    double maxAcc[8];

    /// <summary>
    /// 轴最大减速度，最多8轴
    /// 支持控制器：E2-M300
    /// </summary>
    double maxDec[8];

    /// <summary>
    /// 轴加减速平滑时间，最多8轴
    /// 支持控制器：E2-M300
    /// </summary>
    double smoothTime[8];
}NM_PVTCONFIGURATION;

/// <summary>
/// PVT运动段
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_PVTPoint
{
public:
    /// <summary>
    /// 运动时间
    /// 支持控制器：E2-M300
    /// </summary>
    double time;

    /// <summary>
    /// 指定速度，1：采用指定速度，0：不采用指定速度
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int enableVel[8];

    /// <summary>
    /// 目标位置
    /// 支持控制器：E2-M300
    /// </summary>
    double position[8];

    /// <summary>
    /// 指定速度值
    /// 支持控制器：E2-M300
    /// </summary>
    double velocity[8];
}NM_PVTPOINT;

/// <summary>
/// PVT运动通道状态
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_PVTStatus
{
public:
    /// <summary>
    /// 运动状态
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int state;

    /// <summary>
    /// PVT组是否启用，1：已启用，0：未启用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int enable;

    /// <summary>
    /// PVT缓存中的段数
    /// 支持控制器：E2-M300
    /// </summary>
    int bufferCount;

    /// <summary>
    /// PVT当前运行段号
    /// 支持控制器：E2-M300
    /// </summary>
    int segmentIndex;
}NM_PVTSTATUS;

/// <summary>
/// 示波器通道参数
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_OSCItem
{
    /// <summary>
    /// 通道启用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int enable;

    /// <summary>
    /// 轴号，模式为NM_OSCITEMTYPE_AXISINFO时有用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisIndex;

    /// <summary>
    /// 轴2轴号，模式为NM_OSCITEMTYPE_AXISINFO2时有用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisIndex2;

    /// <summary>
    /// U序号，模式为NM_OSCITEMTYPE_PDO时有用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int uIndex;

    /// <summary>
    /// G序号，模式为NM_OSCITEMTYPE_PDO时有用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int gIndex;

    /// <summary>
    /// PDO值是否为双字节，模式为NM_OSCITEMTYPE_PDO时有用
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int gDword;

    /// <summary>
    /// 示波器通道类型，设定值为枚举类型NM_OSCItemType强制转换为int
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int oscItemType;

    /// <summary>
    /// 轴信息类型，模式为NM_OSCITEMTYPE_AXISINFO时有用，设定值为枚举类型NM_OSCAxisInfo强制转换为int
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisInfoType;

    /// <summary>
    /// 轴信息类型，模式为NM_OSCITEMTYPE_AXISINFO2时有用，设定值为枚举类型NM_OSCAxisInfo强制转换为int
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisInfoType2;
}NM_OSCITEM;

/// <summary>
/// 示波器参数
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_OSCConfiguration
{
    /// <summary>
    /// 最大采集点数
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int depth;

    /// <summary>
    /// 采样周期倍率，间隔时间=总线周期时间*采样周期倍率
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int interval;

    /// <summary>
    /// 采样通道0配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item0;

    /// <summary>
    /// 采样通道1配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item1;

    /// <summary>
    /// 采样通道2配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item2;

    /// <summary>
    /// 采样通道3配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item3;

    /// <summary>
    /// 采样通道4配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item4;

    /// <summary>
    /// 采样通道5配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item5;

    /// <summary>
    /// 采样通道6配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item6;

    /// <summary>
    /// 采样通道7配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item7;

    /// <summary>
    /// 采样通道8配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item8;

    /// <summary>
    /// 采样通道9配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item9;

    /// <summary>
    /// 采样通道10配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item10;

    /// <summary>
    /// 采样通道11配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item11;

    /// <summary>
    /// 采样通道12配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item12;

    /// <summary>
    /// 采样通道13配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item13;

    /// <summary>
    /// 采样通道14配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item14;

    /// <summary>
    /// 采样通道15配置
    /// 支持控制器：E2-M300
    /// </summary>
    NM_OSCITEM item15;

}NM_OSCCONFIGURATION;

/// <summary>
/// 总线状态
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_Businfo
{
    /// <summary>
    /// 总线状态，0：初始化中，1：完全运行中
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int running;

    /// <summary>
    /// 系统最小负载（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int min_payload;

    /// <summary>
    /// 系统最大负载（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int max_payload;

    /// <summary>
    /// 系统当前负载（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int cur_payload;

    /// <summary>
    /// 最小DC偏移（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int min_shift;

    /// <summary>
    /// 最大DC偏移（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int max_shift;

    /// <summary>
    /// 当前DC偏移（us）
    /// 支持控制器：E2-M300
    /// </summary>
    int cur_shift;

    /// <summary>
    /// 配置的从站数量，包含未启用站点和移除站点
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int config_num;

    /// <summary>
    /// 配置中未移除的从站数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int enable_num;

    /// <summary>
    /// 实际检测到的从站数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int active_num;

    /// <summary>
    /// 总线初始化流程状态
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int ecat_reset_phase;

    /// <summary>
    /// 调试1
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int debug1;

    /// <summary>
    /// 授权错误状态
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int dog_status;

    /// <summary>
    /// 调试2
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int debug2;

    /// <summary>
    /// 调试3
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int debug3;

    /// <summary>
    /// 帧丢失计数
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int lost_frames;

    /// <summary>
    /// WKC帧丢失计数
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int lost_wkc;

    /// <summary>
    /// 同步周期，单位us
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int dc_cycle;

    /// <summary>
    /// 掉线的从站数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int offline_num;
}NM_BUSINFO;

/// <summary>
/// 从站信息
/// </summary>
typedef struct NM_Slave
{
public:
    /// <summary>
    /// 从站ID
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    int slaveID;

    /// <summary>
    /// 从站类型
    /// 支持控制器：E2-M300，R2-M100，R2-M300，G2-M100
    /// </summary>
    int slaveType;
}NM_SLAVE;

/// <summary>
/// 轴补偿参数
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_CompensationConfig
{
    /// <summary>
    /// 补偿原点
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int originIndex;

    /// <summary>
    /// 补偿数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int count;

    /// <summary>
    /// 原点坐标位置
    /// 支持控制器：E2-M300
    /// </summary>
    double originPosition;

    /// <summary>
    /// 补偿点间隔距离
    /// 支持控制器：E2-M300
    /// </summary>
    double pitchInterval;

    /// <summary>
    /// 调整速度
    /// 支持控制器：E2-M300
    /// </summary>
    double catchUpVel;

    /// <summary>
    /// 调整加速度
    /// 支持控制器：E2-M300
    /// </summary>
    double catchUpAcc;
}NM_COMPENSATIONCONFIG;

/// <summary>
/// 轴补偿参数
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_Compensation2DConfig
{
    /// <summary>
    /// 轴0轴号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisIndex0;

    /// <summary>
    /// 轴1轴号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int axisIndex1;

    /// <summary>
    /// 轴0补偿点数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int count0;

    /// <summary>
    /// 轴1补偿点数量
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int count1;

    /// <summary>
    /// 轴0补偿原点序号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int originIndex0;

    /// <summary>
    /// 轴1补偿原点序号
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int originIndex1;

    /// <summary>
    /// 调整速度
    /// 支持控制器：E2-M300
    /// </summary>
    double catchUpVel;

    /// <summary>
    /// 调整加速度
    /// 支持控制器：E2-M300
    /// </summary>
    double catchUpAcc;

    /// <summary>
    /// 轴0原点坐标位置
    /// 支持控制器：E2-M300
    /// </summary>
    double originPosition0;

    /// <summary>
    /// 轴1原点坐标位置
    /// 支持控制器：E2-M300
    /// </summary>
    double originPosition1;

    /// <summary>
    /// 轴0补偿点间隔
    /// 支持控制器：E2-M300
    /// </summary>
    double pitchInterval0;

    /// <summary>
    /// 轴1补偿点间隔
    /// 支持控制器：E2-M300
    /// </summary>
    double pitchInterval1;
}NM_COMPENSATION2DCONFIG;

/// <summary>
/// 结构体-轴输入信号配置
/// 支持控制器：E2-M300
/// </summary>
typedef struct NM_AxisInputSignalConfig
{
    /// <summary>
    /// 信号源，0表示默认信号源，信号从0x60FD当中获取；1表示从IO从模块终获取
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int source;

    /// <summary>
    /// IO模块主站号，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int master;

    /// <summary>
    /// PDO信号UIndex，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int uIndex;

    /// <summary>
    /// PDO信号GIndex，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int gIndex;

    /// <summary>
    /// 正限位信号位号，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int posLimitBit;

    /// <summary>
    /// 负限位信号位号，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int negLimitBit;

    /// <summary>
    /// 原点信号位号，source = 1时有效
    /// 支持控制器：E2-M300
    /// </summary>
    unsigned int homeBit;
}NM_AXISINPUTSIGNALCONFIG;

typedef struct NM_LaserConfig
{
    unsigned int uIndex;
    unsigned int gIndex;
    int minDutyCycle;
    int maxDutyCycle;
}NM_LASERCONFIG;

typedef struct NM_CuttingConfig
{
    double circleCompensation;
}NM_CUTTINGCONFIG;

/// <summary>
/// 设置控制器类型，在Open控制器之前使用
/// </summary>
/// <param name="conType">控制器类型，可用类型参见上方宏定义NM_CONTROLLERTYPE_XXX</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetControllerType(int conType);

/// <summary>
/// 设置控制器IP地址，在Open控制器之前使用
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="ip">IP地址字符串</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetControllerIP(int controller, char ip[]);

/// <summary>
/// 获取运动控制器版本
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="version">版本信息结构，包含控制器版本，运动库版本，核心库版本和dll版本</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetVersion(int controller, NM_VERSION* version);

/// <summary>
/// 打开控制器
/// </summary>
/// <param name="controller">控制器号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Open(int controller);

/// <summary>
/// 关闭控制器
/// </summary>
/// <param name="controller">控制器号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Close(int controller);

/// <summary>
/// 伺服上/下电
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="onOff">上下电，1：上电，0：下电</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_ServoOnOff(int controller, unsigned int axisIndex, int onOff);

/// <summary>
/// 获取轴状态
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="axisStatus">轴状态</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetAxisStatus(int controller, unsigned int axisIndex, NM_AXISSTATUS* axisStatus);

/// <summary>
/// 设置轴当前位置为0，轴不移动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetZeroPos(int controller, unsigned int axisIndex);

/// <summary>
/// 设置轴当前位置为指定位置，轴不移动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="position"></param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetPosition(int controller, unsigned int axisIndex, double position);

/// <summary>
/// 清除轴警报
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_ClearAlarm(int controller, unsigned int axisIndex);

/// <summary>
/// 获取轴单位脉冲数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="encoderScale">脉冲数，单位pulse/mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetEncoderScale(int controller, unsigned int axisIndex, double* encoderScale);

/// <summary>
/// 设置轴单位脉冲数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="encoderScale">脉冲数，单位pulse/mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetEncoderScale(int controller, unsigned int axisIndex, double encoderScale);

/// <summary>
/// 设置软限位
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="enable">是否启用，1：启用，0：禁用</param>
/// <param name="positiveLimit">正限位值</param>
/// <param name="negativeLimit">负限位值</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetSoftLimit(int controller, unsigned int axisIndex, int enable, double positiveLimit, double negativeLimit);

/// <summary>
/// 获取软限位
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="enable">是否启用，1：启用，0：禁用</param>
/// <param name="positiveLimit">正限位值</param>
/// <param name="negativeLimit">负限位值</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetSoftLimit(int controller, unsigned int axisIndex, int* enable, double* positiveLimit, double* negativeLimit);

/// <summary>
/// 设置总线轴模式
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="axisMode">轴模式，可用参数参见上方宏定义NM_E2M300_AXISMODE_XXX</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetAxisMode(int controller, unsigned int axisIndex, int axisMode);

/// <summary>
/// 获取总线轴模式
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="axisMode">轴模式，对应参数参见上方宏定义NM_E2M300_AXISMODE_XXX</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetAxisMode(int controller, unsigned int axisIndex, int* axisMode);

/// <summary>
/// 轴回零
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="hParam">回零参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AxisHome(int controller, unsigned int axisIndex, NM_HomeParam hParam);

/// <summary>
/// 轴快速停止（急停）
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AxisQuickStop(int controller, unsigned int axisIndex);

/// <summary>
/// 轴停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AxisStop(int controller, unsigned int axisIndex);

/// <summary>
/// 位置模式运动（绝对）
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="posCommand">位置运动指令</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_PositionMode_Abs(int controller, NM_PositionCommand posCommand);

/// <summary>
/// 位置模式运动（相对）
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="posCommand">位置运动指令</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_PositionMode_Rel(int controller, NM_PositionCommand posCommand);

/// <summary>
/// Jog模式运动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="jogCommand">Jog运动指令</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_JogMode(int controller, NM_JogCommand jogCommand);

/// <summary>
/// 力矩模式运动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="tqCommand">力矩运动指令</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_TorqueMode(int controller, NM_TorqueCommand tqCommand);

/// <summary>
/// 设置主从轴同步
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="folConfig">主从轴同步参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetSyncMasterSlave(int controller, NM_FOLLOWINGCONFIG folConfig);

/// <summary>
/// 启用/禁用龙门解耦功能1，横梁轴运动解耦
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="master">主轴轴号，从0开始</param>
/// <param name="slave">从轴轴号，从0开始</param>
/// <param name="enable">是否启用，1：启用，0：禁用</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetGantryCoupling1(int controller, unsigned int master, unsigned int slave, int enable);

/// <summary>
/// 启用/禁用龙门解耦功能2，主从轴运动解耦
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="master">主轴轴号，从0开始</param>
/// <param name="slave">从轴轴号，从0开始</param>
/// <param name="enable">是否启用，1：启用，0：禁用</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetGantryCoupling2(int controller, unsigned int master, unsigned int slave, int enable);

/// <summary>
/// 解散同步，只需设置从轴
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="slave">从轴轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_ResolveSync(int controller, unsigned int slave);

/// <summary>
/// 前瞻插补通道参数设置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="pathConfig">通道参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetPathIntplLookaheadConfiguration(int controller, int channel, NM_PATHINTPLLOOKAHEADCONFIGURATION pathConfig);

/// <summary>
/// 前瞻插补通道解散
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_DismissPathIntplLookahead(int controller, int channel);

/// <summary>
/// 前瞻插补运动启动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_PathIntplLookahead(int controller, int channel);

/// <summary>
/// 添加前瞻插补数据
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="pointCount">数据点数</param>
/// <param name="pathPoints">前瞻插补数据</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AddPathIntplLookahead(int controller, int channel, int pointCount, NM_PATHINTPLLOOKAHEADCOMMANDPOINT pathPoints[]);

/// <summary>
/// 前瞻插补运动停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Stop_PathIntpl(int controller, int channel);

/// <summary>
/// 获取前瞻插补运动通道状态
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="intplStatus">通道状态</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetPathIntplLookaheadStatus(int controller, int channel, NM_PATHINTPLLOOKAHEADSTATUS* intplStatus);

/// <summary>
/// 清除前瞻插补通道缓存数据
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_PathIntplLookaheadClearCount(int controller, int channel);

/// <summary>
/// 设置PVT通道参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="pvtConfig">PVT通道参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetPVTConfiguration(int controller, int channel, NM_PVTCONFIGURATION pvtConfig);

/// <summary>
/// 解散PVT通道
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_DismissPVTGroup(int controller, int channel);

/// <summary>
/// PVT运动启动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Motion_PVT(int controller, int channel);

/// <summary>
/// PVT运动停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Stop_PVT(int controller, int channel);

/// <summary>
/// 
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="pvtStatus">PVT通道状态</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetPVTStatus(int controller, int channel, NM_PVTSTATUS* pvtStatus);

/// <summary>
/// 清除PVT通道缓存数据
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_PVTClearCount(int controller, int channel);

/// <summary>
/// 添加PVT数据
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="pointCount">数据点数</param>
/// <param name="pvtPoints">PVT数据</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AddPVTPoint(int controller, int channel, int pointCount, NM_PVTPoint pvtPoints[]);

/// <summary>
/// EtherCAT总线读取PDO
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="uIndex">从站号，从0开始</param>
/// <param name="gIndex">G编号</param>
/// <param name="ptr">数据</param>
/// <param name="len">数据长度，可用值1：16位，2：32位</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EtherCATReadPDO(int controller, int masterIndex, unsigned int uIndex, unsigned int gIndex, unsigned short* ptr, unsigned int len);

/// <summary>
/// EtherCAT总线写入PDO
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="uIndex">从站号，从0开始</param>
/// <param name="gIndex">G编号</param>
/// <param name="ptr">数据</param>
/// <param name="len">数据长度，可用值1：16位，2：32位</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EtherCATWritePDO(int controller, int masterIndex, unsigned int uIndex, unsigned int gIndex, unsigned short* ptr, unsigned int len);

/// <summary>
/// EtherCAT读取SDO
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="uIndex">从站号，从0开始</param>
/// <param name="mainIndex">主索引编号</param>
/// <param name="subIndex">子索引编号</param>
/// <param name="byteNum">字节数量</param>
/// <param name="val">SDO值</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EtherCATReadSDO(int controller, int masterIndex, unsigned int uIndex, unsigned int mainIndex, unsigned int subIndex, unsigned int byteNum, unsigned int* val);

/// <summary>
/// EtherCAT写入SDO
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="uIndex">从站号，从0开始</param>
/// <param name="mainIndex">主索引编号</param>
/// <param name="subIndex">子索引编号</param>
/// <param name="byteNum">字节数量</param>
/// <param name="val">SDO值</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EtherCATWriteSDO(int controller, int masterIndex, unsigned int uIndex, unsigned int mainIndex, unsigned int subIndex, unsigned int byteNum, unsigned int val);

/// <summary>
/// 设置示波器参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="oscConfig">示波器参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_OSC_SetConfig(int controller, NM_OSCCONFIGURATION oscConfig);

/// <summary>
/// 获取示波器参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="oscConfig">示波器参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_OSC_GetConfig(int controller, NM_OSCCONFIGURATION* oscConfig);

/// <summary>
/// 示波器启动
/// </summary>
/// <param name="controller">控制器号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_OSC_Start(int controller);

/// <summary>
/// 示波器停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_OSC_Stop(int controller);

/// <summary>
/// 获取示波器数据
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="buffer">缓存数据</param>
/// <param name="bufferSize"></param>
/// <param name="retSize"></param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_OSC_GetData(int controller, double* buffer, unsigned int bufferSize, unsigned int* retSize);

/// <summary>
/// 获取总线状态
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="busInfo">总线状态</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetBusInfo(int controller, int masterIndex, NM_BUSINFO* busInfo);

/// <summary>
/// 获取从站信息
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="slave">从站列表</param>
/// <param name="slaveCount">从站数量</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetSlaveInfo(int controller, NM_SLAVE* slave, int* slaveCount);

/// <summary>
/// 控制器总线重置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="mode">重置模式，参见上方宏定义NM_BUSRESET_XXX</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_ControllerBusReset(int controller, int masterIndex, int mode);

/// <summary>
/// 设置轴补偿参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axis">轴号，从0开始</param>
/// <param name="config">补偿参数</param>
/// <param name="compensationData">补偿数据列表</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetCompensation(int controller, unsigned int axis, NM_COMPENSATIONCONFIG config, double compensationData[]);

/// <summary>
/// 获取轴补偿参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axis">轴号，从0开始</param>
/// <param name="config">补偿参数</param>
/// <param name="compensationData">补偿数据列表</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetCompensation(int controller, unsigned int axis, NM_COMPENSATIONCONFIG* config, double* compensationData);

/// <summary>
/// 启用/禁用轴补偿
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axis">轴号，从0开始</param>
/// <param name="enable">启用/禁用，1：启用，0：禁用</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EnableCompensation(int controller, unsigned int axis, int enable);

/// <summary>
/// 设置二维补偿参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="nmConfig">二维补偿参数</param>
/// <param name="compensationData0">轴0补偿数据列表</param>
/// <param name="compensationData1">轴1补偿数据列表</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetCompensation2D(int controller, NM_Compensation2DConfig nmConfig, double compensationData[]);

/// <summary>
/// 启用/禁用二维补偿
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axis0">补偿轴0轴号，从0开始</param>
/// <param name="axis1">补偿轴1轴号，从0开始</param>
/// <param name="enable">启用/禁用，1：启用，0：禁用</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_EnableCompensation2D(int controller, unsigned int axis0, unsigned int axis1, int enable);

/// <summary>
/// 获取插补通道轴位置，同周期
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="position">位置数据</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetPosition_PathIntplLookahead(int controller, int channel, double position[]);

/// <summary>
/// 获取PVT通道轴位置，同周期
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="channel">通道号，从0开始</param>
/// <param name="position">位置数据</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetPosition_PVT(int controller, int channel, double position[]);

/// <summary>
/// 获取运动库速度类型
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="type">速度类型，0：S曲线，1：T曲线</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetVelocityType(int controller, int* type);

/// <summary>
/// 设置运动库速度类型
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="type">速度类型，0：S曲线，1：T曲线</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetVelocityType(int controller, int type);

/// <summary>
/// 获取主站状态
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="status">主站状态，0：停止，1：运行，2：过渡</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetMasterStatus(int controller, int masterIndex, unsigned int* status);

/// <summary>
/// 将Windows程序载入实时系统
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="app">Windows程序路径名，路径中不可以包含中文</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_LoadERTApp(int controller, char* app);

/// <summary>
/// 设置触发模块编码器脉冲比
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="scale">脉冲比，单位pulse/mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_SetEncoderScale(int controller, int index_Encoder, double scale);

/// <summary>
/// 获取触发模块编码器脉冲比
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="scale">脉冲比，单位pulse/mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_GetEncoderScale(int controller, int index_Encoder, double* scale);

/// <summary>
/// 设置触发模块编码器位置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="value">编码器位置，单位mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_SetEncoderValue(int controller, int masterIndex, unsigned int slave, int index_Encoder, double value);

/// <summary>
/// 获取触发模块编码器位置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="value">编码器位置，单位mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_GetEncoderValue(int controller, int masterIndex, unsigned int slave, int index_Encoder, double* value);

/// <summary>
/// 获取触发模块触发计数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_TriggerOut">触发通道序号，范围0-3</param>
/// <param name="count">触发计数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_GetTriggerCount(int controller, int masterIndex, unsigned int slave, int index_TriggerOut, int* count);

/// <summary>
/// 设置触发通道参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_TriggerOut">触发通道序号，范围0-3</param>
/// <param name="pulseWidth">脉宽</param>
/// <param name="compare">比较器序号，0-7为等间距比较器，8-11为点表比较器</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_SetTriggerOutParam(int controller, int masterIndex, unsigned int slave, int index_TriggerOut, int pulseWidth, int compare);

/// <summary>
/// 手动触发
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_TriggerOut">触发通道序号，范围0-3</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_ManualTrigger(int controller, int masterIndex, unsigned int slave, int index_TriggerOut);

/// <summary>
/// 设置等间距触发参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_LineCompare">线性比较器序号，范围0-7</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="startPos">起始位置，单位mm</param>
/// <param name="endPos">终点位置，单位mm</param>
/// <param name="interval">触发间隔，单位mm</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_SetLineCompareParam(int controller, int masterIndex, unsigned int slave, int index_LineCompare, int index_Encoder, double startPos, double endPos, double interval);

/// <summary>
/// 开启/关闭等间距触发
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_LineCompare">线性比较器序号，范围0-7</param>
/// <param name="enable">开启1/关闭0</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_EnableLineCompare(int controller, int masterIndex, unsigned int slave, int index_LineCompare, int enable);

/// <summary>
/// 设置点表比较器参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_PreCompare">点表比较器序号，范围0-3</param>
/// <param name="index_Encoder">编码器序号，范围0-3</param>
/// <param name="position">位置列表，单位mm</param>
/// <param name="posCount">位置列表长度</param>
/// <param name="dir">方向：0：正，1：负</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_SetPreCompareParam(int controller, int masterIndex, unsigned int slave, int index_PreCompare, int index_Encoder, double* position, int posCount, int dir);

/// <summary>
/// 开启/关闭点表比较器
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_PreCompare">点表比较器序号，范围0-3</param>
/// <param name="enable">开启1/关闭0</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_EnablePreCompare(int controller, int masterIndex, unsigned int slave, int index_PreCompare, int enable);

/// <summary>
/// 清除触发计数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="masterIndex">主站号，从0开始</param>
/// <param name="slave">模块从站号，从0开始</param>
/// <param name="index_TriggerOut">触发通道序号，范围0-3</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Trigger_ClearCounter(int controller, int masterIndex, unsigned int slave, int index_TriggerOut);

/// <summary>
/// 定制功能-设置参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="functionCode">功能号</param>
/// <param name="parameterLength">参数长度</param>
/// <param name="parameters">参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Customize_Function_SetParameter(int controller, int functionCode, int parameterLength, double* parameters);

/// <summary>
/// 定制功能-启动
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="functionCode">功能号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Customize_Function_Start(int controller, int functionCode);

/// <summary>
/// 定制功能-停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="functionCode">功能号</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Customize_Function_Stop(int controller, int functionCode);

/// <summary>
/// 定制功能
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="functionCode">功能号</param>
/// <param name="status">状态</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_Customize_Function_GetStatus(int controller, int functionCode, int* status);

/// <summary>
/// 获取控制器自动控制模式
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="mode">模式，0：非自动模式，1：自动模式</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SetLibAutoControlMode(int controller, int mode);

/// <summary>
/// 设置控制器自动控制模式
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="mode">模式，0：非自动模式，1：自动模式</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_GetLibAutoControlMode(int controller, int* mode);

/// <summary>
/// 获取轴输入信号配置参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="config">轴输入信号配置参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SingleAxis_Get_InputSignalConfig(int controller, unsigned int axisIndex, NM_AxisInputSignalConfig* config);

/// <summary>
/// 设置轴输入信号配置参数
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="config">轴输入信号配置参数</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SingleAxis_Set_InputSignalConfig(int controller, unsigned int axisIndex, NM_AxisInputSignalConfig config);

/// <summary>
/// 获取单轴指定捕获位置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="mode">位置捕获模式</param>
/// <param name="pos">捕获位置</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SingleAxis_Get_LatchPosition(int controller, unsigned int axisIndex, int mode, double* pos);

/// <summary>
/// 清除单轴指定捕获位置
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <param name="mode">位置捕获模式</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_SingleAxis_Clear_LatchPosition(int controller, unsigned int axisIndex, int mode);

/// <summary>
/// 轴回零停止
/// </summary>
/// <param name="controller">控制器号</param>
/// <param name="axisIndex">轴号，从0开始</param>
/// <returns></returns>
NEOMOVEAPIRETURN NM_AxisHomeStop(int controller, unsigned int axisIndex);

NEOMOVEAPIRETURN NM_Laser_SetConfig(int controller, int channel, NM_LaserConfig config);
NEOMOVEAPIRETURN NM_Laser_GetConfig(int controller, int channel, NM_LaserConfig* config);
NEOMOVEAPIRETURN NM_Laser_Enable(int controller, int channel, int enable);
NEOMOVEAPIRETURN NM_Cutting_SetConfig(int controller, int channel, NM_CuttingConfig config);
NEOMOVEAPIRETURN NM_Cutting_GetConfig(int controller, int channel, NM_CuttingConfig* config);
NEOMOVEAPIRETURN NM_Cutting_Enable(int controller, int channel, int enable);