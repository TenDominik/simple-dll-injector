#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

DWORD find_pid(const std::wstring & process_name) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}
	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (!Process32FirstW(snapshot, &pe32)) {
		CloseHandle(snapshot);
		return 0;
	}
	do {
		if (_wcsicmp(pe32.szExeFile, process_name.c_str()) == 0) {
			CloseHandle(snapshot);
			return pe32.th32ProcessID;
		}
	} while (Process32NextW(snapshot, &pe32));
	CloseHandle(snapshot);
	return 0;
}

HANDLE open_target_process(DWORD pid) {
	HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (h_process == NULL) {
		std::cout << "Failed to find process" << std::endl;
		Sleep(3000);
		return 0;
	}
	return h_process;
}

LPVOID write_dll_path(HANDLE h_process, const std::wstring& dll_path) {
	SIZE_T dlSize = (dll_path.size() + 1) * sizeof(wchar_t);
	LPVOID adress = VirtualAllocEx(h_process, NULL, dlSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (adress == NULL) {
		std::cout << "Failed to alloc" << std::endl;
		return 0;
	}
	bool result = WriteProcessMemory(h_process, adress, dll_path.c_str(), dlSize, nullptr);
	if (!result) {
		std::cout << "Failed to write process memory" << std::endl;
		return 0;
	}
	return adress;
}
bool inject_dll(HANDLE h_process, LPVOID remote_path, FARPROC start_routine) {
	LPTHREAD_START_ROUTINE real_start_routine = reinterpret_cast<LPTHREAD_START_ROUTINE>(start_routine);
	HANDLE h_thread = CreateRemoteThread(h_process, NULL, 0, real_start_routine, remote_path, 0, nullptr);
	if (h_thread == NULL) {
		std::cout << "Failed to inject dll" << std::endl;
		return 0;
	}
	CloseHandle(h_thread);
	return true;
}

int wmain(int argc, wchar_t* argv[1]) {
	std::wstring process_name;
	std::wcout << L"Process name: ";
	std::wcin >> process_name;

	std::wstring dll_path;
	if (argc >= 2) {
		dll_path = argv[1];
	}
	else {
		std::wcout << L"Dll path: ";
		std::wcin >> dll_path;
	}

	DWORD pid = find_pid(process_name);
	HANDLE h_process = open_target_process(pid);
	if (h_process == NULL) {
		return 0;
	}
	LPVOID remote_path = write_dll_path(h_process, dll_path);
	if (remote_path == NULL) {
		return 1;
	}
	HMODULE h_kernel = GetModuleHandleW(L"kernel32.dll");
	FARPROC k_adress = GetProcAddress(h_kernel, "LoadLibraryW");
	LPTHREAD_START_ROUTINE start_routine = reinterpret_cast<LPTHREAD_START_ROUTINE>(k_adress);
	bool success = inject_dll(h_process, remote_path, k_adress);
	if (!success) {
		std::cout << "Injection failed" << std::endl;
		CloseHandle(h_process);
		return 1;
	}
	std::cout << "Injection successful!" << std::endl;
	Sleep(3000);
	CloseHandle(h_process);
}
