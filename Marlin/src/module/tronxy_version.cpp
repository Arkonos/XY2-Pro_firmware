/**
 * Tronxy project EEPROM version.
 *
 * The Tronxy UI library used to provide this symbol. Marlin UI builds still
 * use it for settings compatibility when HAS_TRONXY_PROJ is enabled.
 */

#include "../inc/MarlinConfigPre.h"

#if ENABLED(HAS_TRONXY_PROJ) && !TRONXY_UI

extern const uint8_t tronxy_version[4] = { 2, 18, 1, TRONXY_PROJ };

#endif
