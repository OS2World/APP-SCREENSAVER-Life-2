#ifndef LIFE_INCLUDED
#define LIFE_INCLUDED

#define LFM_BASE                 WM_USER+256
#define LFM_INPUT                (LFM_BASE)

typedef struct _HOOKDATA {
   USHORT usSzStruct;
   HWND hwndLife;
   HMODULE hmModule;
   BOOL bSaving;
   USHORT usMouseKludge;
} HOOKDATA, *PHOOKDATA;

typedef PHOOKDATA *PPHOOKDATA;

BOOL EXPENTRY LifeInit(HWND hwndApp,PPHOOKDATA pphdData);
BOOL EXPENTRY LifeTerm(VOID);

#endif                           // #ifndef LIFE_INCLUDED
