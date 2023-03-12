#pragma once
#include <string>
#include <vector>

std::string GetFileNameFromPath(std::string FilePath);

std::string GetFileNameWithoutExtensionFromPath(std::string FilePath);

std::string GetFilePathWithoutExtension(std::string FilePath);

std::string wstrtostr(const std::wstring& wstr);

std::vector<char> StringToCharVector(std::string In);

std::string VectorToString(std::vector<char> In);

std::string GetExtension(std::string FileName);