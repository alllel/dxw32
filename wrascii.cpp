#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <commdlg.h>

static unsigned Step=1;

void Gauge::WriteASCII()
{
OPENFILENAME opfn;
opfn.lStructSize=sizeof(opfn);
opfn.hwndOwner=hFrame;
opfn.lpstrFilter="Tab separated (*.txt)\0*.txt\0Comma separated (*.csv)\0*.csv\0";
opfn.lpstrCustomFilter=NULL;
opfn.nFilterIndex=1;
opfn.lpstrFile=buf;
opfn.nMaxFile=128;
opfn.lpstrFileTitle=NULL;
opfn.lpstrInitialDir=NULL;
opfn.lpstrTitle="Write Channell as";
opfn.Flags=OFN_NOCHANGEDIR|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
opfn.lpstrDefExt=NULL;

lstrcpy(buf,ChNum);

if(GetSaveFileName(&opfn)){
	const char*format=(opfn.nFilterIndex==1)?"%lg\t%lg\t%lg\r\n":"%lg,%lg,%lg\r\n";
	if(opfn.nFileExtension && !buf[opfn.nFileExtension])lstrcpy(buf+opfn.nFileExtension,opfn.nFilterIndex==1?".TXT":".CSV");
	HFILE hf=_lcreat(buf,0);
	if(hf!=HFILE_ERROR){
		if(DLG(ID_STEP,StepDlg)==1){
			unsigned long i;
			BeginWait();
			LockI();
			for(i=start;i<final;i+=Step){
				sprintf(buf,format,
								P2T(i),
								Val(i),
								Imp(i));
				_lwrite(hf,buf,lstrlen(buf));
			}
			EndWait();
		}
		_lclose(hf);
	}else{
		MessageBox(hFrame,"Can't create file",NULL,MB_OK);
	}
}
}

//ARGSUSED
DLGPROC(StepDlg)
{
switch(msg){
	case WM_INITDIALOG:{
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"1");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"2");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"3");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"4");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"5");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"10");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"15");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"20");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"30");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"40");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"50");
		SendDlgItemMessage(hDlg,IDC_ST_STEP,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"100");
		SetDlgItemInt(hDlg,IDC_ST_STEP,Step,FALSE);
		return 0;
	}
	case WM_COMMAND:{
		switch(wParam){
			case IDOK:{
				int trans,ns;
				ns=GetDlgItemInt(hDlg,IDC_ST_STEP,&trans,FALSE);
				if(!trans || !ns){
					if(MessageBox(hDlg,"Wrong integer value",NULL,MB_RETRYCANCEL)==IDCANCEL)
						EndDialog(hDlg,0);
				}else{
					Step=ns;
					EndDialog(hDlg,1);
				}
				return 1;
			}
			case IDCANCEL:{
				EndDialog(hDlg,0);
				return 1;
			}
		}
   }
}
return 0;
}

