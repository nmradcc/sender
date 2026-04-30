#ifndef FATTIME_H_
#define FATTIME_H_

#include "integer.h"

extern DWORD get_fattime (void);

extern void fat_to_time(WORD fattime, char* szTime);
extern void fat_to_date(WORD fatdate, char* szDate);

#endif
