#if EDITOR
#include "Importer.h"
#include <fstream>
#include "Engine/FileUtility.h"
#include <filesystem>
#include <UI/EditorUI/EditorUI.h>


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
		//Editor::CurrentUI->ShowPopUpWindow("File already exists!", { PopUpButton("Replace", true, []()
		//	{
		//		std::filesystem::remove(To);
		//		std::filesystem::copy(From, To);
		//	}
		//), PopUpButton("Cancel", false, nullptr) });
	}
	else
	{
		std::filesystem::copy(InputFile, OutputFileName);
	}
}
#endif