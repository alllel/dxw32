inline Gauge*
GaugeByWnd(HWND hWnd) {
  return dynamic_cast<Gauge*>(Window::GetWindow(hWnd));
}

inline Gauge*
Gauge::LockGauge(HLOCAL h) {
  return dynamic_cast<Gauge*>(LockWindow(h));
}

inline double
Gauge::Val(long i) {
  return I2D(*(val + i));
}

inline float&
Gauge::Imp(long i) {
  return *(imp + i);
}
