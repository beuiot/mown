#include "localization.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "yaml-cpp/yaml.h"
#include "contentfactory.h"

Localization::Localization()
{
	SetDefaultValues();
}

Localization::~Localization()
{

}

void Localization::Clear()
{
	m_LocalizedIds.clear();
}

bool Localization::LoadFromFile(std::string path)
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

		for (auto idIt = config.begin(); idIt != config.end(); ++idIt)
		{
			std::string id = idIt->first.as<std::string>();

			if (m_LocalizedIds.find(id) == m_LocalizedIds.end())
				m_LocalizedIds[id] = std::map<std::string, std::string>();

			for (auto valuePairIt = idIt->second.begin(); valuePairIt != idIt->second.end(); ++valuePairIt)
			{
				std::string language = valuePairIt->first.as<std::string>();
				std::string text = valuePairIt->second.as<std::string>();

				m_LocalizedIds[id][language] = text;
			}
		}
	}
	return true;
}

bool Localization::SaveToFile(std::string path)
{
	std::ofstream file(path, std::ios_base::trunc);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << path << " for writing" << std::endl;
		return false;
	}
	else
	{
		YAML::Node config, node1, node2;

		node1["en"] = "Website name";
		node1["fr"] = "Nom du site";
		config["WEBSITE_NAME"] = node1;

		node2["en"] = "Website description";
		node2["fr"] = "Description du site";
		config["WEBSITE_DESCRIPTION"] = node2;

		file << config;
	}

	return true;
}

void Localization::SetDefaultValues()
{
}

void Localization::ProcessString(std::string& text, const std::string& language, const std::string& defaultLanguage)
{
	for (auto idIt = m_LocalizedIds.begin(); idIt != m_LocalizedIds.end(); ++idIt)
	{
		std::string id = "@" + idIt->first + "@";
		std::string translation = GetLocalizedString(idIt->second, language, defaultLanguage);

		ContentFactory::ReplaceInString(text, id, translation);
	}
}

std::string Localization::GetLocalizedString(const std::map<std::string, std::string>& translations, const std::string& language, const std::string& defaultLanguage)
{
	std::string result;

	std::map<std::string, std::string>::const_iterator it;

	if ((it = translations.find(language)) != translations.end())
		result = it->second;
	else if ((it = translations.find(defaultLanguage)) != translations.end())
		result = it->second;
	else
		result = translations.begin()->second;

	return result;
}

bool Localization::GetLocalizedStringFromId(const std::string& id, std::string& result, const std::string& language) const
{
	std::map<std::string, std::map<std::string, std::string>>::const_iterator idIt;
	std::map<std::string, std::string>::const_iterator translationsIt;

	if ((idIt = m_LocalizedIds.find(id)) != m_LocalizedIds.end())
	{
		if ((translationsIt = idIt->second.find(language)) != idIt->second.end())
		{
			result = translationsIt->second;
			return true;
		}
	}

	return false;
}