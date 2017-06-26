VOID querySpaceRect(HWND hwndWnd,USHORT usX,USHORT usY,PRECTL prclSpace);
USHORT queryNumNeighbors(PSAVERDATA psdData,USHORT usX,USHORT usY);
VOID initBoard(PSAVERDATA psdData);
VOID initRoamer(PSAVERDATA psdData);
MRESULT EXPENTRY saverWndProc(HWND hwndWnd,
                              ULONG ulMsg,
                              MPARAM mpParm1,
                              MPARAM mpParm2);
MRESULT EXPENTRY settingsDlgProc(HWND hwndWnd,
                                 ULONG ulMsg,
                                 MPARAM mpParm1,
                                 MPARAM mpParm2);
INT main(VOID);
