// Settings.h

#ifndef SETTINGS_H_
#define SETTINGS_H_

//#include "stm32f10x.h"
#include <stddef.h>

#define SETTINGS_FILE	"CONFIG.INI"


extern void SetPersistDirty(void);
extern void ClrPersistDirty(void);
extern int GetPersistDirty(void);

extern int GetSettings(void);
extern void SaveSettings(void);

extern void SettingsTask(void *argument);

#endif

