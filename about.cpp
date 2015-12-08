#define STRICT
#include <windows.h>
#include "dxw.h"
#include "ver.h"

//ARGSUSED
DLGPROC(AboutDlg)
{
switch(msg){
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_DATE,__DATE__);
		sprintf(buf,"Version %d.%d",VER_MAJOR,VER_MINOR);
		SetDlgItemText(hDlg,IDC_VER,buf);
		return 1;
	case WM_COMMAND:
		if(wParam==IDOK || wParam==IDCANCEL ){
				EndDialog(hDlg,0);
				return 1;
		}
		break;
}
return 0;
}
