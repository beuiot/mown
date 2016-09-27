#include "projectsettings.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "yaml-cpp/yaml.h"

ProjectSettings::ProjectSettings()
{

}

ProjectSettings::~ProjectSettings()
{

}

bool ProjectSettings::LoadFromFile(std::string path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << path << std::endl;
		return false;
	}
	else
	{
		YAML::Node config = YAML::Load(file);

		if (config["m_WebsiteName"] != NULL)
			m_WebsiteName = config["m_WebsiteName"].as<std::string>();
		else
			m_WebsiteName = "My own website NOW!";

		if (config["m_WebsiteDescription"] != NULL)
			m_WebsiteDescription = config["m_WebsiteDescription"].as<std::string>();
		else
			m_WebsiteDescription = "Yeeeeeeeeep";

		if (config["m_Url"] != NULL)
			m_Url = config["m_Url"].as<std::string>();
		else
			m_Url = "http://www.website.com";

	}
	return true;
}
