#pragma once

namespace Config
{
	void LoadConfigs(); //Load the Config.jscfg file, execute every line as a console command
}
namespace Script
{
	void LoadScript(const char* Name); //Load the given file, execute every line as a console command
}