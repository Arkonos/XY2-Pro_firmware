#pragma once
#include "yType.hpp"
#include "TronxyMachine.h"
#define POS_MAIN_EN         0 //01-首页EN.bmp
#define POS_MAIN_CN         1 //02-首页CN.bmp
#define POS_SYSTEM_EN       2 //03-系统EN.bmp
#define POS_SYSTEM_CN       3 //04-系统CN.bmp
#define POS_TOOL_EN         4 //05-工具EN.bmp
#define POS_TOOL_CN         5 //06-工具CN.bmp
#define POS_FILE_MANU       6 //07-文件操作.bmp
#define POS_FILE_CHOOSE     7 //08-文件选择.bmp
#define POS_MANUAL          8 //09-手动.bmp
#define POS_INFO_EN         9 //10-系统信息EN.bmp
#define POS_INFO_CN         10 //11-系统信息CN.bmp
#define POS_PREHEAT         11 //12-预热.bmp
#define POS_FIXMATERIAL     12 //13-装卸耗材.bmp
#define POS_FIXMATERIAL2    13 //14-装卸耗材.bmp
#define POS_FIXMATERIAL3    14 //15-装卸耗材.bmp
#define POS_SYSTEM_STATUS   15 //16-状态.bmp
#define POS_KEYBOARD        16 //17-数字键盘.bmp
#define POS_FAN_EN          20 //21-风扇EN.bmp
#define POS_FAN_CN          21 //22-风扇CN.bmp
#define POS_MANUALLEVEL_EN  22 //23-平台调平EN.bmp
#define POS_MANUALLEVEL_CN  23 //24-平台调平CN.bmp
#define POS_ZOFFSET_EN      26 //27-Z_offset_EN.bmp
#define POS_ZOFFSET_CN      27 //28-Z_offset_CN.bmp
#define POS_AUTOLEVEL_EN    28 //29-自动平台调平EN.bmp
#define POS_AUTOLEVEL_CN    29 //30-自动平台调平CN.bmp
#define POS_PRINTMODE_EN    30 //31-打印模式EN.bmp
#define POS_PRINTMODE_CN    31 //32-打印模式CN.bmp
#define POS_EOFFSET         32 //33-E偏移调整界面

extern const char *tip_language_en;
extern const char *tip_open_fail_en;
extern const char *tip_read_fail_en;
extern const char *tip_insert_sdcard_en;
extern const char *tip_normal_mode_en;
extern const char *tip_modify_save_en;
extern const char *tip_probe_fail_en;
extern const char *tip_calibrate_en;
extern const char *tip_factoryreset_en;
extern const char *tip_coodextruder_en;
extern const char *tip_stopprint_en;
extern const char *tip_completeprint_en;
extern const char *tip_deletefile_en;
extern const char *tip_pausing_en;
extern const char *tip_power_loss_en;
extern const char *tip_waitforheating_en;
extern const char *tip_recoveryprint_en;
extern const char *tip_nosupport_en;
extern const char *tip_system_reset_en;
extern const char *tip_level_en;
extern const char *tip_fixdualz_en;

extern const char *tip_autoLevel1_en;
extern const char *tip_autoLevel2_en;
extern const char *tip_manualLevel_en;
extern const char *tip_restart_cali_en;

#define POS_LANGUAGE          0
#define POS_ICO_LOGO          1
#define TIP_AUTOLEVEL1_CN     0
#define TIP_MANUALLEVEL_CN    8
#define TIP_NORMALMODE_CN     16
#define TIP_ERROR_CN          24
#define TIP_CONFIRM_CN        32
#define TIP_CANCEL_CN         40
#define TIP_RETRY_CN          48
#define TIP_RUNOUT_CN         56
#define TIP_PAUSE_CN          64
#define TIP_HEATING_CN        72
#define TIP_FACTORY_CN        80
#define TIP_COMPLETE_CN       88
#define TIP_CALI_CN           96
#define TIP_STOP_CN           104
#define TIP_RECONVERY_CN      112
#define TIP_DELETE_CN         120
#define TIP_TOOCOLD_CN        128
#define TIP_INSERT_CN         136
#define TIP_NOTSUPPORT_CN     144
#define TIP_PROBENG_CN        152
#define TIP_SAVE_CN           160
#define TIP_LEVELMODE_CN      168
#define TIP_MANUAL_CN         176
#define TIP_MANUAL_JP         177
#define TIP_MANUAL_KR         178
#define TIP_MANUAL_RS         179
#define TIP_MANUAL_FR         180
#define TIP_MANUAL_GR         181
#define TIP_MANUAL_SP         182
#define TIP_MANUAL_PT         183
#define TIP_AUTO_CN           184
#define TIP_AUTO_JP           185
#define TIP_AUTO_KR           186
#define TIP_AUTO_RS           187
#define TIP_AUTO_FR           188
#define TIP_AUTO_GR           189
#define TIP_AUTO_SP           190
#define TIP_AUTO_PT           191
#define TIP_OPENFAIL_CN       192
#define TIP_READFAIL_CN       200
#define TIP_INSD_CN           208
#define TIP_SAVE_SUCCESS_CN   216
#define TIP_LEVEL_OK_CN       224
#define TIP_LEVEL_NG_CN       232
#define TIP_AUTOLEVEL2_CN     240
#define TIP_TRIGGER_CN        248
#define TIP_HOMEXY_CN         256
#define TIP_MOVELEFT_CN       264
#define TIP_MOVERIGHT_CN      272
#define TIP_PROBE_CN          280
#define TIP_COMPENSATE_CN     288
#define TIP_MOVELIMIT_CN      296
#define TIP_FIXNOW_CN         304
#define TIP_POWER_LOSS_CN     312
#define TIP_DUALZ_LIMIT_CN    320

#define LOG_EROM_ADDR        0x00000000

//UI:0x00040000 -- 0x00060000
#define THEME_EROM_ADDR      0x00060000
//THEME:0x00060000 -- 0x000C0000
#define FONTU16_EROM_ADDR    0x000C0000
//FONT:0x000C0000 -- 0x003EC000
#define SDPIC_ADDR           0x3EC000
//SDPIC:0x003EC000 -- 0x0040000

#define MCU_PIC_ADDR(v)     pic_index[v]
#define MCU_TIP_ADDR(v)     (tip_data + tip_index[v])
