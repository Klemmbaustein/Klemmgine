#include "FileUtility.h"
#include <iostream>
#include <algorithm>

#if _WIN32
#include <Windows.h>
#endif

std::string GetFileNameFromPath(std::string FilePath)
{
	std::string base_filename = FilePath.substr(FilePath.find_last_of("/\\") + 1);
	return base_filename;
}

std::string GetFileNameWithoutExtensionFromPath(std::string FilePath)
{
    size_t lastindex = GetFileNameFromPath(FilePath).find_last_of(".");
    std::string rawname = GetFileNameFromPath(FilePath).substr(0, lastindex);
    return rawname;
}

std::string GetFilePathWithoutExtension(std::string FilePath)
{
    size_t lastindex = FilePath.find_last_of(".");
    std::string rawname = FilePath.substr(0, lastindex);
    return rawname;
}

//https://www.cplusplus.com/forum/windows/74644/
std::string wstrtostr(const std::wstring& wstr)
{
#if _WIN32
    std::string strTo;
    char* szTo = new char[wstr.length() + 1];
    szTo[wstr.size()] = '\0';
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    strTo = szTo;
    delete[] szTo;
    return strTo;
#else
    return "Function not supported on Linux";
#endif
}

std::vector<char> StringToCharVector(std::string In)
{
    std::vector<char> Out;
    for (int i = 0; i < In.size(); i++)
    {
        Out.push_back(In.at(i));
    }
    return Out;
}

std::string VectorToString(std::vector<char> In)
{
    std::string Out;
    for (int i = 0; i < In.size(); i++)
    {
        Out.push_back(In.at(i));
    }
    return Out;
}

std::string GetExtension(std::string FileName)
{
    if (FileName.find_last_of(".") != std::string::npos)
    {
        FileName = FileName.substr(FileName.find_last_of(".") + 1);
        std::transform(FileName.begin(), FileName.end(), FileName.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return FileName;
    }
    return "";
}
