#ifndef BLOGEXPORTER_H
#define BLOGEXPORTER_H

#include <string>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include "projectsettings.h"
#include "article.h"
#include "articletag.h"

class Mown
{
public:
	Mown();
	~Mown();

	bool Export(std::string path);
	void Dump(bool summary);

	bool m_LocalPreview;
	bool m_EnableComments;
	bool m_ForceAll;
	std::string m_WebsiteRoot;

	std::string GetLocalUrl();

private:
	void AddArticleToTag(std::string tagName, Article article);
	std::string GenerateTagList(std::string currentTag);

	bool LoadConfig(std::string path);
	bool SetupExportFolder(std::string sourceFolder, std::string targetFolder);
	void EmptyFolder(std::string path);

	std::string m_LocalUrl;

	//TODO : remove boost dependency
	bool CopyDirectory(
		boost::filesystem::path const & source,
		boost::filesystem::path const & destination);

	ProjectSettings m_Settings;
	std::vector<Article> m_Articles;
	std::vector<Article> m_Pages;
	std::map<std::string, ArticleTag> m_Tags;
	std::vector<ArticleTag> m_SortedTags;

	std::string m_PageTemplate;
	std::string m_ArticleTemplate;
	std::string m_StandalonePageTemplate;
	std::string m_CommentsTemplate;
};

#endif // BLOGEXPORTER_H
