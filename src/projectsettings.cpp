#include "projectsettings.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "yaml-cpp/yaml.h"

ProjectSettings::ProjectSettings()
{
	SetDefaultValues();
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

		if (config["m_WebsiteDescription"] != NULL)
			m_WebsiteDescription = config["m_WebsiteDescription"].as<std::string>();

		if (config["m_Url"] != NULL)
			m_Url = config["m_Url"].as<std::string>();

	}
	return true;
}

bool ProjectSettings::SaveToFile(std::string path)
{
	std::ofstream file(path, std::ios_base::trunc);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << path << " for writing" << std::endl;
		return false;
	}
	else
	{
		YAML::Node config;

		config["m_WebsiteName"] = m_WebsiteName;
		config["m_WebsiteDescription"] = m_WebsiteDescription;
		config["m_Url"] = m_Url;

		file << config;
	}

	return true;
}

void ProjectSettings::SetDefaultValues()
{
	m_WebsiteName = "My own website NOW!";
	m_WebsiteDescription = "Yeeeeeeeeep";
	m_Url = "http://www.website.com";
}
