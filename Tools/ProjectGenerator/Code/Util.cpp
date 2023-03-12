#include "Util.h"
#include "Log.h"
#include <iostream>

#if _WIN32
//Include Windows Headers
#include <Windows.h>
#include <Shlobj.h>
#include <shobjidl.h> 
#endif
namespace Util
{
	std::string GetExtension(std::string File)
	{
		return File.substr(File.find_last_of(".") + 1);
	}
	//https://www.cplusplus.com/forum/windows/74644/
	std::string wstrtostr(const std::wstring& wstr)
	{
#if _WIN32
		std::string strTo;
		char* szTo = new char[wstr.length() + 1];
		szTo[wstr.size()] = '\0';
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1,
			szTo, (int)wstr.length(), NULL, NULL);
		strTo = szTo;
		delete[] szTo;
		return strTo;
#else
		return "Function not supported on Linux";
#endif
	}

#if _WIN32


	std::string ShowOpenFileDialog()
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
								FilePath = wstrtostr(pszFilePath);
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
	std::string ShowSelectFolderDialog()
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
				pFileOpen->SetOptions(FOS_PICKFOLDERS);
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
							FilePath = wstrtostr(pszFilePath);
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
#elif __linux__

#endif
#if _WIN32

	std::string Ask(std::string Question, std::vector<std::string> Options)
	{
		int a = MessageBoxA(NULL, Question.c_str(), "Project manager", MB_YESNO | MB_ICONQUESTION);
		if (a == IDYES)
		{
			return Options[0];
		}
		if (a == IDNO)
		{
			return Options[1];
		}
	}

	std::string AskForFilePath(std::string PathName)
	{
		std::string Path;
		bool IsCorrect = false;
		while (!IsCorrect)
		{
			Log::Print("Please enter the " + PathName + " (enter 'f' for open file dialog)");
			std::cin >> Path;

			if (Path == "f")
			{
				Path = ShowOpenFileDialog();
			}
			if (std::filesystem::exists(Path))
			{
				Log::Print(PathName + " = '" + Path + "'");
				IsCorrect = true;
			}
			else
			{
				Log::Print("'" + Path + "' does not exist", Log::E_WARNING);
			}
		}
		return Path;
	}
	void Notify(std::string Message)
	{
		MessageBoxA(NULL, Message.c_str(), "Project manager", MB_OK);
	}
#elif __linux__

#endif

	std::string GetFileNameFromPath(std::string FilePath)
	{
		std::string base_filename = FilePath.substr(FilePath.find_last_of("/\\") + 1);
		return base_filename;
	}

	std::vector<std::string> GetAllFilesInFolder(std::string Folder, bool IncludeFolders, bool Recursive, std::string RelativePath)
	{
		if (std::filesystem::is_directory(Folder))
		{
			std::vector<std::string> Files;
			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				if (entry.is_directory() && Recursive && entry.path().filename() != "GENERATED")
				{
					auto f = GetAllFilesInFolder(entry.path().string(),
						IncludeFolders, true, RelativePath + GetFileNameFromPath(entry.path().string()) + "/");
					if (IncludeFolders) Files.push_back(RelativePath + GetFileNameFromPath(entry.path().string()));
					Files.insert(Files.end(), f.begin(), f.end());
				}
				else
				{
					Files.push_back(RelativePath + GetFileNameFromPath(entry.path().string()));
				}
			}
			return Files;
		}
		return std::vector<std::string>();
	}


	void CopyFolderContent(std::string Folder, std::string To, std::set<std::string> FilesToIgnore, std::atomic<float>* Progress, float ProgressAmount)
	{
		try
		{
			size_t NumDirs = 0;
			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				NumDirs++;
			}
			float ProgressFraction = ProgressAmount / NumDirs;

			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				std::string name = entry.path().string();
				name = name.substr(name.find_last_of("/\\") + 1);
				if (!FilesToIgnore.contains(name))
				{
					if (std::filesystem::exists(To + "/" + name))
					{
						std::filesystem::remove_all(To + "/" + name);
					}
					std::filesystem::copy(entry.path(), To + "/" + name, std::filesystem::copy_options::recursive);
				}
				if (Progress)
				{
					*Progress += ProgressFraction;
				}
			}
		}
		catch (std::exception& e)
		{
			Log::Print("Error while copying files: " + std::string(e.what()), Log::E_ERROR);
			throw "Copy failed";
		}
	}
}