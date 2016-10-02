#include "mown.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>

#include <boost/filesystem.hpp>

#include "contentfactory.h"
#include "article.h"

Mown::Mown()
{
}

Mown::~Mown()
{

}

bool Mown::Export(const std::string path)
{
	Cleanup();

	boost::filesystem::path sourcePath(path);

	std::cout << "Source: " << sourcePath << std::endl;

	m_Articles.clear();
	m_Pages.clear();

	if (boost::filesystem::exists(sourcePath)
		&& boost::filesystem::is_directory(sourcePath))
	{

		if (!LoadConfig(path))
			return false;

		boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
		for (boost::filesystem::directory_iterator itr(sourcePath);
		itr != end_itr;
			++itr)
		{
			if (boost::filesystem::is_regular_file(itr->status()))
			{
				boost::filesystem::path filePath = itr->path();
				if (filePath.extension().string() == ".yaml" && filePath.filename() != "_settings.yaml")
				{
					Article article;
					article.LoadFromFile(filePath.string());

					article.m_LocalPreview = m_LocalPreview;

					if (article.m_IsPage)
					{
						if (m_ForceAll || !article.m_Ignore)
							m_Pages.push_back(article);
					}
					else
						m_Articles.push_back(article);
				}
			}
		}

		std::sort(m_Articles.begin(), m_Articles.end(), Article::SortByDate);

		boost::filesystem::path exportFolder = sourcePath / "mown-preview";

		if (!m_LocalPreview)
			exportFolder = sourcePath / "mown-export";


		std::cout << "Will export to " << exportFolder << std::endl;

		if (boost::filesystem::exists(exportFolder))
		{
			if (boost::filesystem::is_directory(exportFolder))
				EmptyFolder(exportFolder.string());
			else
			{
				boost::filesystem::remove(exportFolder);
				boost::filesystem::create_directory(exportFolder);
			}
		}
		else
		{
			boost::filesystem::create_directory(exportFolder);
		}

		SetupExportFolder(sourcePath.string(), exportFolder.string());

		m_LocalUrl = "file:///" + exportFolder.string() + "/index.html";
		ContentFactory::ReplaceInString(m_LocalUrl, "\\", "/");

		std::stringstream rssFileContent;
		rssFileContent << "<?xml version=\"1.0\"?>" << std::endl << "<rss version=\"2.0\">" << std::endl;
		rssFileContent << "  <channel>" << std::endl << "    <title>" << m_Settings.m_WebsiteName << "</title>" << std::endl << "    <link>" << m_Settings.m_Url << "</link>" << std::endl << "    <description>Pensées du moment et trucs que j'ai fait</description>" << std::endl;
		rssFileContent << "    <lastBuildDate>" << m_Articles[0].GetStandardDate() << "</lastBuildDate>" << std::endl;

		for (auto it = m_Articles.begin();
		it != m_Articles.end();
			++it)
		{
			if (m_ForceAll || it->m_Ignore == false)
			{
				if (!it->m_Hidden)
				{
					AddArticleToTag("Tous les billets", *it);

					for (auto itTag = it->m_Tags.begin();
					itTag != it->m_Tags.end();
						++itTag)
						AddArticleToTag(*itTag, *it);

					rssFileContent << "    <item>" << std::endl;
					rssFileContent << "       <title>" << it->m_Title << "</title>" << std::endl;
					rssFileContent << "       <link>" << m_Settings.m_Url << it->GetLink() << "</link>" << std::endl;
					rssFileContent << "       <description>" << it->FormatExcerpt() << "</description>" << std::endl;
					rssFileContent << "       <pubDate>" << it->GetStandardDate() << "</pubDate>" << std::endl;
					rssFileContent << "    </item>" << std::endl;
				}
				boost::filesystem::path file = exportFolder / it->GetFileName();
				file.replace_extension(".html");

				std::string fileContent = m_ProjectFiles.GetMainTemplate();
				std::string formatedArticle = it->FormatContent(m_ProjectFiles.GetArticleTemplate(), false, m_EnableComments);

				if (m_EnableComments && it->m_Ignore == false)
					formatedArticle += m_ProjectFiles.GetCommentsTemplate();

				ContentFactory::ReplaceInString(fileContent, "<!-- content -->", formatedArticle);
				ContentFactory::ReplaceInString(fileContent, "<!-- head.m_Title -->", " - " + it->m_Title);
				ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteName -->", m_Settings.m_WebsiteName);
				ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteDescription -->", m_Settings.m_WebsiteDescription);


				std::ofstream fout(file.string());
				if (fout.is_open())
				{
					fout << fileContent;
					fout.close();
				}

				//std::cout << it->Dump(false) << std::endl;

			}
		}


		rssFileContent << "  </channel>" << std::endl << "</rss>" << std::endl;

		//std::cout << rssFileContent.str() << std::endl;
		boost::filesystem::path rssFile = exportFolder / "rss.xml";
		std::ofstream rssFout(rssFile.string());
		if (rssFout.is_open())
		{
			rssFout << rssFileContent.str();
			rssFout.close();
		}

		for (auto it = m_Tags.begin(); it != m_Tags.end(); ++it)
			m_SortedTags.push_back(it->second);

		std::sort(m_SortedTags.begin(), m_SortedTags.end(), ArticleTag::SortByTitle);

		for (auto it = m_SortedTags.begin(); it != m_SortedTags.end(); ++it)
		{
			std::string tagLinks = GenerateTagLinks (it->m_Name);
			std::string pageLinks = GeneratePageLinks (it->m_Name);
			for (int i = 0; i < it->GetPageCount(); i++)
			{
				std::string fileContent = it->FormatArticleListPage(i, m_ProjectFiles.GetMainTemplate(), m_ProjectFiles.GetArticleTemplate(), tagLinks, pageLinks);
				ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteName -->", m_Settings.m_WebsiteName);
				ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteDescription -->", m_Settings.m_WebsiteDescription);

				boost::filesystem::path file = exportFolder / it->GetFileNameForPage(i);
				std::ofstream fout(file.string());
				if (fout.is_open())
				{
					fout << fileContent;
					fout.close();
				}
			}
		}


		for (auto it = m_Pages.begin();
		it != m_Pages.end();
			++it)
		{
			boost::filesystem::path file = exportFolder / it->GetFileName();
			file.replace_extension(".html");

			std::string fileContent = m_ProjectFiles.GetMainTemplate();
			std::string formatedArticle = it->FormatContent(m_ProjectFiles.GetPageTemplate(), false, m_EnableComments);

			if (m_EnableComments && it->m_Ignore == false)
				formatedArticle += m_ProjectFiles.GetCommentsTemplate();

			ContentFactory::ReplaceInString(fileContent, "<!-- content -->", formatedArticle);
			ContentFactory::ReplaceInString(fileContent, "<!-- head.m_Title -->", " - " + it->m_Title);
			ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteName -->", m_Settings.m_WebsiteName);
			ContentFactory::ReplaceInString(fileContent, "<!-- head.m_WebsiteDescription -->", m_Settings.m_WebsiteDescription);

			ContentFactory::ReplaceInString(fileContent, "<!-- pagelinks -->", GeneratePageLinks(it->m_Title));

			std::ofstream fout(file.string());
			if (fout.is_open())
			{
				fout << fileContent;
				fout.close();
			}

			//std::cout << it->Dump(false) << std::endl;
		}

	}
	else
	{
		std::cerr << "Unable to open folder" << std::endl;
		return false;
	}

	return true;
}

void Mown::Dump(bool summary)
{
	std::cout << "Tags: ";
	for (auto it = m_Tags.begin(); it != m_Tags.end(); ++it)
	{
		std::cout << it->second.m_Name << " (" << it->second.m_Articles.size() << ") ";
	}
	std::cout << std::endl;
}

std::string Mown::GetSourceFilenameForPreviewFile(std::string previewFile)
{
	std::string result = "";

	size_t extensionPosition = previewFile.rfind(".html");
	if (extensionPosition != std::string::npos)
	{
		previewFile = previewFile.substr(0, extensionPosition);
	}

	for (auto it = m_Articles.begin(); it != m_Articles.end(); ++it)
	{
		if (it->GetFileName() == previewFile)
		{
			result = it->GetSourceFilePath();
			break;
		}
	}

	for (auto it = m_Pages.begin(); it != m_Pages.end(); ++it)
	{
		if (it->GetFileName() == previewFile)
		{
			result = it->GetSourceFilePath();
			break;
		}
	}

	return result;
}

std::string Mown::GenerateTagLinks(std::string currentTag)
{
	std::stringstream ss;

	ss << "<span class=\"wrapper\">";
	for (auto it = m_SortedTags.begin(); it != m_SortedTags.end(); ++it)
	{
		if (it->m_Name == currentTag)
			ss << "<span class=\"current_tag\">" << it->GetPrettyName () << "</span> ";
		else
			ss << "<a href=\"" << it->GetLinkForPage (0) << "\">" << it->GetPrettyName () << "</a> ";
	}
	ss << "</span>";

	return ss.str();
}

std::string Mown::GeneratePageLinks(std::string currentPage)
{
	std::stringstream ss;

	ss << "<span class=\"wrapper\">";
	for (auto it = m_Pages.begin(); it != m_Pages.end(); ++it)
	{
		if (it->m_Title == currentPage)
			ss << "<span class=\"current_page\">" << it->m_Title << "</span> ";
		else
			ss << "<a href=\"" << it->GetLink () << "\">" << it->m_Title << "</a> ";
	}
	ss << "</span>";

	return ss.str();
}

void Mown::Cleanup()
{
	m_Pages.clear();
	m_Articles.clear();
	m_Tags.clear();
	m_SortedTags.clear();
	m_Settings.SetDefaultValues();
}

void Mown::AddArticleToTag(std::string tagName, Article article)
{
	ArticleTag tag;
	if (m_Tags.find(tagName) == m_Tags.end())
	{
		tag.m_Name = tagName;
		if (tagName == "Tous les billets")
			tag.m_IsIndex = true;
		else
			tag.m_IsIndex = false;

		tag.m_LocalPreview = m_LocalPreview;
		tag.m_WebsiteRoot = m_WebsiteRoot;
		tag.m_EnableComments = m_EnableComments;

		m_Tags[tagName] = tag;
	}

	m_Tags[tagName].m_Articles.push_back(article);
}

void Mown::EmptyFolder(std::string path)
{
	boost::filesystem::path sourcePath(path);
	if (boost::filesystem::exists(sourcePath)
		&& boost::filesystem::is_directory(sourcePath))
	{
		boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
		for (boost::filesystem::directory_iterator itr(sourcePath);
		itr != end_itr;
			++itr)
		{
			if (boost::filesystem::is_directory(itr->status()))
				boost::filesystem::remove_all(itr->path());
			else
				boost::filesystem::remove(itr->path());
		}
	}
}

bool Mown::CopyDirectory(
	boost::filesystem::path const & source,
	boost::filesystem::path const & destination
	)
{
	namespace fs = boost::filesystem;
	try
	{
		// Check whether the function call is valid
		if (
			!fs::exists(source) ||
			!fs::is_directory(source)
			)
		{
			std::cerr << "Source directory " << source.string()
				<< " does not exist or is not a directory." << '\n'
				;
			return false;
		}
		if (fs::exists(destination))
		{
			std::cerr << "Destination directory " << destination.string()
				<< " already exists." << '\n'
				;
			return false;
		}
		// Create the destination directory
		if (!fs::create_directory(destination))
		{
			std::cerr << "Unable to create destination directory"
				<< destination.string() << '\n'
				;
			return false;
		}
	}
	catch (fs::filesystem_error const & e)
	{
		std::cerr << e.what() << '\n';
		return false;
	}
	// Iterate through the source directory
	for (
		fs::directory_iterator file(source);
		file != fs::directory_iterator(); ++file
		)
	{
		try
		{
			fs::path current(file->path());
			if (fs::is_directory(current))
			{
				// Found directory: Recursion
				if (
					!CopyDirectory(
						current,
						destination / current.filename()
						)
					)
				{
					return false;
				}
			}
			else
			{
				// Found file: Copy
				fs::copy_file(
					current,
					destination / current.filename()
					);
			}
		}
		catch (fs::filesystem_error const & e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	return true;
}

std::string Mown::GetLocalUrl()
{
	return m_LocalUrl;
}

bool Mown::LoadConfig(std::string path)
{
	m_ProjectFiles.SetProjectPath(path);

	std::string filePath;
	boost::filesystem::path d(path);

	filePath = (d / "_settings.yaml").string();
	if (!m_Settings.LoadFromFile(filePath) && !m_Settings.SaveToFile(filePath))
		std::cerr << "Unable to load or create settings file " << (d / "_settings.yaml").string() << std::endl;

	bool autoCreateFiles = true;

	if (!m_ProjectFiles.LoadArticleTemplate(autoCreateFiles))
		return false;

	if (!m_ProjectFiles.LoadPageTemplate(autoCreateFiles))
		return false;

	if (!m_ProjectFiles.LoadMainTemplate(autoCreateFiles, (m_LocalPreview ? "index.html" : m_WebsiteRoot)))
		return false;

	if (!m_ProjectFiles.LoadCommentsTemplate(autoCreateFiles))
		return false;

	return true;
}

bool Mown::SetupExportFolder(std::string sourceFolder, std::string targetFolder)
{
	boost::filesystem::path from(sourceFolder);
	boost::filesystem::path to(targetFolder);

	boost::filesystem::path styleSheet = from / "style.css";
	if (boost::filesystem::is_regular_file(styleSheet))
		boost::filesystem::copy(styleSheet, to / "style.css");

	boost::filesystem::path imageFolder = from / "images";
	if (boost::filesystem::is_directory(imageFolder))
	{
		CopyDirectory(imageFolder, to / "images");
	}



	return true;
}
