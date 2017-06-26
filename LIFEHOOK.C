#define INCL_DOSMODULEMGR
#define INCL_WINHOOKS
#define INCL_WININPUT
#define INCL_WINMESSAGEMGR
#include <os2.h>
#include "life.h"

#define DLL_NAME                 "LIFEHOOK"

HOOKDATA hdData;

BOOL EXPENTRY LifeHook(HAB habAnchor,PQMSG pqmMsg,ULONG ulFlags)
//-------------------------------------------------------------------------
// This is the input hook that is used to detect activity (the lack of
// this is, thus, inactivity).  Whenever we see a key press or mouse message,
// notify the application.
//
// Since we can be called in the context of *any* application running, we
// cannot simply store a pointer to the instance data of the saver window,
// unless we use shared memory.  However, we can declare a variable in the
// address space of the DLL and (assuming the DLL attributes are DATA SHARED
// SINGLE) store a pointer to it in the saver window instance data (in
// LifeInit() ).
//
// Input:  habAnchor - handle of the anchor block of the application to
//                     receive this message
//         pqmMsg - pointer to the QMSG structure to be passed to the
//                  application via WinGetMsg/WinPeekMsg
//         ulFlags - specifies whether or not the message will be removed
//                   from the queue.
// Returns:  TRUE if the rest of the hook chain is not to be called, or FALSE
//           if the rest of the hook chain should be called.
//-------------------------------------------------------------------------
{
   switch (pqmMsg->msg) {
   case WM_CHAR:
      if (hdData.hwndLife!=NULLHANDLE) {
         WinSendMsg(hdData.hwndLife,LFM_INPUT,0,0);
      } /* endif */
      break;
   case WM_MOUSEMOVE:
   case WM_BUTTON1DOWN:
   case WM_BUTTON2DOWN:
   case WM_BUTTON3DOWN:
   case WM_BUTTON1UP:
   case WM_BUTTON2UP:
   case WM_BUTTON3UP:
   case WM_BUTTON1DBLCLK:
   case WM_BUTTON2DBLCLK:
   case WM_BUTTON3DBLCLK:
      //-------------------------------------------------------------------
      // See the notes in WM_TIMER/TID_SECONDS section of LIFE.C
      //-------------------------------------------------------------------
      if (hdData.bSaving && (hdData.usMouseKludge>0)) {
         hdData.usMouseKludge--;
      } else
      if (hdData.hwndLife!=NULLHANDLE) {
         WinSendMsg(hdData.hwndLife,LFM_INPUT,0,0);
      } /* endif */
      break;
   default:
      break;
   } /* endswitch */

   return FALSE;
}

BOOL EXPENTRY LifeInit(HWND hwndApp,PPHOOKDATA pphdData)
//-------------------------------------------------------------------------
// This function initializes the DLL and sets the input hook.
//
// Input:  hwndApp - handle of the Life Saver application settings window
//         pphdData - pointer to the pointer to the hook data
// Returns:  TRUE if successful, FALSE otherwise
//-------------------------------------------------------------------------
{
   hdData.usSzStruct=sizeof(HOOKDATA);
   hdData.hwndLife=hwndApp;

   if (DosQueryModuleHandle(DLL_NAME,&hdData.hmModule)) {
      return FALSE;
   } /* endif */

   hdData.bSaving=FALSE;
   hdData.usMouseKludge=0;

   WinSetHook(WinQueryAnchorBlock(hdData.hwndLife),
              NULLHANDLE,
              HK_INPUT,
              (PFN)LifeHook,
              hdData.hmModule);

   *pphdData=&hdData;
   return TRUE;
}

BOOL EXPENTRY LifeTerm(VOID)
//-------------------------------------------------------------------------
// This function releases the input hook and broadcasts a WM_NULL message
// to all top level windows so that they will release the DLL.  If we don't
// do this, the DLL will remain locked and we'll have to reboot in order to
// recompile.
//
// Returns:  TRUE always
//-------------------------------------------------------------------------
{
   WinReleaseHook(WinQueryAnchorBlock(hdData.hwndLife),
                  NULLHANDLE,
                  HK_INPUT,
                  (PFN)LifeHook,
                  hdData.hmModule);
   WinBroadcastMsg(HWND_DESKTOP,WM_NULL,0,0,BMSG_FRAMEONLY|BMSG_POST);
   return TRUE;
}
