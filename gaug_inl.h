inline Gauge*
GaugeByWnd(HWND hWnd) {
  return dynamic_cast<Gauge*>(Window::GetWindow(hWnd));
}


inline double
Gauge::Val(size_t i) {
  return I2D(val[i]);
}

inline float&
Gauge::Imp(size_t i) {
  return imp[i];
}
