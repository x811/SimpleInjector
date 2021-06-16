#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <libloaderapi.h>

void get_proc_id(const char* szWindowName, DWORD& procId)
{
	GetWindowThreadProcessId(FindWindow(NULL, szWindowName), &procId);
}

void show_error_msg(const char* error_msg)
{
	MessageBox(0, error_msg, "Error", MB_OK);
	exit(-1);
}

bool file_exists(std::string file_name)
{
	struct stat buffer;
	return (stat(file_name.c_str(), &buffer) == 0);
}

int main()
{
	DWORD procId;
	char dll_path[MAX_PATH];

	std::string file_name;
	std::string target_name;

	{
		std::cout << "Write File Name (Must Be In Same Directory)\n";
		std::cin >> file_name;
		std::cout << "Write Target Name\n";
		std::cin >> target_name;
	}

	if (!file_exists(file_name))
	{
		show_error_msg("File Doesnt Exist");
	}

	if (!GetFullPathName(file_name.c_str(), MAX_PATH, dll_path, nullptr))
	{
		show_error_msg("Failed To Get Full Path");
	}

	get_proc_id(target_name.c_str(), procId);
	if(procId == NULL)
	{
		show_error_msg("Failed To Get Process Id");
	}
	
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

	if (hProc == NULL)
	{
		show_error_msg("Handle Not Defined");
	}

	void* allocated_memory = VirtualAllocEx(hProc, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!allocated_memory)
	{
		show_error_msg("Memory Not Allocated");
	}

	if (!WriteProcessMemory(hProc, allocated_memory, dll_path, MAX_PATH, nullptr))
	{
		show_error_msg("Failed To WriteProcessMemory");
	}

	HANDLE hRemote = CreateRemoteThread(hProc, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_memory, NULL, nullptr);
	if (!hRemote)
	{
		show_error_msg("Remote Thread Failed");
	}

	CloseHandle(hProc);
	VirtualFreeEx(hProc, allocated_memory, NULL, MEM_RELEASE);
	MessageBox(0, "Injected", "Success", MB_OK);
}