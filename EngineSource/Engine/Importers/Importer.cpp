#if EDITOR
#include "Importer.h"
#include <fstream>
#include "Engine/Utility/FileUtility.h"
#include <filesystem>
#include <UI/EditorUI/Popups/DialogBox.h>


namespace Importer
{
	std::string From;
	std::string To;
}
void Importer::Import(std::string InputFile, std::string CurrentFilePath)
{
	std::string FileName = std::string(FileUtil::GetFileNameFromPath(InputFile));
	std::string FileNameWithoutExtension = FileName.substr(0, FileName.find_last_of("."));
	std::string OutputFileName = CurrentFilePath + "/" + FileName;
	if (std::filesystem::exists(OutputFileName))
	{
		From = InputFile;
		To = OutputFileName;
		/*new DialogBox("File Import", 0, "File already exists!", {DialogBox::Answer("Replace", []()
			{
				std::filesystem::remove(To);
				std::filesystem::copy(From, To);
			}
		), DialogBox::Answer("Cancel", nullptr) });*/
	}
	else
	{
		std::filesystem::copy(InputFile, OutputFileName);
	}
}
#endif