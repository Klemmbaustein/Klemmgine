#include "OS.h"
#if _WIN32
#include <Windows.h>
#include <Shlobj.h>
#include <shobjidl.h> 
#include <Engine/Utility/FileUtility.h>
#include <map>
#include <Psapi.h>
#endif

#if __linux__
#include <unistd.h>
#include <ios>
#include <iostream>
#endif

#include <fstream>
#include <string>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Log.h>

namespace OS
{
	bool ConsoleCanBeHidden = true;
}
void OS::SetConsoleCanBeHidden(bool NewConsoleCanBeHidden)
{
	ConsoleCanBeHidden = NewConsoleCanBeHidden;
}

#if _WIN32
size_t OS::GetMemUsage()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.WorkingSetSize;
}

std::wstring OS::Utf8ToWstring(std::string utf8)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, wchars_num);
	// do whatever with wstr
	std::wstring str = wstr;
	delete[] wstr;
	return str;
}

void OS::SetConsoleWindowVisible(bool Visible)
{
	// Set console code page to UTF-8 so console known how to interpret string data
	SetConsoleOutputCP(CP_UTF8);

	// Enable buffering to prevent VS from chopping up UTF-8 byte sequences
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	SetProcessDPIAware();

	if (ConsoleCanBeHidden)
	{
		::ShowWindow(::GetConsoleWindow(), Visible ? SW_SHOW : SW_HIDE);
	}
}
#endif

#if __linux__
size_t OS::GetMemUsage()
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   size_t vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   return vsize / 1024.0;
}


std::wstring OS::Utf8ToWstring(std::string utf8)
{
	return L"";
}
void OS::SetConsoleWindowVisible(bool Visible)
{
	//Clearing the Console is usually a bad idea on linux
}
#endif



#if _WIN32
std::string OS::ShowOpenFileDialog()
{
	try
	{
		std::string FilePath = "";
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog* pFileOpen;

			// Create the FileOpenDialog object.
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);
				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							FilePath = FileUtil::wstrtostr(pszFilePath);
							return FilePath;
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}
		return FilePath;
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
	return "";
}
#endif

#if __linux__

// I <3 Linux
std::string OS::ShowOpenFileDialog()
{
	char filename[4096];
	FILE *f = popen("zenity --file-selection", "r");
	char* buf = fgets(filename, 4096, f);
	if (!buf)
	{
		return "";
	}
	pclose(f);
	std::string FilenameString = filename;
	StrUtil::ReplaceChar(FilenameString, '\n', "");
	return FilenameString;
}
#endif



//Get the OS's name
#if _WIN64
std::string OS::GetOSString()
{
	int osver = 0;

	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);

	OSVERSIONINFOEXW osInfo;

	HMODULE ntDLL = GetModuleHandleA("ntdll");

	if (!ntDLL)
	{
		return "Unknown";
	}

	*(FARPROC*)&RtlGetVersion = GetProcAddress(ntDLL, "RtlGetVersion");

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		osver = osInfo.dwMajorVersion;
		return "64-bit Windows NT " + std::to_string(osver) + "." + std::to_string(osInfo.dwMinorVersion) + " (Build " + std::to_string(osInfo.dwBuildNumber) + ")";
	}
	return "64-bit Windows (Unknown version)";
}
#else
#if _WIN32
std::string OS::GetOSString()
{
	int osver = 0.0;

	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);

	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		osver = osInfo.dwMajorVersion;
		return "32-bit Windows " + std::to_string(osver) + "." + std::to_string(osInfo.dwMinorVersion) + " (Build " + std::to_string(osInfo.dwBuildNumber) + ")";
	}
	return "32-bit Windows (Unknown version)";
}
#endif
#endif

#if __linux__
std::string OS::GetOSString()
{
	return "Linux";
}
#endif


#if _WIN32

namespace OS
{
	std::map<ConsoleColor, WORD> WindowsColors =
	{
		std::pair(ConsoleColor::White, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
		std::pair(ConsoleColor::Gray, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED),
		std::pair(ConsoleColor::Red, FOREGROUND_RED | FOREGROUND_INTENSITY),
		std::pair(ConsoleColor::Green, FOREGROUND_GREEN | FOREGROUND_INTENSITY),
		std::pair(ConsoleColor::Blue, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY),
		std::pair(ConsoleColor::Yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
	};
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	void SetConsoleColor(ConsoleColor NewColor)
	{
		SetConsoleTextAttribute(hConsole, WindowsColors[NewColor]);
	}
}
#endif

#if __linux__
void OS::SetConsoleColor(ConsoleColor NewColor)
{
	//TODO: implement console colors on linux
}
#endif


#if _WIN32
void OS::OpenFile(std::string Path)
{
	system(("start \"\" \"" + Path + "\"").c_str());
}
#endif
#if __linux__
void OS::OpenFile(std::string Path)
{
	int ret = system(("xdg-open " + Path + " &").c_str());
}
#endif