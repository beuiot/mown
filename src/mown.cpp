#include "mown.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>

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

		boost::filesystem::path exportFolder = sourcePath / (m_LocalPreview ? "mown-preview" : "mown-export");

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

		boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
		for (boost::filesystem::directory_iterator itr(sourcePath);
		itr != end_itr;
			++itr)
		{
			if (boost::filesystem::is_regular_file(itr->status()))
			{
				boost::filesystem::path filePath = itr->path();
				if (filePath.extension().string() == ".yaml" && filePath.filename() != "_settings.yaml" && filePath.filename() != "_localization.yaml")
				{
					Article article;
					article.LoadFromFile(filePath.string(), m_Languages);

					article.m_LocalPreview = m_LocalPreview;

					if (article.m_IsPage)
						m_Pages.push_back(article);
					else
						m_Articles.push_back(article);

					std::string stylesheet = article.GetStylesheet();
					if (!stylesheet.empty())
					{
						boost::filesystem::copy(sourcePath / stylesheet, exportFolder / stylesheet);
					}
				}
			}
		}

		std::sort(m_Articles.begin(), m_Articles.end(), Article::SortByDate);

		SetupExportFolder(sourcePath.string(), exportFolder.string());

		m_LocalUrl = "file:///" + exportFolder.string() + "/index." + m_Settings.m_FileExtention;
		ContentFactory::ReplaceInString(m_LocalUrl, "\\", "/");

		for (auto it = m_Languages.begin(); it != m_Languages.end(); ++it)
		{
			ExportLanguage(*it, exportFolder);
		}
	}
	else
	{
		std::cerr << "Unable to open folder" << std::endl;
		return false;
	}

	return true;
}

bool Mown::PathHasNewContent(std::string path)
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
			if (boost::filesystem::is_regular_file(itr->status()))
			{
				boost::filesystem::path filePath = itr->path();
				if (filePath.extension().string() == ".yaml" && filePath.filename() != "_settings.yaml" && filePath.filename() != "_localization.yaml")
				{
					std::string filePathStr = filePath.string();

					bool found = false;

					for (auto it = m_Articles.begin(); it != m_Articles.end(); ++it)
					{
						if (it->GetSourceFilePath() == filePathStr)
						{
							found = true;
							break;
						}
					}

					for (auto it = m_Pages.begin(); it != m_Pages.end(); ++it)
					{
						if(it->GetSourceFilePath() == filePathStr)
						{
							found = true;
							break;
						}
					}

					if (!found)
						return true;
				}
			}
		}
	}
	return false;
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

const Article& Mown::GetArticleForPreviewFile(std::string previewFile)
{
	std::string language = "";

	size_t extensionPosition = previewFile.rfind("." + m_Settings.m_FileExtention);
	if (extensionPosition != std::string::npos)
	{
		previewFile = previewFile.substr(0, extensionPosition);
	}

	for (auto it = m_Articles.begin(); it != m_Articles.end(); ++it)
	{
		if (it->HasFileName(previewFile, language))
		{
			it->SetLanguage(language);
			return *it;
			break;
		}
	}

	for (auto it = m_Pages.begin(); it != m_Pages.end(); ++it)
	{
		if (it->HasFileName(previewFile, language))
		{
			it->SetLanguage(language);
			return *it;
			break;
		}
	}

	throw std::invalid_argument("Article not found for preview file");
}

std::string Mown::GenerateTagLinks(const std::string& currentTag, const std::string& language, const ProjectSettings& settings)
{
	std::stringstream ss;

	ss << "<span class=\"wrapper\">";
	for (auto it = m_SortedTags.begin(); it != m_SortedTags.end(); ++it)
	{
		if (it->m_Name == currentTag)
			ss << "<span class=\"current_tag\">" << it->GetPrettyName() << "</span> ";
		else
			ss << "<a href=\"@PWD@" << it->GetLinkForPage(0, settings) << "\">" << it->GetPrettyName() << "</a> ";
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
		if (!it->HasCurrentLanguage()
			|| (!m_ForceAll && it->GetIgnore())
			|| it->GetHidden())
			continue;

		if (it->GetTitle() == currentPage)
			ss << "<span class=\"current_page " << it->GetFileNameForLanguage(m_Settings.m_DefaultLanguage) << "\"><span class=\"page_title\">" << it->GetTitle() << "</span></span> ";
		else
		{
			ss << "<span class=\"page_link " << it->GetFileNameForLanguage(m_Settings.m_DefaultLanguage) << "\"><a href=\"";
			if (!it->IsExternallink())
				ss << "@PWD@";
			ss << it->GetLink(m_Settings) << "\"";

			if (it->IsExternallink())
				ss << " target=\"_blank\"";

			ss << "><span class=\"page_title\">" << it->GetTitle() << "</span></a></span>";
		}
	}
	ss << "</span>";

	return ss.str();
}

std::string Mown::GenerateLanguageLinks(std::string currentLanguage)
{
	std::stringstream ss;

	ss << "<span class=\"wrapper\">";
	for (auto it = m_Languages.begin(); it != m_Languages.end(); ++it)
	{
		if (*it != currentLanguage)
		{
			ss << "<a href=\"";

			if (m_Settings.m_DefaultLanguageInRoot && *it == m_Settings.m_DefaultLanguage)
				ss << (m_LocalPreview ? "../" : "") << "@INDEX@";
			else
				ss << "@ROOT@" << *it << (m_LocalPreview ? "/index." + m_Settings.m_FileExtention : "");

			ss << "\">" << *it << "</a> ";
		}
	}
	ss << "</span>";

	return ss.str();
}

void Mown::Cleanup()
{
	m_Pages.clear();
	m_Articles.clear();
	m_Tags.clear();
	m_Languages.clear();
	m_SortedTags.clear();
	m_Settings.SetDefaultValues();
	m_Localization.Clear();
}

void Mown::ExportLanguage(std::string language, boost::filesystem::path folder)
{
	m_Tags.clear();
	m_SortedTags.clear();

	bool exportInSubfolder = m_Languages.size() > 1 && !(m_Settings.m_DefaultLanguageInRoot && language == m_Settings.m_DefaultLanguage);
	bool exportRootIndex = exportInSubfolder && language == m_Settings.m_DefaultLanguage;

	int directoryDepth = exportInSubfolder ? 1 : 0;

	std::string subFolder = m_LocalPreview ? "" : m_WebsiteRoot;
	if (exportInSubfolder && !m_LocalPreview)
		subFolder += language + "/";

	boost::filesystem::path exportFolder = exportInSubfolder ? folder / language : folder;

	if (!boost::filesystem::exists(exportFolder))
		boost::filesystem::create_directory(exportFolder);

	std::stringstream rssFileContent;
	rssFileContent << "<?xml version=\"1.0\"?>" << std::endl << "<rss version=\"2.0\">" << std::endl;
	rssFileContent << "  <channel>" << std::endl << "    <title>@WEBSITE_NAME@</title>" << std::endl << "    <link>" << m_Settings.m_Url << "</link>" << std::endl << "    <description>Pensées du moment et trucs que j'ai fait</description>" << std::endl;
	if (m_Articles.size() > 0)
	{
		rssFileContent << "    <lastBuildDate>" << m_Articles[0].GetStandardDate() << "</lastBuildDate>" << std::endl;
	}

	bool hasHomePage = false;
	Article homePage;

	for (auto it = m_Pages.begin();
	it != m_Pages.end();
		++it)
	{
		it->SetLanguage(language);

		if (it->GetIsHomepage())
		{
			hasHomePage = true;
			homePage = *it;
		}
	}

	std::sort(m_Pages.begin(), m_Pages.end(), Article::SortByOrder);

	CreateTag("Tous les billets");

	for (auto it = m_Articles.begin();
	it != m_Articles.end();
		++it)
	{
		std::string mainUrl = "";
		if (it->SetLanguage(m_Settings.m_DefaultLanguage))
		{
			mainUrl = m_Settings.m_Url + "/" + (m_Settings.m_DefaultLanguageInRoot ? "" : (m_Settings.m_DefaultLanguage + "/")) + it->GetLink(m_Settings);
		}

		bool hasWantedLanguage = it->SetLanguage(language);
		if (!hasWantedLanguage)
			continue;

		if (m_ForceAll || it->GetIgnore() == false)
		{
			std::string url = m_Settings.m_Url + (m_LocalPreview ? "/" : "") + "@PWD@" + it->GetLink(m_Settings);
			if (mainUrl.empty())
				mainUrl = url;

			if (!it->GetHidden())
			{
				AddArticleToTag("Tous les billets", *it);

				for (auto itTag = it->GetTags().begin();
				itTag != it->GetTags().end();
					++itTag)
					AddArticleToTag(*itTag, *it);

				rssFileContent << "    <item>" << std::endl;
				rssFileContent << "       <title>" << it->GetTitle() << "</title>" << std::endl;
				rssFileContent << "       <link>" << url << "</link>" << std::endl;
				rssFileContent << "       <description>" << it->FormatExcerpt() << "</description>" << std::endl;
				rssFileContent << "       <pubDate>" << it->GetStandardDate() << "</pubDate>" << std::endl;
				rssFileContent << "    </item>" << std::endl;
			}
			boost::filesystem::path file = exportFolder / it->GetFileName();
			file.replace_extension("." + m_Settings.m_FileExtention);

			std::string fileContent = m_ProjectFiles.GetMainTemplate();
			std::string formatedArticle = it->FormatContent(m_ProjectFiles.GetArticleTemplate(), false, m_EnableComments, m_Settings);

			if (m_EnableComments && it->GetIgnore() == false)
				formatedArticle += m_ProjectFiles.GetCommentsTemplate();

			ContentFactory::ReplaceInString(fileContent, "<!-- content -->", formatedArticle);
			ContentFactory::ReplaceInString(fileContent, "<!-- head.m_Title -->", " - " + it->GetTitle());

			PostProcessContent(fileContent, directoryDepth, subFolder, language, url, mainUrl, *it);

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

	std::string fileContent = rssFileContent.str();
	PostProcessContent(fileContent, directoryDepth, subFolder, language, "", "", Article());

	if (m_Articles.size() > 0)
	{
		std::string rssFileName = "rss";
		if (exportInSubfolder && !exportRootIndex)
			rssFileName += "_" + language;
		rssFileName += ".xml";
		boost::filesystem::path rssFile = folder / rssFileName;

		std::ofstream rssFout(rssFile.string());
		if (rssFout.is_open())
		{
			rssFout << fileContent;
			rssFout.close();
		}
	}

	for (auto it = m_Tags.begin(); it != m_Tags.end(); ++it)
		m_SortedTags.push_back(it->second);

	std::sort(m_SortedTags.begin(), m_SortedTags.end(), ArticleTag::SortByTitle);

	for (auto it = m_SortedTags.begin(); it != m_SortedTags.end(); ++it)
	{
		it->SetLanguage(m_Localization, language);

		std::string tagLinks = GenerateTagLinks(it->m_Name, language, m_Settings);
		std::string pageLinks = GeneratePageLinks(it->m_Name);
		std::string languageLinks = GenerateLanguageLinks(language);
		for (int i = 0; i < it->GetPageCount(); i++)
		{
			std::string fileContent = it->FormatArticleListPage(i, m_ProjectFiles.GetMainTemplate(), m_ProjectFiles.GetArticleTemplate(), tagLinks, pageLinks, languageLinks, m_Settings);

			if (exportRootIndex && i == 0 && it->m_IsIndex)
			{
				std::string upDirectoryFileContent = fileContent;
				PostProcessContent(upDirectoryFileContent, 0, m_LocalPreview ? language + "/" : subFolder, language, "", "", Article());

				boost::filesystem::path file = folder / it->GetFileNameForPage(i, m_Settings);
				std::ofstream fout(file.string());
				if (fout.is_open())
				{
					fout << upDirectoryFileContent;
					fout.close();
				}
			}

			PostProcessContent(fileContent, directoryDepth, subFolder, language, "", "", Article());

			boost::filesystem::path file = exportFolder / it->GetFileNameForPage(i, m_Settings);
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

		std::string mainUrl = "";
		if (it->SetLanguage(m_Settings.m_DefaultLanguage))
		{
			mainUrl = m_Settings.m_Url + "/" + (m_Settings.m_DefaultLanguageInRoot ? "" : (m_Settings.m_DefaultLanguage + "/")) + it->GetLink(m_Settings);
		}

		it->SetLanguage(language);
		if (!it->HasCurrentLanguage()
			|| (!m_ForceAll && it->GetIgnore())
			|| it->IsExternallink())
			continue;

		boost::filesystem::path file = exportFolder / it->GetFileName();
		file.replace_extension("." + m_Settings.m_FileExtention);
		std::string url = m_Settings.m_Url + (m_LocalPreview ? "/" : "") + "@PWD@" + it->GetLink(m_Settings);
		if (mainUrl.empty())
			mainUrl = url;

		std::string fileContent = m_ProjectFiles.GetMainTemplate();
		std::string formatedArticle = it->FormatContent(m_ProjectFiles.GetPageTemplate(), false, m_EnableComments, m_Settings);

		if (m_EnableComments && it->GetIgnore() == false)
			formatedArticle += m_ProjectFiles.GetCommentsTemplate();

		ContentFactory::ReplaceInString(fileContent, "<!-- content -->", formatedArticle);
		ContentFactory::ReplaceInString(fileContent, "<!-- head.m_Title -->", " - " + it->GetTitle());

		ContentFactory::ReplaceInString(fileContent, "<!-- pagelinks -->", GeneratePageLinks(it->GetTitle()));
		ContentFactory::ReplaceInString(fileContent, "<!-- languagelinks -->", GenerateLanguageLinks(language));

		if (exportRootIndex && it->GetIsHomepage())
		{
			std::string upDirectoryFileContent = fileContent;
			PostProcessContent(upDirectoryFileContent, 0, m_LocalPreview ? language + "/" : subFolder, language, "", "", *it);

			boost::filesystem::path file = folder / ("index." + m_Settings.m_FileExtention);
			std::ofstream fout(file.string());
			if (fout.is_open())
			{
				fout << upDirectoryFileContent;
				fout.close();
			}
		}

		PostProcessContent(fileContent, directoryDepth, subFolder, language, url, mainUrl, *it);

		std::ofstream fout(file.string());
		if (fout.is_open())
		{
			fout << fileContent;
			fout.close();
		}

		if (it->GetIsHomepage())
		{
			boost::filesystem::path fileIndex = exportFolder / ("index." + m_Settings.m_FileExtention);
			std::ofstream foutIndex(fileIndex.string());
			if (foutIndex.is_open())
			{
				foutIndex << fileContent;
				foutIndex.close();
			}
		}

		//std::cout << it->Dump(false) << std::endl;
	}

	if (m_Languages.size() > 1 && language == m_Settings.m_DefaultLanguage)
	{
		boost::filesystem::path file = folder / ".htaccess";
		std::ofstream fout(file.string());
		if (fout.is_open())
		{
			fout << m_ProjectFiles.GetHtaccessFile(m_Settings.m_DefaultLanguage, exportInSubfolder, m_Languages);
		}
	}
}

void Mown::PostProcessContent(std::string& content, int directoryDepth, const std::string& subFolder, const std::string& language, const std::string& url, const std::string& mainUrl, const Article& article)
{
	content = ContentFactory::ReplaceImageTags(content);

	std::string resourcesPath = "";
	if (m_LocalPreview)
	{
		for (int i = 0; i < directoryDepth; i++)
			resourcesPath += "../";
	}
	else
		resourcesPath = m_WebsiteRoot;

	std::string index = m_LocalPreview ? "@ROOT@" : m_WebsiteRoot;

	if (!(language == m_Settings.m_DefaultLanguage && m_Settings.m_DefaultLanguageInRoot))
		index += language + "/";

	index += "@INDEXFILE@";

	content = ContentFactory::ReplaceMownLinkTags(content, std::bind(&Mown::getArticleLink, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	std::string stylesheet = "";
	if (!article.GetStylesheet().empty())
		stylesheet = "<link rel=\"stylesheet\" type=\"text/css\" href=\"@ROOT@" + article.GetStylesheet() + "\" />";
	ContentFactory::ReplaceInString(content, "<!-- stylesheets -->", stylesheet);

	std::string footer = "";
	const std::string id = "FOOTER_" + article.GetTitle();

	if (!m_Localization.GetLocalizedStringFromId(id, footer, language))
		m_Localization.GetLocalizedStringFromId(id, footer, m_Settings.m_DefaultLanguage);

	ContentFactory::ReplaceInString(content, "<!-- m_Footer -->", footer);

	for (auto otherLanguage = m_Languages.begin(); otherLanguage != m_Languages.end(); ++otherLanguage)
	{
		bool isDefaultLanguage = (*otherLanguage == m_Settings.m_DefaultLanguage);
		bool isInsubfolder = !(isDefaultLanguage && m_Settings.m_DefaultLanguageInRoot);

		std::stringstream stream;
		stream << "@ROOT@";
		if (isInsubfolder)
			stream << *otherLanguage << "/";

		std::string fileName = article.GetFileNameForLanguage(*otherLanguage);
		stream << fileName << (m_LocalPreview ? "." + m_Settings.m_FileExtention : "");

		ContentFactory::ReplaceInString(content, "@LOCALIZED_LINK_" + *otherLanguage + "@", stream.str());
	}

	/*
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

		languageLinksStream << GetFileNameForLanguage(it->m_Language) << (m_LocalPreview ? "." + m_Settings.m_FileExtention : "") << "\">" << it->m_Language << "</a>";
	}
	languageLinksStream << "</span>";*/

	ContentFactory::ReplaceInString(content, "@LINK_TRAIL@", (m_LocalPreview ? "." + m_Settings.m_FileExtention : ""));
	ContentFactory::ReplaceInString(content, "@PAGE_URL@", url);
	ContentFactory::ReplaceInString(content, "@PAGE_IDENTIFIER@", mainUrl);
	ContentFactory::ReplaceInString(content, "@INDEX@", index);
	ContentFactory::ReplaceInString(content, "@INDEXFILE@", (m_LocalPreview ? "." + m_Settings.m_FileExtention : ""));
	ContentFactory::ReplaceInString(content, "@ROOT@", resourcesPath);
	ContentFactory::ReplaceInString(content, "@LANGUAGE@", language);
	ContentFactory::ReplaceInString(content, "@PWD@", subFolder);

	m_Localization.ProcessString(content, language, m_Settings.m_DefaultLanguage);
}

void Mown::AddArticleToTag(std::string tagName, Article article)
{
	CreateTag(tagName);

	m_Tags[tagName].m_Articles.push_back(article);
}

void Mown::CreateTag(std::string tagName)
{
	if (m_Tags.find(tagName) == m_Tags.end())
	{
		ArticleTag tag;
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

bool Mown::getArticleLink(const std::string & fileName, std::string & link, std::string & title)
{
	for (auto it = m_Pages.begin(); it != m_Pages.end(); ++it)
	{
		std::string filePath = it->GetSourceFilePath();

		if (filePath.find(fileName) != std::string::npos)
		{
			link = it->GetLink(m_Settings);
			title = it->GetTitle();
			return true;
		}
	}
	for (auto it = m_Articles.begin(); it != m_Articles.end(); ++it)
	{
		std::string filePath = it->GetSourceFilePath();

		if (filePath.find(fileName) != std::string::npos)
		{
			link = it->GetLink(m_Settings);
			title = it->GetTitle();
			return true;
		}
	}
	return true;
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

bool Mown::LoadConfig(const std::string& path)
{
	m_ProjectFiles.Configure(path, true);

	std::string filePath;
	boost::filesystem::path d(path);

	filePath = (d / "_settings.yaml").string();
	if (!m_Settings.LoadFromFile(filePath) && !m_Settings.SaveToFile(filePath))
		std::cerr << "Unable to load or create settings file " << filePath << std::endl;

	filePath = (d / "_localization.yaml").string();
	if (!m_Localization.LoadFromFile(filePath) && !m_Localization.SaveToFile(filePath))
		std::cerr << "Unable to load or create localization file " << filePath << std::endl;

	bool autoCreateFiles = true;

	if (!m_ProjectFiles.LoadMainTemplate())
		return false;

	if (!m_ProjectFiles.LoadArticleTemplate())
		return false;

	if (!m_ProjectFiles.LoadPageTemplate())
		return false;

	if (!m_ProjectFiles.LoadCommentsTemplate())
		return false;

	if (!m_ProjectFiles.CreateStyleSheetIfNecessary())
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

	boost::filesystem::path favicon = from / "favicon.ico";
	if (boost::filesystem::is_regular_file(favicon))
		boost::filesystem::copy(favicon, to / "favicon.ico");

	boost::filesystem::path imageFolder = from / "images";
	if (boost::filesystem::is_directory(imageFolder))
	{
		CopyDirectory(imageFolder, to / "images");
	}

	boost::filesystem::path scriptsFolder = from / "scripts";
	if (boost::filesystem::is_directory(scriptsFolder))
	{
		CopyDirectory(scriptsFolder, to / "scripts");
	}

	return true;
}
