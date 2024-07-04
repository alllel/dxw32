#ifndef _RT_H_
#define _RT_H_

typedef enum { Left  = 1,
               Right = -1,
               None  = 0 } state;
class RT : public Window {
 public:
  BOOL R_left : 1;
  state P_st : 2;
  BOOL T0_def : 1, T1_def : 1, R0_def : 1, R1_def : 1;
  BOOL clr : 1;
  double P_height, R_angle;
  double T0, T1, R0, R1;
  int n;
  HLOCAL Chns[GMAX];
  void InitDlg(HWND);
  int ReadDlg(HWND);
  virtual void Draw(HDC, RECT&, DCtype, RECT*);
  virtual char* PicName();
  virtual BOOL Command(WPARAM cmd);
  virtual BOOL WinProc(Msg& M);
  void FindChannell(WORD);
  void Load(char* fname);
  void Save(char* fname);
  RT();
};

extern RT* Rt;

#endif
