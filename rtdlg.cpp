#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "rt.h"
#include <sstream>
#include <iomanip>
#include <span>
#include <cassert>

//RT* Rt = nullptr;
struct Unit {
  const char* name;
  double factor;
};

Unit ULen[] = {
  { "m", 1.0 },
  { "cm", 0.01 },
  { "mm", 0.001 },
  { nullptr, 0.0 }
};
Unit UTime[] = {
  { "s", 1.0 },
  { "ms", 0.001 },
  { "mks", 0.000001 },
  { nullptr, 0.0 }
};

/** structure containing internal data necessary during lifecycle of RT dialog box
 *
 */
struct RtDlgState {
  HWND hDlg;                                     ///<The dialog window
  std::shared_ptr<RT> Rt;                        ///< The RT window
  std::vector<HWND> Chns;                        ///< List of channels in IDC_RT_CHNS listbox
  std::vector<std::pair<int, bool>> lines;       ///< list of items in TDC_RT_LINES listbox
  void AddLine(int line);                        ///< add a RT line to array and to the listbox
  void AddChn(Gauge const&, bool chk = false);   ///< add a channel into array and to the listbox
  void RemoveChn(size_t idx);                    ///< remove channel from listbox and the array
  void InitDialog();                             ///< Fill dialog elements with values
  void SelChangeLine(int l_idx, bool chk_state); ///< respond to select/unselect of line in DlgBox
  void CheckLine(int l_idx, bool chn_state);     ///< select line and add chalnnels
  void UncheckLine(int l_idx);                   ///< deselect line and remove channels
  RtDlgState(HWND hDlg, HWND hRt);
  static char const* Angles[];
  WINBOOL Command(WORD idctl, WORD notcode, void (*)(bool));

  void SetItemText(int id, std::string const& text) const {
    SetDlgItemText(hDlg, id, text.c_str());
  }
  [[nodiscard]] std::string_view GetItemText(int id) const {
    return { buf, GetDlgItemTextA(hDlg, id, buf, std::size(buf)) };
  }
  void SetItem(int id, double val) const {
    SetDlgItemDouble(hDlg, id, val);
  }
  void GetItem(int id, double& val) const {
    GetDlgItemDouble(hDlg, id, val);
  }

  [[nodiscard]] bool IsChecked(int id) const {
    return IsDlgButtonChecked(hDlg, id);
  }
  void CheckItem(int id, bool val) const {
    CheckDlgButton(hDlg, id, val);
  }

  void SetItemUnit(int ID, double Val, std::span<Unit> U) const;
  bool GetItemUnit(int ID, double& val, std::span<Unit> U) const;
  auto SendItemMessage(int ID, WORD msg, WPARAM wParam, LPARAM lParam) const {
    return SendDlgItemMessage(hDlg, ID, msg, wParam, lParam);
  }
};

char const* RtDlgState::Angles[] = {
  "-45",
  "-30",
  "-15",
  "0",
  "15",
  "30",
  "45",
};

RtDlgState::RtDlgState(HWND hDlg, HWND hRt)
    : hDlg(hDlg),
      Rt(Window::GetWindow<RT>(hRt)) {

  //Fill list of all lines in all gauges
  for (auto G : GaugeIterator()) {
    AddLine(G->angle);
  }
  if (!Rt) {
    Rt = std::make_shared<RT>();
  }
}

/**
 * Maintain sorted list of lines
 * @param line line to all if not yet added
 */
void
RtDlgState::AddLine(int line) {
  //  if (lines.empty()) {
  //    lines.emplace_back(line, false);
  //  } else {
  auto it = std::ranges::lower_bound(lines, line, {}, &std::pair<int, bool>::first);
  if (it == lines.end() || it->first != line) {
    lines.insert(it, std::make_pair(line, false));
  }
  //  }
}

/**
 * Fill dialog items with appropriate values
 * Delegate the most part of work to RT::InitDlg
 */
void
RtDlgState::InitDialog() {
  for (auto [l, chk] : lines) {
    auto s = std::to_string(l);
    SendItemMessage(IDC_RT_LINES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str()));
  }
  for (auto s : Angles) {
    SendItemMessage(IDC_RT_ANGL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
  }
  Rt->InitDlg(*this);
  SetFocus(GetDlgItem(hDlg, IDC_RT_LINES));
}

/**
 * Add a channel to internal list and to list box
 * @param G the channel to add
 * @param chk the selected state of line in list box
 */
void
RtDlgState::AddChn(const Gauge& G, bool chk) {
  auto it = std::ranges::find(Chns, G.hWnd);
  long pos;
  if (it == Chns.end()) {
    std::stringstream ss;
    ss << G.ChNum << ':' << G.angle << '-' << std::setprecision(2) << G.radius;
    auto chtxt = ss.str();
    pos        = SendItemMessage(IDC_RT_CHNS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(chtxt.c_str()));
    Chns.insert(std::next(Chns.begin(), pos), G.hWnd);
  } else {
    pos = std::distance(Chns.begin(), it);
  }
  SendItemMessage(IDC_RT_CHNS, LB_SETSEL, chk, pos);
}

/**
 * Remove a channel from both internal list and the list box
 * @param idx
 */
void
RtDlgState::RemoveChn(size_t idx) {
  auto len = SendItemMessage(IDC_RT_CHNS, LB_DELETESTRING, static_cast<WPARAM>(idx), 0);
  Chns.erase(std::next(Chns.begin(), idx));
  if (Chns.size() != len) {
    MessageBox(hFrame, "Internal error ", "List of channels", MB_OK | MB_ICONEXCLAMATION);
  }
}

/**
 * React on possible change of selected state of a line in ID_RT_LINES
 * @param l_idx
 * @param chk_state
 */
void
RtDlgState::SelChangeLine(int l_idx, bool chk_state) {
  auto& [line, chk] = lines[l_idx];
  if (chk == chk_state) return; // no changes

  if (chk_state) { // Line is selected; Add all channels into listbox
    CheckLine(l_idx, true);
  } else { //line is deselected
    UncheckLine(l_idx);
  }
  chk = chk_state;
  //lines[l_idx].second = chk_state;
}

void
RtDlgState::CheckLine(int l_idx, bool chn_state) {
  for (auto G : GaugeIterator()) {
    if (G->angle == lines[l_idx].first) {
      AddChn(*G, chn_state);
    }
  }
  SendItemMessage(IDC_RT_LINES, LB_SETSEL, 1, l_idx);
  lines[l_idx].second = true;
}

void
RtDlgState::UncheckLine(int l_idx) {
  for (size_t idx = 0; idx < Chns.size();) {
    auto G = GaugeByWnd(Chns[idx]);
    if (G && G->angle == lines[l_idx].first) {
      RemoveChn(idx);
    } else {
      ++idx;
    }
  }
  SendItemMessage(IDC_RT_LINES, LB_SETSEL, 0, l_idx);
  lines[l_idx].second = false;
}

inline void
CMAX(double& n, double i) {
  if (i > n) n = i;
}
inline void
CMIN(double& n, double i) {
  if (i < n) n = i;
}

auto
select_unit(double Val, std::span<Unit> U) -> Unit const& {
  if (Val == 0.0) return U.front();
  for (auto const& u : U) {
    if (fabs(Val / u.factor) > 1.0) return u;
  }
  return U.back();
}

void
RtDlgState::SetItemUnit(int ID, double Val, std::span<Unit> U) const {
  auto const& u = select_unit(Val, U);
  std::stringstream ss;
  ss << (Val / u.factor) << ' ' << u.name;
  SetItemText(ID, ss.str());
}

bool
RtDlgState::GetItemUnit(int ID, double& val, std::span<Unit> U) const {
  double w;
  char* pos;
  auto txt = GetItemText(ID);
  if (txt.empty()) return false;
  w = std::strtod(txt.data(), &pos);
  while (*pos == ' ') ++pos;
  auto u_dlg = txt.substr(pos - txt.begin());
  if (u_dlg.empty()) {
    val = w * U[0].factor;
    return true;
  }
  auto it = std::ranges::find_if(U, [u_dlg](Unit const& u) {
    return strnicmp(u.name, u_dlg.data(), u_dlg.size()) == 0;
  });
  if (it == U.end()) return false;
  val = w * it->factor;
  return true;
}

WINBOOL
RtDlgState::Command(WORD idCtl, WORD wNotCode, void (*end_dialog)(bool)) {
  switch (idCtl) {

    case IDC_RT_PL:
      if (wNotCode == BN_CLICKED) {
        if (IsChecked(IDC_RT_PL)) {
          CheckItem(IDC_RT_PR, false);
        }
      }
      return TRUE;

    case IDC_RT_PR:
      if (wNotCode == BN_CLICKED) {
        if (IsChecked(IDC_RT_PR))
          CheckItem(IDC_RT_PL, false);
      }
      return TRUE;

    case IDC_RT_LINES: {
      if (wNotCode == LBN_SELCHANGE) {
        auto i = SendItemMessage(IDC_RT_LINES, LB_GETCARETINDEX, 0, 0);
        auto j = SendItemMessage(IDC_RT_LINES, LB_GETSEL, i, 0) > 0;
        SelChangeLine(i, j);
        return TRUE;
      } //LBN_SELCHANGE
    } break; //case IDC_RT_LINES

    case IDC_SAVE_RT: {
      OFN ofn;
      ofn.lpstrFile       = fname;
      ofn.nMaxFile        = sizeof(fname);
      ofn.lpstrFilter     = "RT diagrams (*.rt)\0*.rt\0All Files\0*.*\0";
      ofn.lpstrInitialDir = Directory;
      ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
      ofn.lpstrDefExt = "rt";
      fname[0]        = 0;
      if (!GetSaveFileName(&ofn)) break;
      {
        auto tmp = std::make_shared<RT>();
        tmp->ReadDlg(*this);

        strcpy(tmp->FontName, Rt->FontName);
        tmp->FontHeight = Rt->FontHeight;
        tmp->FontType   = Rt->FontType;
        tmp->LineWidth  = Rt->LineWidth;

        tmp->Save(fname);
      }
      return TRUE;
    }

    case IDC_LOAD_RT: {
      OFN ofn;
      ofn.lpstrFile       = fname;
      ofn.nMaxFile        = sizeof(fname);
      ofn.lpstrFilter     = "RT diagrams (*.rt)\0*.rt\0All Files\0*.*\0";
      ofn.lpstrInitialDir = Directory;
      ofn.Flags |= OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
      ofn.lpstrDefExt = "rt";
      fname[0]        = 0;
      if (!GetOpenFileName(&ofn)) break;
      {
        auto tmp = std::make_shared<RT>();
        tmp->Load(fname);

        strcpy(Rt->FontName, tmp->FontName);
        Rt->FontHeight = tmp->FontHeight;
        Rt->FontType   = tmp->FontType;
        Rt->LineWidth  = tmp->LineWidth;
        tmp->InitDlg(*this);
      }
      return TRUE;
    }

    case IDOK: {
      if (!Rt->ReadDlg(*this)) {
        auto rsp = MessageBox(hFrame, "No channels selected!", nullptr, MB_RETRYCANCEL);
        if (rsp == IDCANCEL) end_dialog(false);
      } else {
        end_dialog(true);
      }
      return TRUE;
    } //case IDOK

    case IDCANCEL:
      end_dialog(false);
      return TRUE;

    default:
      break;
  } //switch(idCtl)

  return FALSE;
}

int
RT::ReadDlg(RtDlgState& dlg) {
  int N, i;
  N = (int) dlg.SendItemMessage(IDC_RT_CHNS, LB_GETSELCOUNT, 0, 0);
  if (!N) return FALSE;
  N = (int) dlg.SendItemMessage(IDC_RT_CHNS, LB_GETCOUNT, 0, 0);
  Chns.clear();
  for (i = 0; i < N; ++i) {
    if (dlg.SendItemMessage(IDC_RT_CHNS, LB_GETSEL, i, 0) <= 0) continue;
    auto G = GaugeByWnd(dlg.Chns[i]);
    if (G) {
      if (Chns.empty()) {
        R0 = G->radius;
        R1 = G->radius;
        T0 = G->Start();
        T1 = G->Final();
      } else {
        CMIN(R0, G->radius);
        CMAX(R1, G->radius);
        CMIN(T0, G->Start());
        CMAX(T1, G->Final());
      }
      Chns.push_back(G->hWnd);
    }
  } //for(i
  R0_def = !dlg.GetItemUnit(IDC_RT_RMIN, R0, ULen);
  R1_def = !dlg.GetItemUnit(IDC_RT_RMAX, R1, ULen);
  T0_def = !dlg.GetItemUnit(IDC_RT_TMIN, T0, UTime);
  T1_def = !dlg.GetItemUnit(IDC_RT_TMAX, T1, UTime);

  dlg.GetItem(IDC_RT_HGHT, P_height);
  dlg.GetItem(IDC_RT_ANGL, R_angle);
  R_left = dlg.IsChecked(IDC_RT_RL);
  clr    = dlg.IsChecked(IDC_RT_CLR);
  if (dlg.IsChecked(IDC_RT_PL))
    P_st = Left;
  else if (dlg.IsChecked(IDC_RT_PR))
    P_st = Right;
  else
    P_st = None;
  return TRUE;
}

void
RT::InitDlg(RtDlgState& dlg) {
  dlg.SetItem(IDC_RT_ANGL, R_angle);
  dlg.SetItem(IDC_RT_HGHT, P_height);
  dlg.CheckItem(R_left ? IDC_RT_RL : IDC_RT_RR, TRUE);
  dlg.CheckItem(IDC_RT_PL, P_st == Left);
  dlg.CheckItem(IDC_RT_PR, P_st == Right);
  dlg.CheckItem(IDC_RT_CLR, clr);
  if (!T0_def) {
    dlg.SetItemUnit(IDC_RT_TMIN, T0, UTime);
  } else {
    dlg.SetItemText(IDC_RT_TMIN, "Auto");
  }
  if (!T1_def) {
    dlg.SetItemUnit(IDC_RT_TMAX, T1, UTime);
  } else {
    dlg.SetItemText(IDC_RT_TMAX, "Auto");
  }
  if (!R0_def) {
    dlg.SetItemUnit(IDC_RT_RMIN, R0, ULen);
  } else {
    dlg.SetItemText(IDC_RT_RMIN, "Auto");
  }
  if (!R1_def) {
    dlg.SetItemUnit(IDC_RT_RMAX, R1, ULen);
  } else {
    dlg.SetItemText(IDC_RT_RMAX, "Auto");
  }
  for (auto hG : Chns) {
    auto G = GaugeByWnd(hG);
    if (!G) {
      MessageBox(hFrame, "non-existing Channel window", "RT dialog", MB_OK | MB_ICONEXCLAMATION);
      continue;
    }
    auto it = std::ranges::find(dlg.lines, G->angle, &std::pair<int, bool>::first);
    if (it == dlg.lines.end()) {
      MessageBox(hFrame, "Something goes wrong, unknown line", "RT dialog", MB_OK | MB_ICONEXCLAMATION);
      continue;
    }
    dlg.CheckLine(it - dlg.lines.begin(), false);
  }
  for (auto hG : Chns) {
    auto G = GaugeByWnd(hG);
    if (G) dlg.AddChn(*G, true);
  }
}

DLG_PROC(RTdlg) {
  static std::shared_ptr<RtDlgState> dlg {};
  auto end_dialog = [](bool Ok) {
    assert(dlg);
    assert(dlg->Rt);
    if (Ok) {
      dlg->Rt->Create();
      assert(dlg->Rt->hWnd);
      InvalidateRect(dlg->Rt->hWnd, nullptr, TRUE);
    }
    EndDialog(dlg->hDlg, static_cast<INT_PTR>(Ok));
    dlg.reset();
  };
  if (dlg) {
    assert(hDlg == dlg->hDlg);
    assert(dlg->Rt);
  }
  switch (msg) {
    case WM_INITDIALOG: {
      dlg = std::make_shared<RtDlgState>(hDlg, reinterpret_cast<HWND>(lParam));
      dlg->InitDialog();
      return FALSE;
    } //case WM_INITDIALOG
    case WM_DESTROY: {
      dlg.reset();
      return FALSE;
    }
    case WM_COMMAND: {
      if (dlg)
        return dlg->Command(LOWORD(wParam), HIWORD(wParam), end_dialog);
      else
        return FALSE;
    } //case WM_COMMAND

    default:
      break;
  } //switch(msg)
  return FALSE;
}

void
RT::Create() {
  if (hWnd) return;
  MDICREATESTRUCT cs;
  cs.szClass = "DXWRT";
  cs.szTitle = "R-T diagram";
  cs.hOwner  = hInst;
  cs.x = cs.y = CW_USEDEFAULT;
  cs.cx       = 550;
  cs.cy       = 650;
  cs.style    = 0;
  cs.lParam   = 0;
  Window::Create(cs);
}

void
RT::Load(char* rt_fname) {
  int i;

  GetPrivateProfileString("RT", "Tmin", "Auto", buf, 100, rt_fname);
  T0_def = !strcmp(buf, "Auto");
  if (!T0_def) T0 = atof(buf);

  GetPrivateProfileString("RT", "Tmax", "Auto", buf, 100, rt_fname);
  T1_def = !strcmp(buf, "Auto");
  if (!T1_def) T1 = atof(buf);

  GetPrivateProfileString("RT", "Rmin", "Auto", buf, 100, rt_fname);
  R0_def = !strcmp(buf, "Auto");
  if (!R0_def) R0 = atof(buf);

  GetPrivateProfileString("RT", "Rmax", "Auto", buf, 100, rt_fname);
  R1_def = !strcmp(buf, "Auto");
  if (!R1_def) R1 = atof(buf);

  GetPrivateProfileString("RT", "Height", "10", buf, 100, rt_fname);
  P_height = atof(buf);
  if (P_height == 0.0) P_height = 10;

  GetPrivateProfileString("RT", "Angle", "0", buf, 100, rt_fname);
  R_angle = atof(buf);

  GetPrivateProfileString("RT", "Color", "No", buf, 100, rt_fname);
  clr = !strcmp(buf, "Yes");

  GetPrivateProfileString("RT", "Raxis", "Left", buf, 100, rt_fname);
  R_left = !strcmp(buf, "Left");

  GetPrivateProfileString("RT", "Paxis", "Right", buf, 100, rt_fname);
  if (strcmp(buf, "Left") == 0) {
    if (strcmp(buf, "Right") == 0) P_st = None;
    else
      P_st = Right;
  } else
    P_st = Left;

  GetPrivateProfileString("RT", "Nch", "0", buf, 100, rt_fname);
  int n = atoi(buf);
  Chns.clear();
  for (i = 0; i < n; ++i) {
    char cnum[5];
    cnum[0] = 'C';
    itoa(i, cnum + 1, 10);
    GetPrivateProfileString("Channels", cnum, "---", buf, 100, rt_fname);
    auto G = GaugeByChNum(buf);
    if (G) Chns.push_back(G->hWnd);
  }
  GetPrivateProfileString("RT", "FontName", "Arial", buf, 100, rt_fname);
  strcpy(FontName, buf);

  GetPrivateProfileString("RT", "FontHeight", "8", buf, 100, rt_fname);
  FontHeight = (int) (10 * atof(buf));

  FontType = 0;
  GetPrivateProfileString("RT", "FontBold", "0", buf, 100, rt_fname);
  if (atoi(buf)) FontType |= BOLD_FONTTYPE;
  GetPrivateProfileString("RT", "FontItalic", "0", buf, 100, rt_fname);
  if (atoi(buf)) FontType |= ITALIC_FONTTYPE;

  GetPrivateProfileString("RT", "LineWidth", ".7", buf, 100, rt_fname);
  LineWidth = (int) (10 * atof(buf));
}

void
RT::Save(char* rt_fname) {

  if (T0_def) strcpy(buf, "Auto");
  else
    gcvt(T0, 6, buf);
  WritePrivateProfileString("RT", "Tmin", buf, rt_fname);
  if (T1_def) strcpy(buf, "Auto");
  else
    gcvt(T1, 6, buf);
  WritePrivateProfileString("RT", "Tmax", buf, rt_fname);
  if (R0_def) strcpy(buf, "Auto");
  else
    gcvt(R0, 6, buf);
  WritePrivateProfileString("RT", "Rmin", buf, rt_fname);
  if (R1_def) strcpy(buf, "Auto");
  else
    gcvt(R1, 6, buf);
  WritePrivateProfileString("RT", "Rmax", buf, rt_fname);
  gcvt(P_height, 4, buf);
  WritePrivateProfileString("RT", "Height", buf, rt_fname);
  gcvt(R_angle, 4, buf);
  WritePrivateProfileString("RT", "Angle", buf, rt_fname);
  WritePrivateProfileString("RT", "Color", (clr ? "Yes" : "No"), rt_fname);
  WritePrivateProfileString("RT", "Raxis", (R_left ? "Left" : "Right"), rt_fname);
  WritePrivateProfileString("RT", "Paxis", (P_st == Left ? "Left" : (P_st == Right ? "Right" : "No")), rt_fname);
  itoa(static_cast<int>(Chns.size()), buf, 10);
  WritePrivateProfileString("RT", "Nch", buf, rt_fname);

  int i = 0;
  for (auto hG : Chns) {
    auto G = GaugeByWnd(hG);
    if (!G) continue;
    buf[0] = 'C';
    itoa(i, buf + 1, 10);
    WritePrivateProfileString("Channels", buf, G->ChNum, rt_fname);
    ++i;
  }

  WritePrivateProfileString("RT", "FontName", FontName, rt_fname);
  WritePrivateProfileString("RT", "FontHeight", gcvt(FontHeight * 0.1, 4, buf), rt_fname);
  WritePrivateProfileString("RT", "FontBold", (FontType & BOLD_FONTTYPE ? "1" : "0"), rt_fname);
  WritePrivateProfileString("RT", "FontItalic", (FontType & ITALIC_FONTTYPE ? "1" : "0"), rt_fname);
  WritePrivateProfileString("RT", "LineWidth", gcvt(LineWidth * 0.1, 4, buf), rt_fname);
}
