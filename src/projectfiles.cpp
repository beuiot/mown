#include <boost/filesystem.hpp>
#include "contentfactory.h"

#include "projectfiles.h"

#include <iostream>
#include <sstream>
#include <fstream>

ProjectFiles::ProjectFiles()
{
}

ProjectFiles::~ProjectFiles()
{

}

void ProjectFiles::SetProjectPath(std::string projectPath)
{
	m_ProjectPath = projectPath;
}

bool ProjectFiles::LoadMainTemplate(bool createIfMissing, std::string websiteRoot)
{
	m_MainTemplate = LoadFile("_template.html");

	if (m_MainTemplate.empty() && createIfMissing)
	{
		std::stringstream ss;

		ss << "--- <b>Main template begin</b><hr>" << std::endl;
		ss << "Website name: <a href=\"/\"><!-- head.m_WebsiteName --></a><hr>" << std::endl;
		ss << "Title: <!-- head.m_Title --><hr>" << std::endl;
		ss << "Website description: <!-- head.m_WebsiteDescription --><hr>" << std::endl;
		ss << "Page links: <!-- pagelinks --><hr>" << std::endl;
		ss << "Tag links: <!-- taglinks --><hr>" << std::endl;
		ss << "Page list: <!-- pagelist --><hr>" << std::endl;
		ss << "Content: <!-- content --><hr>" << std::endl;
		ss << "--- <b>Main template end</b><br />" << std::endl;

		m_MainTemplate = ss.str();

		WriteFile("_template.html", m_MainTemplate);
	}

	if (m_MainTemplate.empty())
	{
		std::cerr << "Unable to open template file " << "_template.html" << std::endl;
		return false;
	}

	ContentFactory::ReplaceInString(m_MainTemplate, "\"/\"", "\"" + websiteRoot + "\"");

	return true;
}

bool ProjectFiles::LoadArticleTemplate(bool createIfMissing)
{
	m_ArticleTemplate = LoadFile("_article_template.html");

	if(m_ArticleTemplate.empty() && createIfMissing)
	{
		std::stringstream ss;

		ss << "--- <b>Article template begin</b><hr>" << std::endl;
		ss << "Date: <!-- m_Date.day -->&nbsp; <!-- m_Date.month -->&nbsp; <!-- m_Date.year --><hr>" << std::endl;
		ss << "Title: <!-- m_Title --><hr>" << std::endl;
		ss << "Content: <!-- m_Content --><hr>" << std::endl;
		ss << "Comments link: <!-- m_CommentsLink --><hr>" << std::endl;
		ss << "Tags: <!-- m_Tags --><hr>" << std::endl;
		ss << "--- <b>Article template end</b><br />" << std::endl;

		m_ArticleTemplate = ss.str();

		WriteFile("_article_template.html", m_ArticleTemplate);
	}

	if (m_ArticleTemplate.empty())
	{
		std::cerr << "Unable to open template file " << "_article_template.html" << std::endl;
		return false;
	}

	return true;
}

bool ProjectFiles::LoadPageTemplate(bool createIfMissing)
{
	m_PageTemplate = LoadFile("_page_template.html");

	if (m_PageTemplate.empty() && createIfMissing)
	{
		std::stringstream ss;

		ss << "--- <b>Page template begin</b><hr>" << std::endl;
		ss << "Title: <!-- m_Title --><hr>" << std::endl;
		ss << "Comments link: <!-- m_CommentsLink --><hr>" << std::endl;
		ss << "--- <b>Page template end</b><br />" << std::endl;

		m_PageTemplate = ss.str();

		WriteFile("_page_template.html", m_PageTemplate);
	}

	if (m_PageTemplate.empty())
	{
		std::cerr << "Unable to open template file " << "_page_template.html" << std::endl;
		return false;
	}

	return true;
}

bool ProjectFiles::LoadCommentsTemplate(bool createIfMissing)
{
	m_CommentsTemplate = LoadFile("_comments_template.html");

	if (m_CommentsTemplate.empty() && createIfMissing)
	{
		std::stringstream ss;

		ss << "--- <b>Comments template begin</b><hr>" << std::endl;
		ss << "--- <b>Comments template end</b><br />" << std::endl;

		m_CommentsTemplate = ss.str();

		WriteFile("_comments_template.html", m_CommentsTemplate);
	}

	if (m_CommentsTemplate.empty())
	{
		std::cerr << "Unable to open template file " << "_comments_template.html" << std::endl;
		return false;
	}

	return true;
}

std::string ProjectFiles::GetMainTemplate()
{
	return m_MainTemplate;
}

std::string ProjectFiles::GetArticleTemplate()
{
	return m_ArticleTemplate;
}

std::string ProjectFiles::GetPageTemplate()
{
	return m_PageTemplate;
}

std::string ProjectFiles::GetCommentsTemplate()
{
	return m_CommentsTemplate;
}

std::string ProjectFiles::LoadFile(std::string fileName)
{
	boost::filesystem::path d(m_ProjectPath);
	std::ifstream fin;

	fin.open((d / fileName).string());
	if (fin.is_open())
	{
		std::stringstream buffer;
		buffer << fin.rdbuf();
		fin.close();
		return buffer.str();
	}

	return std::string();
}

void ProjectFiles::WriteFile(std::string fileName, std::string content)
{
	boost::filesystem::path d(m_ProjectPath);

	std::ofstream file((d / fileName).string(), std::ios_base::trunc);

	if (!file.is_open())
	{
		std::cerr << "Could not open " << fileName << " for writing" << std::endl;
	}
	else
	{
		file << content;
	}
}
