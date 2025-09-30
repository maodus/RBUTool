#include "tool/ExeRunner.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>

#include <cstdio>
#include <cstdlib>

#endif

static std::string quote(std::string_view s) {
#if defined(_WIN32)
  std::string out;
  out.reserve(s.size() + 2);
  out.push_back('"');
  for (char c : s) {
    if (c == '"') out.push_back('"');
    out.push_back(c);
  }
  out.push_back('"');
  return out;
#else
  std::string out;
  out.reserve(s.size() + 2);
  out.push_back('\'');
  for (char c : s) {
    if (c == '\'') {
      out += "'\\''";
    } else
      out.push_back(c);
  }
  out.push_back('\'');
  return out;
#endif
}

#if defined(_WIN32)
static ExeResult RunWindows(const std::string& exe_path,
                            const std::vector<std::string>& args) {
  std::string cmd = quote(exe_path);
  for (auto& a : args) {
    cmd.push_back(' ');
    cmd += a;
  }

  SECURITY_ATTRIBUTES sa{};
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;
  HANDLE hRead = nullptr, hWrite = nullptr;
  if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    throw std::runtime_error("CreatePipe failed");
  SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFOA si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
  si.hStdOutput = hWrite;
  si.hStdError = hWrite;

  PROCESS_INFORMATION pi{};
  std::string mutableCmd = cmd;
  std::cout << mutableCmd << std::endl;

  BOOL ok = CreateProcessA(nullptr, mutableCmd.data(), nullptr, nullptr, TRUE,
                           CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
  CloseHandle(hWrite);

  ExeResult res;
  if (!ok) {
    CloseHandle(hRead);
    res.exit_code = -1;
    res.log_out = "CreateProcess failed";
    return res;
  }

  char buf[4096];
  DWORD bytes;
  while (ReadFile(hRead, buf, sizeof(buf), &bytes, nullptr) && bytes > 0) {
    res.log_out.append(buf, buf + bytes);
  }
  CloseHandle(hRead);

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 0;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  res.exit_code = static_cast<int>(code);
  return res;
}
#endif

ExeRunner::ExeRunner(const std::string exe_path) : path_(exe_path) {}

ExeResult ExeRunner::Run(const std::vector<std::string>& args) {
  return RunWindows(this->path_, args);
}
