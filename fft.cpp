#define STRICT
#include <windows.h>
#include <cmath>

static double* fft_cos = nullptr;
static int FFT_len     = 0;

static void
FFT_prepare(int n) {
  int i;
  double PI_n;
  if (FFT_len == n) return;
  FFT_len = n;
  n       = n / 2;
  delete fft_cos;
  fft_cos = new double[n];
  PI_n    = M_PI / n;
  for (i = 0; i < n; ++i) fft_cos[i] = cos(PI_n * i);
}

void
FFT_release() {
  FFT_len = 0;
  delete fft_cos;
  fft_cos = nullptr;
}

// �室��� ���ᨢ - val ᮤ�ন� len �ᥫ,
//  �ᯮ�������� � ���ࢠ��� delt.
// ��室�� ���ᨢ� freq_re � freq_im ���� ᮤ�ঠ��
//  �� len/2 �ᥫ ᫥���騬 ��ࠧ��:
//		freq_re[0] - ����⢨⥫쭠� ���� 0-�� �����. ����
//		freq_im[0] - ����⢨⥫쭠� ���� len-�� �����. ����
//		freq_re[i],freq_im[i] i=1...len-1 - i-� �����. ����
void ft4(double* val, int delt, double* freq_re, double* freq_im);
void
fft(double* val, int delt, double* freq_re, double* freq_im, int len) {
  if (len == 4) {
    ft4(val, delt, freq_re, freq_im);
    return;
  }
  int n  = len / 2;
  int n2 = n / 2;
  int n4 = n2 / 2;
  int i;
  fft(val, delt * 2, freq_re, freq_im, n);
  fft(val + delt, delt * 2, freq_re + n2, freq_im + n2, n);

  // ⥯��� ���ᨢ� ᮤ�ঠ�:
  // �����樥��� ���� ����� �-⮢:
  //  freq_re[0],freq_im[0] - 0-� � n2-�
  //  freq_re[i],freq_im[i] i=1...n2-1 - i-�
  // �����樥��� ���� ������� �-⮢
  //  freq_re[n2],freq_im[n2] - 0-� � n2-�
  //  freq_re[i+n2],freq_im[i+n2] i=1...n2-1 - i-�
  {
    double x;
    // �८�ࠧ������ �����樥�⮢, �� ������ ������ ���
    // freq[0]=freq[0]+freq[n/2]   ������ ��� ���
    // freq[n]=freq[0]-freq[n/2] , �� freq[n] �࠭���� � freq_im[0]
    // freq[n/2]=freq_odd[n/2]-j*freq_even[n/2]=
    //          =freq_im[0]-j*freq_im[n/2]
    x           = freq_re[n2];
    freq_re[n2] = freq_im[0];
    freq_im[n2] = -freq_im[n2];
    freq_im[0]  = freq_re[0] - x;
    freq_re[0]  = freq_re[0] + x;
  }
  for (i = 1; i < n4; ++i) {
    // ������� �����樥�⮢(� �������᭮� ����) i=1...n/4-1
    // freq[i]   =     freq[i]    +     coeff[i]   *freq[n2+i]
    // freq[n-i] =conj(freq[i])   -conj(coeff[i]   *freq[n2+i])
    // freq[n2-i]=     freq[n2-i] +     coeff[n2-i]*freq[n-i]
    // freq[n2+i]=conj(freq[n2-i])-conj(coeff[n2-i]*freq[n-i]
    // ����� coeff[i]=cos(pi*i/n)-j*sin(pi*i/n)=
    //               =cos(pi*i/n)-j*cos(pi*(n2-i)/n)
    //       coeff[n2-i]=cos(pi*(n2-i)/n)-j*sin(pi*(n2-i)/n)=
    //                  =cos(pi*(n2-i)/n)-j*cos(pi*i/n)
    // ������� i/n=(i*delt)/(n*delt)=2*(i*delt)/maxlen
    // cos(2*pi*k/maxlen)=fft_cos[k] k=0...maxlen/2-1
    double c1, c2; // ��ᨭ���
    double x1, y1; //coeff[i]*freq[n2+i]
    double x2, y2; //coeff[n2-i]*freq[n-i]
    c1              = fft_cos[i * delt];
    c2              = fft_cos[(n2 - i) * delt];
    x1              = c1 * freq_re[n2 + i] + c2 * freq_im[n2 + i]; //coeff[i]=c1-j*c2
    y1              = c1 * freq_im[n2 + i] - c2 * freq_re[n2 + i];
    x2              = c2 * freq_re[n - i] + c1 * freq_im[n - i]; //coeff[n2-i]=c2-j*c1
    y2              = c2 * freq_im[n - i] - c1 * freq_re[n - i];
    freq_re[n - i]  = freq_re[i] - x1;
    freq_im[n - i]  = -freq_im[i] + y1;
    freq_re[i]      = freq_re[i] + x1;
    freq_im[i]      = freq_im[i] + y1;
    freq_re[n2 + i] = freq_re[n2 - i] - x2;
    freq_im[n2 + i] = -freq_im[n2 - i] + y2;
    freq_re[n2 - i] = freq_re[n2 - i] + x2;
    freq_im[n2 - i] = freq_im[n2 - i] + y2;
  }
  {
    // ������� ��⠢���� �����樥�⮢(� �������᭮� ����) i=n/4
    // freq[i]  =     freq[i]    +     coeff[i]   *freq[n2+i]
    // freq[n-i]=conj(freq[i])   -conj(coeff[i]   *freq[n2+i])
    // coeff[n/4]=cos(pi/4)-j*sin(pi/4)=cos(pi/4)(1-j)
    int n34  = n2 + n4;
    double c = fft_cos[n4 * delt];
    double x, y; //freq[3n/4]*coeff[n/4]
    x            = c * (freq_re[n34] + freq_im[n34]);
    y            = c * (freq_im[n34] - freq_re[n34]);
    freq_re[n34] = freq_re[n4] - x;
    freq_im[n34] = -freq_im[n4] + y;
    freq_re[n4]  = freq_re[n4] + x;
    freq_im[n4]  = freq_im[n4] + y;
  }
}

void
ft4(double* val, int delt, double* freq_re, double* freq_im) {
  // f0=v0+v1+v2+v3			->re[0]
  // f1=(v0-v2)-j*(v1-v3) ->re[1],im[1]
  // f2=v0-v1+v2-v3			->im[0]
  double s0, s1;
  s0         = val[0] + val[2 * delt];
  s1         = val[delt] + val[3 * delt];
  freq_re[0] = s0 + s1;
  freq_im[0] = s0 - s1;
  freq_re[1] = val[0] - val[2 * delt];
  freq_im[1] = val[3 * delt] - val[delt];
}

void
transform(double* val, double* freq_re, double* freq_im, int len) {
  int i;
  if (len < 8) return;
  for (i = len; !(i & 1); i >>= 1);
  if (i != 1) return;
  FFT_prepare(len);
  fft(val, 1, freq_re, freq_im, len);
}
