inline Gauge*
GaugeByWnd(HWND hWnd) {
  return dynamic_cast<Gauge*>(Window::GetWindow(hWnd));
}


inline double
Gauge::Val(long i) {
  return I2D(*(val + i));
}

inline float&
Gauge::Imp(long i) {
  return *(imp + i);
}
