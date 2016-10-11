#ifndef TAG_H
#define TAG_H

#include <string>
#include <vector>

#include "article.h"
#include "localization.h"

class ArticleTag
{
public:
	ArticleTag();
	~ArticleTag();

	void SetLanguage(const Localization& localization, const std::string& language);

	std::string m_Name;
	std::string GetName();
	std::string GetPrettyName();
	bool m_IsIndex;
	std::vector<Article> m_Articles;

	int m_ArticlesPerPage;
	bool m_LocalPreview;
	bool m_EnableComments;
	std::string m_WebsiteRoot;

	std::string GetFileNameForPage(int page);
	std::string GetLinkForPage(int page);
	std::string GeneratePageList(int page);

	int GetPageCount();

	std::vector<Article> GetPageArticles(int page);

	std::string FormatArticleListPage(int page, const std::string & pageTemplate, const std::string & articleTemplate, const std::string & tagList, const std::string & pageLinks, const std::string & languageLinks);

	static bool SortByTitle(const ArticleTag &lhs, const ArticleTag& rhs);

private:
	std::string m_CurrentLanguage;
	Localization m_Localization;
};

#endif // TAG_H
