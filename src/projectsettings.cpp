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

		if (config["m_Url"] != NULL)
			m_Url = config["m_Url"].as<std::string>();

		if (config["m_DefaultLanguage"] != NULL)
			m_DefaultLanguage = config["m_DefaultLanguage"].as<std::string>();

		if (config["m_DefaultLanguageInRoot"] != NULL)
			m_DefaultLanguageInRoot = config["m_DefaultLanguageInRoot"].as<bool>();

		if (config["m_FileExtention"] != NULL)
			m_FileExtention = config["m_FileExtention"].as<std::string>();

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

		config["m_Url"] = m_Url;
		config["m_DefaultLanguage"] = m_DefaultLanguage;
		config["m_DefaultLanguageInRoot"] = m_DefaultLanguageInRoot;
		config["m_FileExtention"] = m_FileExtention;

		file << config;
	}

	return true;
}

void ProjectSettings::SetDefaultValues()
{
	m_Url = "http://www.website.com";
	m_DefaultLanguage = "en";
	m_DefaultLanguageInRoot = true;
	m_FileExtention = "html";
}
