inline std::shared_ptr<Gauge>
GaugeByWnd(HWND hWnd) {
  return Window::GetWindow<Gauge>(hWnd);
}

inline double
Gauge::Val(size_t i) {
  return I2D(val[i]);
}

inline float&
Gauge::Imp(size_t i) {
  return imp[i];
}
