//-------------------------------------------------------------------------
// The Game of Life - A Screen Saver
//
// Written by Larry Salomon, Jr. with the intent of disseminating yet more
// PM source for the purpose of teaching those who are still struggling
// and need a bit of guidance.
//
// The link step of the build process requires the library CMN32.LIB,
// which can be found in the common.zip package also on cdrom.com
// (in the /pub/os2/all/program directory, I believe).  For more inform-
// ation on the common.zip package, see the Programming Reference (.INF
// format) contained in that ZIP file.
//
// Any questions, comments, or concerns can be sent via email to me.  My
// address is os2man@panix.com
//-------------------------------------------------------------------------
#define INCL_DOSMISC
#define INCL_GPIBITMAPS
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#define INCL_WINBUTTONS
#define INCL_WINDIALOGS
#define INCL_WINENTRYFIELDS
#define INCL_WINHELP
#define INCL_WININPUT
#define INCL_WINPALETTE
#define INCL_WINPOINTERS
#define INCL_WINSHELLDATA
#define INCL_WINSTDSLIDER
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINWINDOWMGR
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define INCL_CMNLIB
#include <common.h>
#include "rc.h"
#include "help.h"
#include "life.h"

//-------------------------------------------------------------------------
// Beginning of 16-bit declarations
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// We need access to the 16-bit API DosGetInfoSeg() so that we can determine
// the id of the current session.  Though the documentation for
// DosQuerySysInfo() says that you can specify the QSV_FOREGROUND_FS_SESSION
// parameter to get this, that constant is not defined in the 2.0 toolkit
// and hard-coding its value - 24 - returns an error.
//
// Why do we need to know the session id?  See the notes below.  :)
//-------------------------------------------------------------------------
typedef struct _GINFOSEG {
    ULONG   time;               /* time in seconds                           */
    ULONG   msecs;              /* milliseconds                              */
    UCHAR   hour;               /* hours                                     */
    UCHAR   minutes;            /* minutes                                   */
    UCHAR   seconds;            /* seconds                                   */
    UCHAR   hundredths;         /* hundredths                                */
    USHORT  timezone;           /* minutes from UTC                          */
    USHORT  cusecTimerInterval; /* timer interval (units = 0.0001 seconds)   */
    UCHAR   day;                /* day                                       */
    UCHAR   month;              /* month                                     */
    USHORT  year;               /* year                                      */
    UCHAR   weekday;            /* day of week                               */
    UCHAR   uchMajorVersion;    /* major version number                      */
    UCHAR   uchMinorVersion;    /* minor version number                      */
    UCHAR   chRevisionLetter;   /* revision letter                           */
    UCHAR   sgCurrent;          /* current foreground session                */
    UCHAR   sgMax;              /* maximum number of sessions                */
    UCHAR   cHugeShift;         /* shift count for huge elements             */
    UCHAR   fProtectModeOnly;   /* protect mode only indicator               */
    USHORT  pidForeground;      /* pid of last process in forground session  */
    UCHAR   fDynamicSched;      /* dynamic variation flag                    */
    UCHAR   csecMaxWait;        /* max wait in seconds                       */
    USHORT  cmsecMinSlice;      /* minimum timeslice (milliseconds)          */
    USHORT  cmsecMaxSlice;      /* maximum timeslice (milliseconds)          */
    USHORT  bootdrive;          /* drive from which the system was booted    */
    UCHAR   amecRAS[32];        /* system trace major code flag bits         */
    UCHAR   csgWindowableVioMax;/* maximum number of VIO windowable sessions */
    UCHAR   csgPMMax;           /* maximum number of pres. services sessions */
} GINFOSEG;
typedef GINFOSEG FAR *PGINFOSEG;

USHORT APIENTRY16 DOS16GETINFOSEG(PSEL _Seg16 pselGlobal,PSEL _Seg16 pselLocal);
//-------------------------------------------------------------------------
// End of 16-bit declarations
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// CLS_SAVER - class of the screen saver window.  The Settings window is a
//             dialog, so it has no class.  :)
// PRF_APPNAME - application name for the Prf*() functions.
// PRF_SETTINGS - key name for the settings info.
// COORD(pcdData,usX,usY) - converts a 2-d coordinate in a 1-d array index.
// TID_LIFECLICKS - timer id for the screen saver.
// TID_SECONDS - timer id for the inactivity timer.
// TM_TIMEOUT - timeout value for TID_LIFECLICKS.
// MAX_COLORS - number of colors in aulColors[].
// MAX_MARGIN - defines the boundaries of the centers of the patterns that
//              are generated.  Used to avoid boundary checks when gen'ing
//              the universe.
// MAX_TICKS - modulus factor when calculating the number of ticks for the
//             current universe.
// BASE_TICKS - base number of ticks for every universe.  A random number
//              from 0-(MAX_TICKS-1) is added to this to get the lifetime of
//              each universe.
//-------------------------------------------------------------------------
#define CLS_SAVER                "LifeSaver"

#define PRF_APPNAME              "Life Saver"
#define PRF_SETTINGS             "Settings"

#define COORD(pcdData,usX,usY) (((usY)*pcdData->szlBoard.cx)+(usX))

#define TID_LIFECLICKS           1
#define TID_SECONDS              2

#define TM_TIMEOUT               750

#define MAX_COLORS               16
#define MAX_MARGIN               3

#define MAX_TICKS                25
#define BASE_TICKS               25

//-------------------------------------------------------------------------
// The colors to be used.  The age of a square (in ticks) is mod'd with
// the maximum number of colors to get an index into the palette, which
// is based on this array.
//-------------------------------------------------------------------------
ULONG aulColors[MAX_COLORS]={
   0x00FFFF00L,
   0x00CCFF33L,
   0x0099FF66L,
   0x0066FF99L,
   0x0033FFCCL,
   0x0000FFFFL,
   0x0033CCFFL,
   0x006699FFL,
   0x009966FFL,
   0x00CC33FFL,
   0x00FF00FFL,
   0x00FF33CCL,
   0x00FF6699L,
   0x00FF9966L,
   0x00FFCC33L,
   0x00000000L
};

//-------------------------------------------------------------------------
// Settings information
//-------------------------------------------------------------------------
typedef struct _SETTINGS {
   USHORT usSzStruct;
   ULONG ulSaveTime;
   BOOL bDisable;
} SETTINGS, *PSETTINGS;

//-------------------------------------------------------------------------
// Instance data for the saver window
//-------------------------------------------------------------------------
typedef struct _SAVERDATA {
   USHORT usSzStruct;
   HAB habAnchor;
   HWND hwndOwner;

   SETTINGS sSettings;
   ULONG ulSeconds;
   HWND hwndFocus;
   PHOOKDATA phdHook;

   HPS hpsPaint;
   HPAL hpPalette;
   HBITMAP hbmSpace;
   SIZEL szlSpace;
   SIZEL szlBoard;
   PSHORT psBoard;
   PSHORT psNew;
   ULONG ulMaxTicks;
   ULONG ulRoamer;
   ULONG ulNumTicks;
} SAVERDATA, *PSAVERDATA;

//-------------------------------------------------------------------------
// Instance data for the client window
//-------------------------------------------------------------------------
typedef struct _CLIENTDATA {
   USHORT usSzStruct;
   HAB habAnchor;
   HWND hwndSaver;
   HWND hwndHelp;
   PSAVERDATA psdData;
} CLIENTDATA, *PCLIENTDATA;

#include "proto.h"

VOID querySpaceRect(HWND hwndWnd,USHORT usX,USHORT usY,PRECTL prclSpace)
//-------------------------------------------------------------------------
// This function returns the rectangle for the coordinates (usX,usY).
//
// Input:  hwndWnd - specifies the saver window
//         usX, usY - specifies the coordinates
// Output:  prclSpace - points to the rectangle for the space
//-------------------------------------------------------------------------
{
   PSAVERDATA psdData;
   RECTL rclWnd;

   psdData=WinQueryWindowPtr(hwndWnd,0);
   WinQueryWindowRect(hwndWnd,&rclWnd);
   prclSpace->xLeft=rclWnd.xLeft+usX*psdData->szlSpace.cx;
   prclSpace->yBottom=rclWnd.yTop-(usY+1)*psdData->szlSpace.cy;
   prclSpace->xRight=prclSpace->xLeft+psdData->szlSpace.cx;
   prclSpace->yTop=prclSpace->yBottom+psdData->szlSpace.cy;
   return;                       // Avoid the "implicit return" warning.  Ugh!
}

USHORT queryNumNeighbors(PSAVERDATA psdData,USHORT usX,USHORT usY)
//-------------------------------------------------------------------------
// This function returns the number of neighbors of the space (usX,usY)
//
// Input:  psdData - points to the saver window instance data.  We pass this
//                   instead of the pointer to the board buffer because we
//                   also need the size of the board.  A better solution
//                   would be to typedef a BOARD datatype which includes
//                   the size...Nah...Too easy.  :)
//         usX, usY - coordinates of the space to check
// Returns:  number of neighbors
//
// In the notes below, 'X' is the space to be checked and the neighbors
// are number thusly:
//
// 1 2 3
// 4 X 6
// 7 8 9
//-------------------------------------------------------------------------
{
   USHORT usNum;

   usNum=0;

   if (usX>0) {
      //-------------------------------------------------------------------
      // Check neighbors 1, 4, and 7
      //-------------------------------------------------------------------
      if (psdData->psBoard[COORD(psdData,usX-1,usY)]!=-1) {
         usNum++;
      } /* endif */

      if ((usY>0) && (psdData->psBoard[COORD(psdData,usX-1,usY-1)]!=-1)) {
         usNum++;
      } /* endif */

      if ((usY<psdData->szlBoard.cy-1) &&
          (psdData->psBoard[COORD(psdData,usX-1,usY+1)]!=-1)) {
         usNum++;
      } /* endif */
   } /* endif */

   if (usX<psdData->szlBoard.cx-1) {
      //-------------------------------------------------------------------
      // Check neighbors 3, 6, and 9
      //-------------------------------------------------------------------
      if (psdData->psBoard[COORD(psdData,usX+1,usY)]!=-1) {
         usNum++;
      } /* endif */

      if ((usY>0) && (psdData->psBoard[COORD(psdData,usX+1,usY-1)]!=-1)) {
         usNum++;
      } /* endif */

      if ((usY<psdData->szlBoard.cy-1) &&
          (psdData->psBoard[COORD(psdData,usX+1,usY+1)]!=-1)) {
         usNum++;
      } /* endif */
   } /* endif */

   //----------------------------------------------------------------------
   // Check neighbors 2 and 8
   //----------------------------------------------------------------------
   if ((usY>0) && (psdData->psBoard[COORD(psdData,usX,usY-1)]!=-1)) {
      usNum++;
   } /* endif */

   if ((usY<psdData->szlBoard.cy-1) &&
       (psdData->psBoard[COORD(psdData,usX,usY+1)]!=-1)) {
      usNum++;
   } /* endif */

   return usNum;
}

VOID initBoard(PSAVERDATA psdData)
//-------------------------------------------------------------------------
// This function reinitializes the universe.
//
// Input:  psdData - points to the saver window instance data
//-------------------------------------------------------------------------
{
   USHORT usX;
   USHORT usY;
   ULONG ulNumPtrns;
   ULONG ulIndex;

   psdData->ulMaxTicks=rand()%MAX_TICKS+BASE_TICKS;
   psdData->ulRoamer=psdData->ulMaxTicks/4;
   psdData->ulNumTicks=0;

   for (usX=0; usX<psdData->szlBoard.cx; usX++) {
      for (usY=0; usY<psdData->szlBoard.cy; usY++) {
         psdData->psBoard[COORD(psdData,usX,usY)]=-1;
         psdData->psNew[COORD(psdData,usX,usY)]=-1;
      } /* endfor */
   } /* endfor */

   //----------------------------------------------------------------------
   // Generate a random number (1-3) of patterns.  In the patters below,
   // 'x' indicates a filled space, 'X' indicates the filled center of the
   // pattern, and '.' indicates the empty center of the pattern
   //----------------------------------------------------------------------
   ulNumPtrns=rand()%3+1;

   for (ulIndex=0; ulIndex<ulNumPtrns; ulIndex++) {
      usX=(USHORT)(rand()%(psdData->szlBoard.cx-MAX_MARGIN*4))+MAX_MARGIN*2;
      usY=(USHORT)(rand()%(psdData->szlBoard.cy-MAX_MARGIN*4))+MAX_MARGIN*2;

      switch (rand()%8) {
      case 0:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x
         //   x X x
         //     x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+1)]=0;
         break;
      case 1:
         //----------------------------------------------------------------
         // Pattern:
         //
         //   x     x
         //   x .   x
         //   x     x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY+1)]=0;
         break;
      case 2:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x x
         //       x
         //   x X x
         //   x
         //   x x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-2)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-2)]=0;
         break;
      case 3:
         //----------------------------------------------------------------
         // Pattern:
         //
         //   x
         //     x
         // x x . x x
         //     x
         //       x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY-2)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY+2)]=0;
         break;
      case 4:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x x
         //   x X   x
         //   x     x
         //     x x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;

         psdData->psBoard[COORD(psdData,usX,usY)]=0;
         break;
      case 5:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x x
         //   x . x x
         //   x     x
         //     x x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;

         psdData->psBoard[COORD(psdData,usX+1,usY)]=0;
         break;
      case 6:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x x
         //   x .   x
         //   x   x x
         //     x x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;

         psdData->psBoard[COORD(psdData,usX+1,usY+1)]=0;
         break;
      case 7:
         //----------------------------------------------------------------
         // Pattern:
         //
         //     x x
         //   x .   x
         //   x x   x
         //     x x
         //----------------------------------------------------------------
         psdData->psBoard[COORD(psdData,usX-1,usY)]=0;
         psdData->psBoard[COORD(psdData,usX-1,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY+2)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY+1)]=0;
         psdData->psBoard[COORD(psdData,usX+2,usY)]=0;
         psdData->psBoard[COORD(psdData,usX+1,usY-1)]=0;
         psdData->psBoard[COORD(psdData,usX,usY-1)]=0;

         psdData->psBoard[COORD(psdData,usX,usY+1)]=0;
         break;
      default:
         break;
      } /* endswitch */
   } /* endfor */

   return;
}

VOID initRoamer(PSAVERDATA psdData)
//-------------------------------------------------------------------------
// This function creates a roamer.
//
// Input:  psdData - points to the saver window instance data
//-------------------------------------------------------------------------
{
   USHORT usDelta;

   usDelta=rand()%5;

   psdData->psBoard[COORD(psdData,3+usDelta,2)]=0;
   psdData->psBoard[COORD(psdData,4+usDelta,3)]=0;
   psdData->psBoard[COORD(psdData,4+usDelta,4)]=0;
   psdData->psBoard[COORD(psdData,3+usDelta,4)]=0;
   psdData->psBoard[COORD(psdData,2+usDelta,4)]=0;
   return;
}

MRESULT EXPENTRY saverWndProc(HWND hwndWnd,
                              ULONG ulMsg,
                              MPARAM mpParm1,
                              MPARAM mpParm2)
//-------------------------------------------------------------------------
// Saver window procedure
//-------------------------------------------------------------------------
{
   PSAVERDATA psdData;

   psdData=WinQueryWindowPtr(hwndWnd,0);

   switch (ulMsg) {
   case WM_CREATE:
      {
         SIZEL szlPaint;
         BITMAPINFOHEADER bmihInfo;
         ULONG ulChanged;
         ULONG ulSzBuf;

         //----------------------------------------------------------------
         // Allocate and initialize the instance data
         //----------------------------------------------------------------
         psdData=calloc(1,sizeof(SAVERDATA));
         if (psdData==NULL) {
            return MRFROMSHORT(TRUE);
         } /* endif */

         WinSetWindowPtr(hwndWnd,0,psdData);

         psdData->usSzStruct=sizeof(SAVERDATA);
         psdData->habAnchor=WinQueryAnchorBlock(hwndWnd);
         psdData->hwndOwner=WinQueryWindow(hwndWnd,QW_OWNER);

         szlPaint.cx=0;
         szlPaint.cy=0;

         psdData->hpsPaint=GpiCreatePS(psdData->habAnchor,
                                       WinOpenWindowDC(hwndWnd),
                                       &szlPaint,
                                       PU_PELS|GPIT_NORMAL|GPIA_ASSOC);
         if (psdData->hpsPaint==NULLHANDLE) {
            free(psdData);
            return MRFROMSHORT(TRUE);
         } /* endif */

         psdData->hpPalette=GpiCreatePalette(psdData->habAnchor,
                                             0,
                                             LCOLF_CONSECRGB,
                                             MAX_COLORS,
                                             aulColors);
         if (psdData->hpPalette==NULLHANDLE) {
            GpiDestroyPS(psdData->hpsPaint);
            free(psdData);
            return MRFROMSHORT(TRUE);
         } /* endif */

         GpiSelectPalette(psdData->hpsPaint,psdData->hpPalette);
         WinRealizePalette(hwndWnd,psdData->hpsPaint,&ulChanged);

         psdData->hbmSpace=GpiLoadBitmap(psdData->hpsPaint,
                                         NULLHANDLE,
                                         BMP_SPACE,
                                         0,
                                         0);

         GpiQueryBitmapParameters(psdData->hbmSpace,&bmihInfo);
         psdData->szlSpace.cx=bmihInfo.cx;
         psdData->szlSpace.cy=bmihInfo.cy;

         psdData->szlBoard.cx=WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN)/
                                 psdData->szlSpace.cx;
         psdData->szlBoard.cy=WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN)/
                                 psdData->szlSpace.cy;

         psdData->psBoard=calloc(psdData->szlBoard.cx*psdData->szlBoard.cy,
                                 sizeof(SHORT));
         if (psdData->psBoard==NULL) {
            GpiDeleteBitmap(psdData->hbmSpace);
            GpiDeletePalette(psdData->hpPalette);
            GpiDestroyPS(psdData->hpsPaint);
            free(psdData);
            return MRFROMSHORT(TRUE);
         } /* endif */

         psdData->psNew=calloc(psdData->szlBoard.cx*psdData->szlBoard.cy,
                               sizeof(SHORT));
         if (psdData->psNew==NULL) {
            GpiDeleteBitmap(psdData->hbmSpace);
            GpiDeletePalette(psdData->hpPalette);
            GpiDestroyPS(psdData->hpsPaint);
            free(psdData->psBoard);
            free(psdData);
            return MRFROMSHORT(TRUE);
         } /* endif */

         //----------------------------------------------------------------
         // If we can't find the settings info in the .INI file, initialize
         // them to the defaults
         //----------------------------------------------------------------
         ulSzBuf=sizeof(SETTINGS);

         if (!PrfQueryProfileData(HINI_USERPROFILE,
                                  PRF_APPNAME,
                                  PRF_SETTINGS,
                                  &psdData->sSettings,
                                  &ulSzBuf) ||
             (psdData->sSettings.usSzStruct!=sizeof(SETTINGS))) {
            psdData->sSettings.usSzStruct=sizeof(SETTINGS);
            psdData->sSettings.ulSaveTime=60;
            psdData->sSettings.bDisable=FALSE;
         } /* endif */

         psdData->ulSeconds=0;

         WinStartTimer(psdData->habAnchor,
                       hwndWnd,
                       TID_SECONDS,
                       1000);

         //----------------------------------------------------------------
         // Tell the DLL to install itself
         //----------------------------------------------------------------
         LifeInit(hwndWnd,&psdData->phdHook);
      }
      break;
   case WM_DESTROY:
      LifeTerm();
      WinStopTimer(psdData->habAnchor,hwndWnd,TID_SECONDS);
      WinStopTimer(psdData->habAnchor,hwndWnd,TID_LIFECLICKS);
      GpiDeletePalette(psdData->hpPalette);
      GpiDeleteBitmap(psdData->hbmSpace);
      GpiDestroyPS(psdData->hpsPaint);
      free(psdData->psBoard);
      free(psdData->psNew);
      free(psdData);
      break;
   case WM_SIZE:
      psdData->szlBoard.cx=SHORT1FROMMP(mpParm2)/psdData->szlSpace.cx;
      psdData->szlBoard.cy=SHORT2FROMMP(mpParm2)/psdData->szlSpace.cy;
      break;
   case WM_TIMER:
      switch (SHORT1FROMMP(mpParm1)) {
      case TID_LIFECLICKS:
         {
            USHORT usX;
            USHORT usY;
            USHORT usNum;

            psdData->ulNumTicks++;

            //-------------------------------------------------------------
            // If we've exceeded the lifespan of this universe, create a
            // new one, else if it's time for the roamer to appear, create
            // it.
            //-------------------------------------------------------------
            if (psdData->ulNumTicks==psdData->ulMaxTicks) {
               initBoard(psdData);
            } else
            if (psdData->ulNumTicks==psdData->ulRoamer) {
               initRoamer(psdData);
            } /* endif */

            //-------------------------------------------------------------
            // For every space in the universe, check the number of neighbors
            // and update it accordingly.
            //-------------------------------------------------------------
            for (usX=0; usX<psdData->szlBoard.cx; usX++) {
               for (usY=0; usY<psdData->szlBoard.cy; usY++) {
                  usNum=queryNumNeighbors(psdData,usX,usY);

                  switch (usNum) {
                  case 0:
                  case 1:
                  case 4:
                  case 5:
                  case 6:
                  case 7:
                  case 8:
                     psdData->psNew[COORD(psdData,usX,usY)]=-1;
                     break;
                  case 2:
                     if (psdData->psBoard[COORD(psdData,usX,usY)]!=-1) {
                        psdData->psNew[COORD(psdData,usX,usY)]=
                           psdData->psBoard[COORD(psdData,usX,usY)]+1;
                     } else {
                        psdData->psNew[COORD(psdData,usX,usY)]=-1;
                     } /* endif */
                     break;
                  case 3:
                     psdData->psNew[COORD(psdData,usX,usY)]=
                        psdData->psBoard[COORD(psdData,usX,usY)]+1;
                     break;
                  default:
                     break;
                  } /* endswitch */
               } /* endfor */
            } /* endfor */

            memcpy(psdData->psBoard,
                   psdData->psNew,
                   psdData->szlBoard.cx*psdData->szlBoard.cy*sizeof(SHORT));

            WinInvalidateRect(hwndWnd,NULL,FALSE);
            WinUpdateWindow(hwndWnd);
         }
         break;
      case TID_SECONDS:
         {
            USHORT usSelGlobal;
            USHORT usSelLocal;
            PGINFOSEG _Seg16 pgisGlobal;
            ULONG ulCx;
            ULONG ulCy;

            //-------------------------------------------------------------
            // Talk about some hocus-pocus to call a 16-bit routine!!!
            // Get the current session id and if it is not PM (id==1),
            // don't monitor the inactivity.
            //-------------------------------------------------------------
            DOS16GETINFOSEG((PSEL _Seg16)&usSelGlobal,(PSEL _Seg16)&usSelLocal);
            pgisGlobal=(PGINFOSEG _Seg16)MAKEULONG(0,usSelGlobal);

            if ((pgisGlobal->sgCurrent==1) && (!psdData->sSettings.bDisable)) {
               psdData->ulSeconds++;
            } else {
               psdData->ulSeconds=0;
            } /* endif */

            //-------------------------------------------------------------
            // If you've been inactive for too long, check your heart. :)
            // Seriously, if the number of seconds of inactivity has
            // elapsed, start the saver.
            //-------------------------------------------------------------
            if (psdData->ulSeconds>=psdData->sSettings.ulSaveTime) {
               initBoard(psdData);

               WinShowPointer(HWND_DESKTOP,FALSE);

               ulCx=WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
               ulCy=WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);

               //----------------------------------------------------------
               // What a kludge!  If we don't set the focus to ourselves,
               // certain applications might paint over us (i.e. the
               // system clock).
               //----------------------------------------------------------
               psdData->hwndFocus=WinQueryFocus(HWND_DESKTOP);
               WinSetFocus(HWND_DESKTOP,psdData->hwndOwner);

               WinSetWindowPos(hwndWnd,
                               NULLHANDLE,
                               0,
                               0,
                               ulCx,
                               ulCy,
                               SWP_MOVE|SWP_SIZE|SWP_SHOW);

               //----------------------------------------------------------
               // Stop the inactivity timer and start the universe timer
               //----------------------------------------------------------
               WinStopTimer(psdData->habAnchor,
                            hwndWnd,
                            TID_SECONDS);

               WinStartTimer(psdData->habAnchor,
                             hwndWnd,
                             TID_LIFECLICKS,
                             TM_TIMEOUT);

               //----------------------------------------------------------
               // The usMouseKludge field is used because if you are a
               // hidden window and then you show yourself, if the mouse
               // is over you when you appear you get a pair of WM_MOUSEMOVE
               // messages.  We don't want the saver to appear and then
               // disappear again, so set this to the number of WM_MOUSEMOVEs
               // to ignore so the hook doesn't notify us.
               //----------------------------------------------------------
               psdData->phdHook->usMouseKludge=2;
               psdData->phdHook->bSaving=TRUE;
            } /* endif */
         }
         break;
      default:
         return WinDefWindowProc(hwndWnd,ulMsg,mpParm1,mpParm2);
      } /* endswitch */
      break;
   case WM_PAINT:
      {
         RECTL rclPaint;
         RECTL rclWnd;
         USHORT usX;
         USHORT usY;
         RECTL rclSpace;

         WinBeginPaint(hwndWnd,psdData->hpsPaint,&rclPaint);

         //----------------------------------------------------------------
         // Paint the sections of the window that are not covered by a
         // space black (aulColors[MAX_COLORS-1]==0x00000000, RGB).
         //----------------------------------------------------------------
         WinQueryWindowRect(hwndWnd,&rclWnd);
         rclWnd.xLeft=psdData->szlBoard.cx*psdData->szlSpace.cx;
         WinFillRect(psdData->hpsPaint,&rclWnd,MAX_COLORS-1);

         WinQueryWindowRect(hwndWnd,&rclWnd);
         rclWnd.yTop-=psdData->szlBoard.cy*psdData->szlSpace.cy;
         WinFillRect(psdData->hpsPaint,&rclWnd,MAX_COLORS-1);

         rclPaint.xLeft=rclPaint.xLeft/psdData->szlSpace.cx;
         rclPaint.xRight=rclPaint.xRight/psdData->szlSpace.cx+1;
         rclPaint.yBottom=(rclWnd.yTop-rclPaint.yBottom)/psdData->szlSpace.cy;
         rclPaint.yTop=(rclWnd.yTop-rclPaint.yTop)/psdData->szlSpace.cy+1;

         for (usX=0; usX<psdData->szlBoard.cx; usX++) {
            for (usY=0; usY<psdData->szlBoard.cy; usY++) {
               querySpaceRect(hwndWnd,usX,usY,&rclSpace);

               if ((psdData->psBoard[COORD(psdData,usX,usY)])==-1) {
                  WinFillRect(psdData->hpsPaint,&rclSpace,MAX_COLORS-1);
               } else {
                  WinDrawBitmap(psdData->hpsPaint,
                                psdData->hbmSpace,
                                NULL,
                                (PPOINTL)&rclSpace.xLeft,
                                psdData->psBoard[COORD(psdData,usX,usY)]%(MAX_COLORS-1),
                                MAX_COLORS-1,
                                DBM_NORMAL);
               } /* endif */
            } /* endfor */
         } /* endfor */

         WinEndPaint(psdData->hpsPaint);
      }
      break;
   case LFM_INPUT:
      if (psdData->phdHook->bSaving) {
         //----------------------------------------------------------------
         // If we're in saver mode, stop it.
         //----------------------------------------------------------------
         WinStopTimer(psdData->habAnchor,
                      hwndWnd,
                      TID_LIFECLICKS);
         psdData->phdHook->bSaving=FALSE;

         WinSetFocus(HWND_DESKTOP,psdData->hwndFocus);

         WinSetWindowPos(hwndWnd,
                         NULLHANDLE,
                         0,
                         0,
                         0,
                         0,
                         SWP_HIDE);

         WinShowPointer(HWND_DESKTOP,TRUE);
      } /* endif */

      //-------------------------------------------------------------------
      // Reset the inactivity count and restart the timer.
      //-------------------------------------------------------------------
      psdData->ulSeconds=0;
      WinStartTimer(psdData->habAnchor,
                    hwndWnd,
                    TID_SECONDS,
                    1000);
      break;
   default:
      return WinDefWindowProc(hwndWnd,ulMsg,mpParm1,mpParm2);
   } /* endswitch */

   return MRFROMSHORT(FALSE);
}

MRESULT EXPENTRY settingsDlgProc(HWND hwndWnd,
                                 ULONG ulMsg,
                                 MPARAM mpParm1,
                                 MPARAM mpParm2)
{
   PCLIENTDATA pcdData;

   pcdData=WinQueryWindowPtr(hwndWnd,0);

   switch (ulMsg) {
   case WM_INITDLG:
      {
         HELPINIT hiInit;
         CHAR achBuf[256];

         srand(time(NULL));

         //----------------------------------------------------------------
         // Allocate and initialize the instance data
         //----------------------------------------------------------------
         pcdData=calloc(1,sizeof(CLIENTDATA));
         if (pcdData==NULL) {
            WinAlarm(HWND_DESKTOP,WA_ERROR);
            WinDismissDlg(hwndWnd,FALSE);
            return MRFROMSHORT(FALSE);
         } /* endif */

         WinSetWindowPtr(hwndWnd,0,pcdData);

         pcdData->usSzStruct=sizeof(CLIENTDATA);
         pcdData->habAnchor=WinQueryAnchorBlock(hwndWnd);

         pcdData->hwndSaver=WinCreateWindow(HWND_DESKTOP,
                                            CLS_SAVER,
                                            "",
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            hwndWnd,
                                            HWND_TOP,
                                            WND_SAVER,
                                            NULL,
                                            NULL);
         if (pcdData->hwndSaver==NULLHANDLE) {
            free(pcdData);
            WinAlarm(HWND_DESKTOP,WA_ERROR);
            WinDismissDlg(hwndWnd,FALSE);
            return MRFROMSHORT(FALSE);
         } /* endif */

         pcdData->psdData=WinQueryWindowPtr(pcdData->hwndSaver,0);

         //----------------------------------------------------------------
         // Create the help instance
         //----------------------------------------------------------------
         hiInit.cb=sizeof(HELPINIT);
         hiInit.ulReturnCode=0L;
         hiInit.pszTutorialName=NULL;
         hiInit.phtHelpTable=(PHELPTABLE)MAKEULONG(HELP_SETTINGS,0xFFFF);
         hiInit.hmodHelpTableModule=NULLHANDLE;
         hiInit.hmodAccelActionBarModule=NULLHANDLE;
         hiInit.idAccelTable=0;
         hiInit.idActionBar=0;

         WinLoadString(pcdData->habAnchor,
                       NULLHANDLE,
                       STR_HELPTITLE,
                       sizeof(achBuf),
                       achBuf);
         hiInit.pszHelpWindowTitle=achBuf;

         hiInit.fShowPanelId=CMIC_HIDE_PANEL_ID;
         hiInit.pszHelpLibraryName="LIFE.HLP";

         pcdData->hwndHelp=WinCreateHelpInstance(pcdData->habAnchor,&hiInit);
         if (pcdData->hwndHelp!=NULLHANDLE) {
            if (hiInit.ulReturnCode==0) {
               WinAssociateHelpInstance(pcdData->hwndHelp,hwndWnd);
            } else {
               WinDestroyHelpInstance(pcdData->hwndHelp);
               pcdData->hwndHelp=NULLHANDLE;
            } /* endif */
         } /* endif */

         if (pcdData->hwndHelp==NULLHANDLE) {
            WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_HELP),FALSE);
         } /* endif */

         WinSendDlgItemMsg(hwndWnd,
                           DST_SL_TIMEOUT,
                           SLM_SETTICKSIZE,
                           MPFROM2SHORT(SMA_SETALLTICKS,5),
                           0);

         WinSendMsg(hwndWnd,WM_COMMAND,MPFROMSHORT(DST_PB_UNDO),0);

         //----------------------------------------------------------------
         // Since the initial position of any slider is 0, if the current
         // setting also evaluates to position 0, the slider will not
         // change the value and thus we will not get a WM_CONTROL message.
         // So, send the message anyways so that the entryfield will get
         // updated.  Also, since no changes have been made, disable the
         // Undo button.
         //----------------------------------------------------------------
         WinSendMsg(hwndWnd,
                    WM_CONTROL,
                    MPFROM2SHORT(DST_SL_TIMEOUT,SLN_CHANGE),
                    0);
         WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_UNDO),FALSE);

         CmnWinCenterWindow(hwndWnd);
      }
      break;
   case WM_DESTROY:
      WinDestroyWindow(pcdData->hwndSaver);
      WinDestroyHelpInstance(pcdData->hwndHelp);
      free(pcdData);
      break;
   case WM_CONTROL:
      switch (SHORT1FROMMP(mpParm1)) {
      case DST_CB_DISABLE:
         switch (SHORT2FROMMP(mpParm1)) {
         case BN_CLICKED:
         case BN_DBLCLICKED:
            WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_UNDO),TRUE);
            break;
         default:
            return WinDefDlgProc(hwndWnd,ulMsg,mpParm1,mpParm2);
         } /* endswitch */
         break;
      case DST_SL_TIMEOUT:
         switch (SHORT2FROMMP(mpParm1)) {
         case SLN_CHANGE:
         case SLN_SLIDERTRACK:
            {
               ULONG ulPos;
               CHAR achText[32];

               //----------------------------------------------------------
               // The user has changed the slider position, so update the
               // entryfield to reflect the current value
               //----------------------------------------------------------
               ulPos=LONGFROMMR(WinSendDlgItemMsg(hwndWnd,
                                                  DST_SL_TIMEOUT,
                                                  SLM_QUERYSLIDERINFO,
                                                  MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                               SMA_INCREMENTVALUE),
                                                  0));
               ulPos=ulPos*15+30;

               sprintf(achText,"%ld:%02ld",ulPos/60,ulPos%60);
               WinSetDlgItemText(hwndWnd,DST_EF_TIMEOUT,achText);

               WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_UNDO),TRUE);
            }
            break;
         default:
            return WinDefDlgProc(hwndWnd,ulMsg,mpParm1,mpParm2);
         } /* endswitch */
         break;
      default:
         return WinDefDlgProc(hwndWnd,ulMsg,mpParm1,mpParm2);
      } /* endswitch */
      break;
   case WM_COMMAND:
      switch (SHORT1FROMMP(mpParm1)) {
      case DST_PB_SET:
      case DID_OK:
      case DST_PB_APPLY:
         {
            ULONG ulPos;

            //-------------------------------------------------------------
            // Query the values and update the saver instance data.  If
            // "Set" was specified, write to the .INI file.
            //-------------------------------------------------------------
            ulPos=LONGFROMMR(WinSendDlgItemMsg(hwndWnd,
                                               DST_SL_TIMEOUT,
                                               SLM_QUERYSLIDERINFO,
                                               MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                            SMA_INCREMENTVALUE),
                                               0));
            ulPos=ulPos*15+30;

            pcdData->psdData->sSettings.ulSaveTime=ulPos;
            pcdData->psdData->ulSeconds=0;

            pcdData->psdData->sSettings.bDisable=
               SHORT1FROMMR(WinSendDlgItemMsg(hwndWnd,
                                              DST_CB_DISABLE,
                                              BM_QUERYCHECK,
                                              0,
                                              0));

            if (SHORT1FROMMP(mpParm1)!=DST_PB_APPLY) {
               PrfWriteProfileData(HINI_USERPROFILE,
                                   PRF_APPNAME,
                                   PRF_SETTINGS,
                                   &pcdData->psdData->sSettings,
                                   (ULONG)pcdData->psdData->usSzStruct);
            } /* endif */

            WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_UNDO),FALSE);
         }
         break;
      case DST_PB_UNDO:
         //----------------------------------------------------------------
         // Revert back to the last "Set" or "Apply"
         //----------------------------------------------------------------
         WinSendDlgItemMsg(hwndWnd,
                           DST_SL_TIMEOUT,
                           SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                           MPFROMLONG((pcdData->psdData->sSettings.ulSaveTime-
                                        30)/15));
         WinSendDlgItemMsg(hwndWnd,
                           DST_CB_DISABLE,
                           BM_SETCHECK,
                           MPFROMSHORT(pcdData->psdData->sSettings.bDisable),
                           0);
         WinEnableWindow(WinWindowFromID(hwndWnd,DST_PB_UNDO),FALSE);
         break;
      case DST_PB_HELP:
         WinSendMsg(pcdData->hwndHelp,HM_EXT_HELP,0,0);
         break;
      default:
         break;
      } /* endswitch */
      break;
   default:
      return WinDefDlgProc(hwndWnd,ulMsg,mpParm1,mpParm2);
   } /* endswitch */

   return MRFROMSHORT(FALSE);
}

INT main(VOID)
{
   HAB habAnchor;
   HMQ hmqQueue;

   habAnchor=WinInitialize(0);
   hmqQueue=WinCreateMsgQueue(habAnchor,0);

   //----------------------------------------------------------------------
   // Register the classes of the various types of windows we use
   //----------------------------------------------------------------------
   WinRegisterClass(habAnchor,
                    CLS_SAVER,
                    saverWndProc,
                    CS_SIZEREDRAW,
                    sizeof(PVOID));

   WinDlgBox(HWND_DESKTOP,
             HWND_DESKTOP,
             settingsDlgProc,
             NULLHANDLE,
             DLG_SETTINGS,
             NULL);

   WinDestroyMsgQueue(hmqQueue);
   WinTerminate(habAnchor);
   return 0;
}
