//
// Created by lel on 7/15/24.
//
#define STRICT
#include <windows.h>
#include <cstring>

#ifndef DXW32_REGKEY_HPP
#define DXW32_REGKEY_HPP
struct RegKey {
  HKEY key = nullptr;
  long last_error;
  ~RegKey() {
    if (key) RegCloseKey(key);
  }
  operator bool() const { return key != nullptr; }
  explicit RegKey(char const* path) {
    //key        = reinterpret_cast<HKEY>(1);
    last_error = RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &key);
    if (last_error == ERROR_SUCCESS) return;
    last_error = RegCreateKeyExA(HKEY_CURRENT_USER, path, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, nullptr);
  }
  RegKey(RegKey const& parent, char const* path) {
    //key        = reinterpret_cast<HKEY>(1);
    last_error = RegCreateKeyExA(parent.key, path, 0, nullptr, 0, 0, nullptr, &key, nullptr);
  }
  bool SetDefStr(const char* val) {
    last_error = RegSetValueA(key, nullptr, REG_SZ, val, std::strlen(val));
    return last_error == ERROR_SUCCESS;
  }
  bool SetStr(char const* name, const char* val) {
    last_error = RegSetValueExA(key, name, 0, REG_SZ,
                                reinterpret_cast<BYTE*>(const_cast<char*>(val)), std::strlen(val));
    return last_error == ERROR_SUCCESS;
  }
  bool SetDword(char const* name, DWORD val) {
    last_error = RegSetValueExA(key, name, 0, REG_DWORD, reinterpret_cast<BYTE*>(&val), sizeof(val));
    return last_error == ERROR_SUCCESS;
  }
  std::string GetString(char const* name) {
    DWORD vtype;
    DWORD len;
    last_error = RegGetValueA(key, nullptr, name, RRF_RT_REG_SZ, &vtype, buf, &len);
    if (last_error == ERROR_SUCCESS) {
      return std::string { buf, len };
    } else {
      return std::string {};
    }
  }
  DWORD GetDword(char const* name) {
    DWORD vtype;
    DWORD len;
    DWORD value;
    last_error = RegGetValueA(key, nullptr, name, RRF_RT_REG_DWORD, &vtype, &value, &len);
    if (last_error == ERROR_SUCCESS) {
      return value;
    } else {
      return 0;
    }
  }
};

#endif //DXW32_REGKEY_HPP
