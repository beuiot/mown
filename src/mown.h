#ifndef BLOGEXPORTER_H
#define BLOGEXPORTER_H

#include <string>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include "projectsettings.h"
#include "localization.h"
#include "article.h"
#include "articletag.h"
#include "projectfiles.h"

class Mown
{
public:
	Mown();
	~Mown();

	bool Export(std::string path);
	bool PathHasNewContent(std::string path);
	void Dump(bool summary);

	const Article& GetArticleForPreviewFile(std::string previewFile);

	bool m_LocalPreview;
	bool m_EnableComments;
	bool m_ForceAll;
	std::string m_WebsiteRoot;

	std::string GetLocalUrl();

private:
	void Cleanup();
	void ExportLanguage(std::string language, boost::filesystem::path exportFolder);
	void PostProcessContent(std::string& content, int directoryDepth, const std::string& subFolder, const std::string& language, const std::string& url, const std::string& mainUrl, const Article& article);
	void AddArticleToTag(std::string tagName, Article article);
	void CreateTag(std::string tagName);
	std::string GenerateTagLinks(const std::string& currentTag, const std::string& language, const ProjectSettings& settings);
	std::string GeneratePageLinks(std::string currentPage);
	std::string GenerateLanguageLinks(std::string currentLanguage);

	bool LoadConfig(const std::string& path);
	bool SetupExportFolder(std::string sourceFolder, std::string targetFolder);
	void EmptyFolder(std::string path);

	bool getArticleLink(const std::string& fileName, std::string& link, std::string& title);

	std::string m_LocalUrl;

	//TODO : remove boost dependency
	bool CopyDirectory(
		boost::filesystem::path const & source,
		boost::filesystem::path const & destination);

	ProjectSettings m_Settings;
	Localization m_Localization;
	std::vector<Article> m_Articles;
	std::vector<Article> m_Pages;
	std::vector<std::string> m_Languages;
	std::map<std::string, ArticleTag> m_Tags;
	std::vector<ArticleTag> m_SortedTags;

	ProjectFiles m_ProjectFiles;
};

#endif // BLOGEXPORTER_H
