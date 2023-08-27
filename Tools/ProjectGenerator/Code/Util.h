#include <set>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#define MAX_PATH_LENGTH 260
#else
#ifdef __linux__
#define MAX_PATH_LENGTH 4096
#endif
#endif

namespace Util
{

	std::string GetExtension(std::string File);
	std::string wstrtostr(const std::wstring& wstr);
	std::string ShowOpenFileDialog();
	std::string ShowSelectFolderDialog();
	std::string Ask(std::string Question, std::vector<std::string> Options);
	std::string AskForFilePath(std::string PathName);
	void Notify(std::string Message);

	std::string GetFileNameFromPath(std::string FilePath);
	std::vector<std::string> GetAllFilesInFolder(std::string Folder, bool IncludeFolders = false, bool Recursive = true, std::string RelativePath = "/");
	void CopyFolderContent(std::string Folder, std::string To, std::set<std::string> FilesToIgnore = {}, std::atomic<float>* Progress = nullptr, float ProgressAmount = 1);
}