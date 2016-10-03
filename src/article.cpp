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
	m_Ignore(false),
	m_Hidden(false),
	m_LocalPreview(true), m_IsPage(false),
	m_Title(""),
	m_Date(boost::posix_time::second_clock::local_time().date()), m_Tags(NULL),
	m_Content("")
{

}

Article::~Article()
{

}

bool Article::LoadFromFile(std::string path)
{
	m_SourceFilePath = path;
	std::string rawYaml;

	if (LoadFile(rawYaml, m_Content))
	{
		if (rawYaml.empty())
		{
			m_Tags.push_back("Tag1");
			boost::filesystem::path p(m_SourceFilePath);
			m_Title = p.stem().string();
			SaveFile();
		}
		else
		{
			YAML::Node config = YAML::Load(rawYaml);

			if (config["m_Ignore"] != NULL)
				m_Ignore = config["m_Ignore"].as<bool>();

			if (config["m_Hidden"] != NULL)
				m_Hidden = config["m_Hidden"].as<bool>();

			if (config["m_Title"] != NULL)
				m_Title = config["m_Title"].as<std::string>();

			if (config["m_Date"] != NULL)
			{
				std::string dateString = config["m_Date"].as<std::string>();
				m_Date = boost::gregorian::from_string(dateString);
			}

			YAML::Node tags = config["m_Tags"];
			if (tags != NULL)
			{

				for (unsigned i = 0; i < tags.size(); i++) {
					std::string tag = tags[i].as<std::string>();
					m_Tags.push_back(tag);
				}
			}
		}
	}

	return true;
}


std::string Article::GetSourceFilePath()
{
	return m_SourceFilePath;
}

std::string Article::Dump(bool showContent)
{
	std::stringstream result;

	result << m_Title << std::endl << m_Date << std::endl;

	for (auto it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		result << *it << " ";
	}
	result << std::endl;

	if (showContent)
		result << m_Content << std::endl;

	return result.str();
}

std::string Article::GetFileName()
{
	using namespace boost::gregorian;

	std::stringstream fileName;

	std::string title = m_Title;
	ContentFactory::SanitizeString(title);

	if (!m_IsPage)
	{
		date_facet* facet = new date_facet();
		fileName.imbue(std::locale(std::locale::classic(), facet));

		facet->format("%Y-%m-%d");
		fileName << m_Date;

		fileName << "-";
	}

	fileName << title;

	return fileName.str();
}


std::string Article::GetLink()
{
	return GetFileName() + (m_LocalPreview ? ".html" : "");
}

std::string Article::GetStandardDate()
{
	using namespace boost::gregorian;

	std::stringstream fileName;

	date_facet* facet = new date_facet();
	fileName.imbue(std::locale(std::locale::classic(), facet));
	// Sun, 19 May 2002 15:21:36 GMT
	facet->format("%a, %b %d %Y 11:00:00 GMT");
	fileName << m_Date;

	return fileName.str();
}

std::string Article::FormatContent(const std::string & articleTemplate, bool isInList, bool enableComments)
{
	std::istringstream iss(m_Content);
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
		commentsLink = "<a href=\"" + GetFileName() + (m_LocalPreview ? ".html" : "") + "\">Lire la suite...</a>";

	if (isInList)
	{
		ContentFactory::ReplaceInString(result, "<!-- m_Title -->", "<a href=\"" + GetFileName() + (m_LocalPreview ? ".html" : "") + "\">" + m_Title + "</a>");

		if (enableComments && !hasExcerpt)
			commentsLink = "<a href=\"" + GetFileName() + (m_LocalPreview ? ".html" : "") + "#comments\">Commentaires</a>";
	}

	ContentFactory::ReplaceInString(result, "<!-- m_Title -->", m_Title);

	ContentFactory::ReplaceInString(result, "<!-- m_CommentsLink -->", commentsLink);

	ContentFactory::ReplaceInString(result, "<!-- m_Content -->", sstr.str());

	std::stringstream ss;

	if (!m_IsPage)
	{
		ss.width(2);
		ss.fill('0');
		ss << m_Date.day();
		ContentFactory::ReplaceInString(result, "<!-- m_Date.day -->", ss.str());

		ss.str("");
		ss.clear();
		ss.width(4);
		ss.fill('0');
		ss << m_Date.year();
		ContentFactory::ReplaceInString(result, "<!-- m_Date.year -->", ss.str());

		using namespace boost::gregorian;

		std::string months[12] = { "Janvier", "Février", "Mars", "Avril",
		  "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre",
		  "Novembre", "Décembre" };

		date_facet* german_facet = new date_facet();
		ss.imbue(std::locale(std::locale::classic(), german_facet));

		greg_month m = m_Date.month();

		german_facet->long_month_names(std::vector<std::string>(months, months + 12));

		german_facet->month_format("%B");
		ss.str("");
		ss.clear();
		ss << m;
		ContentFactory::ReplaceInString(result, "<!-- m_Date.month -->", ss.str());

		ss.str("");
		ss.clear();

		for (auto it = m_Tags.begin(); it != m_Tags.end(); ++it)
		{
			if (it != m_Tags.begin())
				ss << ", ";
			ss << *it;
		}

		ContentFactory::ReplaceInString(result, "<!-- m_Tags -->", ss.str());
	}
	return result;
}
std::string Article::FormatExcerpt()
{
	std::istringstream iss(m_Content);
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

bool Article::SortByDate(const Article &lhs, const Article &rhs)
{
	return lhs.m_Date > rhs.m_Date;
}

bool Article::LoadFile(std::string& rawYaml, std::string& rawContent)
{
	rawYaml = "";
	rawContent = "";
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

		std::string line;
		while (getline(file, line))
		{
			if (line.find("---") < line.size())
			{
				yamlSeparatorsEncountered++;
				continue;
			}
			else if (line.find("***Draft***") < line.size())
				break;

			if (yamlSeparatorsEncountered == 1)
				rawYamlStream << line << std::endl;
			else
				rawContentStream << line << std::endl;
		}

		rawYaml = rawYamlStream.str();
		rawContent = rawContentStream.str();
		return true;
	}
	return false;
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
		YAML::Node config;

		config["m_Ignore"] = m_Ignore;
		config["m_Hidden"] = m_Hidden;
		config["m_Title"] = m_Title;

		if (!m_IsPage)
		{
			config["m_Tags"] = m_Tags;

			using namespace boost::gregorian;
			std::stringstream dateStream;

			date_facet* facet = new date_facet();
			dateStream.imbue (std::locale (std::locale::classic(), facet));
			facet->format("%Y-%m-%d");
			dateStream << m_Date;

			config["m_Date"] = dateStream.str();
		}

		file << "---" << std::endl;
		file << config << std::endl;
		file << "---" << std::endl;
		file << m_Content;
	}

	return true;
}
