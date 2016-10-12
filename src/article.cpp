#include "article.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "yaml-cpp/yaml.h"
#include "boost/date_time.hpp"
#include <locale>
#include <boost/filesystem.hpp>

#include "contentfactory.h"

Article::Article() :
	m_LocalPreview(true),
	m_IsPage(false),
	m_CurrentLanguage(""),
	m_HasCurrentLanguage(false)
{

}

Article::~Article()
{

}

bool Article::LoadFromFile(std::string path, std::vector<std::string>& languages)
{
	m_SourceFilePath = path;
	bool hasToSave = false;

	if (LoadFile())
	{
		for (auto it = m_Data.begin();
		it != m_Data.end();
			++it)
		{
			if (!it->ParseYaml(m_Data[0]))
				hasToSave = true;

			bool languageFound = false;
			for (auto lgIt = languages.begin(); lgIt != languages.end(); ++lgIt)
			{
				if (*lgIt == it->m_Language)
				{
					languageFound = true;
					break;
				}
			}

			if (!languageFound)
				languages.push_back(it->m_Language);
		}
	}

	if (m_Data.size() == 0)
	{
		ArticleData data;
		m_Data.push_back(data);

		hasToSave = true;
	}

	if (hasToSave)
	{
		SaveFile();
	}

	FindData("", m_CurrentData);

	return true;
}


std::string Article::GetSourceFilePath()
{
	return m_SourceFilePath;
}

std::string Article::Dump(bool showContent)
{
	std::stringstream result;

	result << m_CurrentData.m_Title << "(" << m_CurrentData.m_Language << ")" << std::endl;
	result << m_CurrentData.m_Date << std::endl;

	for (auto it = m_CurrentData.m_Tags.begin(); it != m_CurrentData.m_Tags.end(); ++it)
	{
		result << *it << " ";
	}
	result << std::endl;

	if (showContent)
		result << m_CurrentData.m_Content << std::endl;

	return result.str();
}

std::string Article::GetFileName()
{
	return GetFileNameForLanguage(m_CurrentLanguage);
}

std::string Article::GetFileNameForLanguage(const std::string& language)
{
	ArticleData data;

	if (!FindData(language, data))
		data = m_CurrentData;

	using namespace boost::gregorian;

	std::stringstream fileName;

	std::string title = data.m_Title;
	ContentFactory::SanitizeString(title);

	if (!m_IsPage)
	{
		date_facet* facet = new date_facet();
		fileName.imbue(std::locale(std::locale::classic(), facet));

		facet->format("%Y-%m-%d");
		fileName << data.m_Date;

		fileName << "-";
	}

	fileName << title;

	return fileName.str();
}


bool Article::HasFileName(const std::string& fileName)
{
	for (auto it = m_Data.begin(); it != m_Data.end(); ++it)
	{
		if (GetFileNameForLanguage(it->m_Language) == fileName)
			return true;
	}
	return false;
}

std::string Article::GetLink()
{
	return GetLinkForLanguage(m_CurrentLanguage);
}

std::string Article::GetLinkForLanguage(const std::string& language)
{
	return GetFileNameForLanguage(language) + (m_LocalPreview ? ".html" : "");
}

std::string Article::GetStandardDate()
{
	using namespace boost::gregorian;

	std::stringstream fileName;

	date_facet* facet = new date_facet();
	fileName.imbue(std::locale(std::locale::classic(), facet));
	// Sun, 19 May 2002 15:21:36 GMT
	facet->format("%a, %b %d %Y 11:00:00 GMT");
	fileName << m_CurrentData.m_Date;

	return fileName.str();
}

std::string Article::FormatContent(const std::string & articleTemplate, bool isInList, bool enableComments, const ProjectSettings& settings)
{
	std::istringstream iss(m_CurrentData.m_Content);
	std::stringstream sstr;

	bool bulletListStarted = false;
	bool hasExcerpt = false;

	std::string line;
	while (getline(iss, line))
	{
		if (line.find("***Excerpt***") != std::string::npos)
		{
			if (isInList)
			{
				hasExcerpt = true;
				break;
			}
			else
				continue;
		}
		line = ContentFactory::ReplaceImageTags(line);
		line = ContentFactory::ReplaceLinkTags(line, m_LocalPreview);

		if (bulletListStarted && !(line.length() >= 2 && line[0] == '*' && line[1] != '*'))
		{
			bulletListStarted = false;
			sstr << "</ul>" << std::endl;
		}
		if (line.length() >= 2 && line[0] == '*' && line[1] != '*')
		{
			if (!bulletListStarted)
			{
				bulletListStarted = true;
				sstr << "<ul>" << std::endl;
			}

			sstr << "<li>" << line.substr(1) << "</li>" << std::endl;
		}
		else if (line.find("**") == 0)
			sstr << "<h2>" << line.substr(2) << "</h2>" << std::endl;
		else if (line.find("<p") != std::string::npos || line.find("<ul") != std::string::npos || line.find("<li") != std::string::npos || line.find("</ul") != std::string::npos)
			sstr << line << std::endl;
		else if (line.length() > 0)
			sstr << "<p>" << line << "</p>" << std::endl;

	}

	std::string result = articleTemplate;
	std::string commentsLink = "";

	if (hasExcerpt)
		commentsLink = "<a href=\"@PWD@" + GetFileName() + (m_LocalPreview ? ".html" : "") + "\">Lire la suite...</a>";

	if (isInList)
	{
		ContentFactory::ReplaceInString(result, "<!-- m_Title -->", "<a href=\"@PWD@" + GetFileName() + (m_LocalPreview ? ".html" : "") + "\">" + m_CurrentData.m_Title + "</a>");

		if (enableComments && !hasExcerpt)
			commentsLink = "<a href=\"@PWD@" + GetFileName() + (m_LocalPreview ? ".html" : "") + "#comments\">Commentaires</a>";
	}

	ContentFactory::ReplaceInString(result, "<!-- m_Title -->", m_CurrentData.m_Title);

	ContentFactory::ReplaceInString(result, "<!-- m_CommentsLink -->", commentsLink);

	ContentFactory::ReplaceInString(result, "<!-- m_Content -->", sstr.str());

	bool isCurrentDefaultLanguage = (m_CurrentData.m_Language == settings.m_DefaultLanguage);
	bool isCurrentInsubfolder = !(isCurrentDefaultLanguage && settings.m_DefaultLanguageInRoot);

	if (m_Data.size() > 1)
	{
		std::stringstream languageLinksStream;

		languageLinksStream << "<span class=\"wrapper\">";
		for (auto it = m_Data.begin(); it != m_Data.end(); ++it)
		{
			if (it->m_Language == m_CurrentLanguage)
				continue;

			bool isDefaultLanguage = (it->m_Language == settings.m_DefaultLanguage);
			bool isInsubfolder = !(isDefaultLanguage && settings.m_DefaultLanguageInRoot);

			languageLinksStream << "<a href=\"@PWD@";
			if (m_LocalPreview)
			{
				if (isCurrentInsubfolder)
					languageLinksStream << "../";
			}

			if (isInsubfolder)
				languageLinksStream << it->m_Language << "/";

			languageLinksStream << GetFileNameForLanguage(it->m_Language) << (m_LocalPreview ? ".html" : "") << "\">" << it->m_Language << "</a>";
		}
		languageLinksStream << "</span>";

		ContentFactory::ReplaceInString(result, "<!-- m_ArticleLanguageLinks -->", languageLinksStream.str());
	}

	std::stringstream ss;

	if (!m_IsPage)
	{
		ss.width(2);
		ss.fill('0');
		ss << m_CurrentData.m_Date.day();
		ContentFactory::ReplaceInString(result, "<!-- m_Date.day -->", ss.str());

		ss.str("");
		ss.clear();
		ss.width(4);
		ss.fill('0');
		ss << m_CurrentData.m_Date.year();
		ContentFactory::ReplaceInString(result, "<!-- m_Date.year -->", ss.str());

		using namespace boost::gregorian;

		std::string months[12] = { "Janvier", "Février", "Mars", "Avril",
		  "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre",
		  "Novembre", "Décembre" };

		date_facet* german_facet = new date_facet();
		ss.imbue(std::locale(std::locale::classic(), german_facet));

		greg_month m = m_CurrentData.m_Date.month();

		german_facet->long_month_names(std::vector<std::string>(months, months + 12));

		german_facet->month_format("%B");
		ss.str("");
		ss.clear();
		ss << m;
		ContentFactory::ReplaceInString(result, "<!-- m_Date.month -->", ss.str());

		ss.str("");
		ss.clear();

		for (auto it = m_CurrentData.m_Tags.begin(); it != m_CurrentData.m_Tags.end(); ++it)
		{
			if (it != m_CurrentData.m_Tags.begin())
				ss << ", ";
			ss << *it;
		}

		ContentFactory::ReplaceInString(result, "<!-- m_Tags -->", ss.str());
	}
	return result;
}
std::string Article::FormatExcerpt()
{
	std::istringstream iss(m_CurrentData.m_Content);
	std::stringstream sstr;

	std::string line;
	while (getline(iss, line))
	{
		if (line.find("***Excerpt***") != std::string::npos)
		{
			break;
		}
		line = ContentFactory::ReplaceImageTags(line);
		line = ContentFactory::ReplaceLinkTags(line, m_LocalPreview);
		if (line.length() > 0)
			sstr << line;

	}

	std::string result = sstr.str();
	ContentFactory::ReplaceInString(result, "\n", " ");
	return result;
}

bool Article::SetLanguage(std::string language)
{
	m_HasCurrentLanguage = FindData(language, m_CurrentData);
	m_CurrentLanguage = language;
	return HasCurrentLanguage();
}

bool Article::HasCurrentLanguage()
{
	return m_HasCurrentLanguage;
}

bool Article::GetIgnore()
{
	return m_CurrentData.m_Ignore;
}

bool Article::GetHidden()
{
	return m_CurrentData.m_Hidden;
}

std::string Article::GetTitle()
{
	return m_CurrentData.m_Title;
}

const std::vector<std::string>& Article::GetTags()
{
	return m_CurrentData.m_Tags;
}

bool Article::SortByDate(const Article &lhs, const Article &rhs)
{
	return lhs.m_CurrentData.m_Date > rhs.m_CurrentData.m_Date;
}

bool Article::LoadFile()
{
	std::ifstream file(m_SourceFilePath);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << m_SourceFilePath << std::endl;
		return false;
	}
	else
	{
		if (m_SourceFilePath.find("p_") != std::string::npos)
		{
			std::cout << m_SourceFilePath << " is page." << std::endl;
			m_IsPage = true;
		}

		std::stringstream rawYamlStream;
		std::stringstream rawContentStream;

		int yamlSeparatorsEncountered = 0;
		bool draftSeparatorEncountered = false;

		std::string line;
		while (getline(file, line))
		{
			if (line.find("---") == 0)
			{
				yamlSeparatorsEncountered++;
				if (yamlSeparatorsEncountered > 1 && yamlSeparatorsEncountered % 2 == 1)
				{
					AddArticleData(rawYamlStream.str(), rawContentStream.str());

					rawYamlStream.str(std::string());
					rawYamlStream.clear();
					rawContentStream.str(std::string());
					rawContentStream.clear();

					draftSeparatorEncountered = false;
				}
				continue;
			}
			else if (line.find("***Draft***") < line.size())
			{
				draftSeparatorEncountered = true;
				continue;
			}

			if (yamlSeparatorsEncountered % 2 == 1)
				rawYamlStream << line << std::endl;
			else if (!draftSeparatorEncountered)
				rawContentStream << line << std::endl;
		}

		AddArticleData(rawYamlStream.str(), rawContentStream.str());
		return true;
	}
	return false;
}

void Article::AddArticleData(std::string rawYaml, std::string rawContent)
{
	if (rawYaml.empty() || !rawContent.empty())
	{
		ArticleData articleData;

		articleData.m_RawYaml = rawYaml;
		articleData.m_Content = rawContent;

		if (rawYaml.empty())
		{
			articleData.m_Tags.push_back("Tag1");
			boost::filesystem::path p(m_SourceFilePath);
			articleData.m_Title = p.stem().string();
		}

		m_Data.push_back(articleData);
	}
}

bool Article::SaveFile()
{
	std::ofstream file(m_SourceFilePath, std::ios_base::trunc);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << m_SourceFilePath << " for writing" << std::endl;
		return false;
	}
	else
	{
		for (auto it = m_Data.begin(); it != m_Data.end(); ++it)
		{
			YAML::Node config;

			config["m_Ignore"] = it->m_Ignore;
			config["m_Hidden"] = it->m_Hidden;
			config["m_Title"] = it->m_Title;
			config["m_Language"] = it->m_Language;

			if (!m_IsPage)
			{
				config["m_Tags"] = it->m_Tags;

				using namespace boost::gregorian;
				std::stringstream dateStream;

				date_facet* facet = new date_facet();
				dateStream.imbue (std::locale (std::locale::classic(), facet));
				facet->format("%Y-%m-%d");
				dateStream << it->m_Date;

				config["m_Date"] = dateStream.str();
			}

			file << "---" << std::endl;
			file << config << std::endl;
			file << "---" << std::endl;
			file << it->m_Content;
		}
	}
	return true;
}

bool Article::FindData(std::string language, ArticleData & data)
{
	ArticleData result;
	bool found = false;

	if (m_Data.size() > 0)
		result = m_Data[0];

	for (auto it = m_Data.begin(); it != m_Data.end(); ++it)
	{
		if (it->m_Language == language)
		{
			result = *it;
			found = true;
			break;
		}
	}

	data = result;
	return found;
}

Article::ArticleData::ArticleData() :
	m_Ignore(false),
	m_Hidden(false),
	m_Title(""),
	m_Language("en"),
	m_Date(boost::posix_time::second_clock::local_time().date()),
	m_Tags(NULL),
	m_Content("")
{

}

Article::ArticleData::~ArticleData()
{

}

bool Article::ArticleData::ParseYaml(const ArticleData& defaultValues)
{
	YAML::Node config = YAML::Load(m_RawYaml);

	if (config.size() <= 0)
		return false;

	if (config["m_Ignore"] != NULL)
		m_Ignore = config["m_Ignore"].as<bool>();
	else
		m_Ignore = defaultValues.m_Ignore;

	if (config["m_Hidden"] != NULL)
		m_Hidden = config["m_Hidden"].as<bool>();
	else
		m_Hidden = defaultValues.m_Hidden;

	if (config["m_Title"] != NULL)
		m_Title = config["m_Title"].as<std::string>();
	else
		m_Title = defaultValues.m_Title;

	if (config["m_Language"] != NULL)
		m_Language = config["m_Language"].as<std::string>();
	else
		m_Language = defaultValues.m_Language;

	if (config["m_Date"] != NULL)
	{
		std::string dateString = config["m_Date"].as<std::string>();
		m_Date = boost::gregorian::from_string(dateString);
	}
	else
		m_Date = defaultValues.m_Date;

	YAML::Node tags = config["m_Tags"];
	if (tags != NULL)
	{

		for (unsigned i = 0; i < tags.size(); i++) {
			std::string tag = tags[i].as<std::string>();
			m_Tags.push_back(tag);
		}
	}
	else
		m_Tags = defaultValues.m_Tags;

	return true;
}
