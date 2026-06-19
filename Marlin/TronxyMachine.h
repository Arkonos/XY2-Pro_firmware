#pragma once
#define PROJ_XY2_PRO       11 //255-255-240

#define UI_MARLIN_DEFAULT  0
#define UI_PIC480X320      1

#define TRONXY_PROJ  PROJ_XY2_PRO
#define TRONXY_UI 0 // UI_PIC480X320 or UI_MARLIN_DEFAULT


#define HAS_TRONXY_PROJ    1

#define TRONXY_MACHINE_NAME       "XY2-PRO"
#define TRONXY_BACKUP_POWER       0 //UPS functionality: power-failure detection requires specific input pins; configure the appropriate interrupt service routine (ISR) accordingly.
#define TRONXY_SCREEN_SHUTDOWN    1 //Display sleep function
#define TRONXY_HOME_XY_ONLY       0 //In the manual debug UI, if reset is XY-only, activate the Z reset button; otherwise, keep it disabled.
#define DUAL_ZMAX_HOME            1 //Home at the far end, reverse direction to clear the switch, then move forward again to locate the zero point.
#define EXTRUDERS                 0 //Number of extruders
#define USE_EXTRUDER              1 //T,g TITAN:1,BMG:2,TAOLI:3,general:4
// #define FIX_MOUNTED_PROBE