#pragma once
#include <string>
#include <vector>

namespace Pack
{
	struct PackFile
	{
		std::string FileName;
		std::string Content;

		PackFile(std::string FileName, std::string Content)
		{
			this->FileName = FileName;
			this->Content = Content;
		}
	};
	//Returns a vector containing the file names and contents of a given .pack file
	std::vector<PackFile> GetPackContents(std::string Pack);
	//Saves the given vector of file names and contents to a .pack file
	bool SaveToPack(std::vector<PackFile> Content, std::string Path);
	//Saves the content of the given folder to a .pack file
	void SaveFolderToPack(std::string Folder, std::string Outf);

	std::string GetFile(std::string File);
}