#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <windows.h>
#include <tlhelp32.h>

std::uint32_t get_pid(std::string_view process_name) {
    std:uint32_t pid = 0;

    HANDLE hdl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hdl == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 proc_entry;
    proc_entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hdl, &proc_entry)) {
        do {
            if (process_name == proc_entry.szExeFile) {
                pid = proc_entry.th32ProcessID;
                break;
            }
        } while (Process32Next(hdl, &proc_entry));
    }

    CloseHandle(hdl);

    return pid;
}

int main(int argc, char **argv) {
    auto pid = get_pid("SPCImage.exe");
    if (pid == 0) {
        std::fprintf(stderr, "Failed to get pid: %#x\n", GetLastError());
        return 1;
    }

    std::printf("Found pid: %#x\n", pid);

    HANDLE hdl = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (pid == 0) {
        std::fprintf(stderr, "Failed to get handle: %#x\n", GetLastError());
        return 1;
    }

    auto dll_path = (std::filesystem::current_path() / "spcinject.dll").string();

    void *mem = VirtualAllocEx(hdl, 0x00, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem) {
        std::fprintf(stderr, "Failed allocate the ddl: %#x\n", GetLastError());
        return 1;
    }

    if (auto rc = WriteProcessMemory(hdl, mem, dll_path.c_str(), dll_path.size(), nullptr); !rc) {
        std::fprintf(stderr, "Failed write the ddl: %#x\n", GetLastError());
        return 1;
    }

    HANDLE hdl_thread = CreateRemoteThread(hdl, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryA, mem, 0, nullptr);

    if (!hdl_thread || hdl_thread == INVALID_HANDLE_VALUE) {
        printf("Failed to start the thread: %#x\n", GetLastError());
        return 1;
    }

    std::printf("Started dll: %#x\n", hdl_thread);

    CloseHandle(hdl_thread);
    return 0;
}
