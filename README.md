# Simple DLL injector

A simple Windows DLL injector built as a learning project, using the classic `CreateRemoteThread` + `LoadLibraryW` technique.

## How it works

1. Finds the target process by name via `CreateToolhelp32Snapshot` / `Process32First` / `Process32Next`
2. Opens a handle to the process with `OpenProcess`
3. Allocates memory inside the target process with `VirtualAllocEx` and writes the DLL path into it via `WriteProcessMemory`
4. Resolves the address of `LoadLibraryW` from `kernel32.dll` (loaded at the same address across processes)
5. Creates a remote thread in the target process via `CreateRemoteThread`, pointing it at `LoadLibraryW` with the written path as the argument
6. Waits for the thread to finish and checks the result

## Build

**Requirements:**
- Windows 10/11, 64-bit
- Visual Studio 2022 (or newer) with the Desktop development with C++ workload

**Steps:**
1. Open the project in Visual Studio
2. Set configuration to `Release` / `x64`
3. Build (`Ctrl+Shift+B`)

## Usage

injector.exe <path_to_dll>

Or run it without an argument — the program will prompt for the DLL path interactively. It will also ask for the target process name (e.g. `notepad.exe`).

## Notes

- The injector and the target process must be the **same architecture** (x64 injector → x64 process only)
- Requires sufficient privileges to open the target process (same-level or run as administrator depending on the target)
- The compiled `.exe` may get flagged by antivirus software / Windows Defender — this is expected behavior for DLL injection tools and not specific to this project
- Built as an educational project to understand the Windows API mechanics behind DLL injection

## Example DLL

An example DLL (`example_dll/`) is included for testing — it shows a message box on `DLL_PROCESS_ATTACH` to confirm successful injection.

## License

MIT — see [LICENSE](LICENSE)
