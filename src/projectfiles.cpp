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

void ProjectFiles::Configure(std::string projectPath, bool createIfMissing)
{
	m_ProjectPath = projectPath;
	m_CreateIfMissing = createIfMissing;
}

bool ProjectFiles::LoadMainTemplate()
{
	return LoadTemplate(
		kPFMainTemplateFileName,
		m_MainTemplate,
		std::bind(&ProjectFiles::GetMainTemplateDefaultContent, *this));
}

bool ProjectFiles::LoadArticleTemplate()
{
	return LoadTemplate(
		kPFArticleTemplateFileName,
		m_ArticleTemplate,
		std::bind(&ProjectFiles::GetArticleTemplateDefaultContent, *this));
}

bool ProjectFiles::LoadPageTemplate()
{
	return LoadTemplate(
		kPFPageTemplateFileName,
		m_PageTemplate,
		std::bind(&ProjectFiles::GetPageTemplateDefaultContent, *this));
}

bool ProjectFiles::LoadCommentsTemplate()
{
	return LoadTemplate(
		kPFCommentsTemplateFileName,
		m_CommentsTemplate,
		std::bind(&ProjectFiles::GetCommentsTemplateDefaultContent, *this));
}

bool ProjectFiles::CreateStyleSheetIfNecessary()
{
	boost::filesystem::path d(m_ProjectPath);
	if (!boost::filesystem::exists(d / "style.css"))
	{
		boost::filesystem::ofstream (d / "style.css");
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

bool ProjectFiles::LoadTemplate(std::string path, std::string &target, std::function<std::string()> defaultContent)
{
	target = LoadFile(path);

	if (target.empty() && m_CreateIfMissing)
	{
		target = defaultContent();

		WriteFile(path, target);
	}

	if (target.empty())
	{
		std::cerr << "Unable to open template file " << path << std::endl;
		return false;
	}

	return true;
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


std::string ProjectFiles::GetMainTemplateDefaultContent()
{
	std::stringstream ss;

	ss << "--- <b>Main template begin</b><hr>" << std::endl;
	ss << "Website name: <a href=\"@INDEX@\">@WEBSITE_NAME@</a><hr>" << std::endl;
	ss << "Title: <!-- head.m_Title --><hr>" << std::endl;
	ss << "Website description: @WEBSITE_DESCRIPTION@<hr>" << std::endl;
	ss << "Page links: <!-- pagelinks --><hr>" << std::endl;
	ss << "Tag links: <!-- taglinks --><hr>" << std::endl;
	ss << "Page list: <!-- pagelist --><hr>" << std::endl;
	ss << "Content: <!-- content --><hr>" << std::endl;
	ss << "--- <b>Main template end</b><br />" << std::endl;

	return ss.str();
}

std::string ProjectFiles::GetArticleTemplateDefaultContent()
{
	std::stringstream ss;

	ss << "--- <b>Article template begin</b><hr>" << std::endl;
	ss << "Date: <!-- m_Date.day -->&nbsp; <!-- m_Date.month -->&nbsp; <!-- m_Date.year --><hr>" << std::endl;
	ss << "Title: <!-- m_Title --><hr>" << std::endl;
	ss << "Content: <!-- m_Content --><hr>" << std::endl;
	ss << "Comments link: <!-- m_CommentsLink --><hr>" << std::endl;
	ss << "Tags: <!-- m_Tags --><hr>" << std::endl;
	ss << "--- <b>Article template end</b><br />" << std::endl;

	return ss.str();
}

std::string ProjectFiles::GetPageTemplateDefaultContent()
{
	std::stringstream ss;

	ss << "--- <b>Page template begin</b><hr>" << std::endl;
	ss << "Title: <!-- m_Title --><hr>" << std::endl;
	ss << "Content: <!-- m_Content --><hr>" << std::endl;
	ss << "Comments link: <!-- m_CommentsLink --><hr>" << std::endl;
	ss << "--- <b>Page template end</b><br />" << std::endl;

	return ss.str();
}

std::string ProjectFiles::GetCommentsTemplateDefaultContent()
{
	std::stringstream ss;

	ss << "--- <b>Comments template begin</b><hr>" << std::endl;
	ss << "--- <b>Comments template end</b><br />" << std::endl;

	return ss.str();
}
