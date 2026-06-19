#pragma once

#include "src/inc/MarlinConfig.h"

#define	POS_BOOTINFO        0x0800C000 //扇区:C000-10000:16KB
#define	POS_FIRMWARENAME    0x0800C008
#define POS_VERSION         0x0800C200
#define POS_INTERRUPT       0x0800C500
#define BOOT_INFO_SIZE      1060
#define INTERRUPT_SIGN      0xA5A5

