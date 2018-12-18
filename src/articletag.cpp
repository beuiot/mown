#include "articletag.h"

#include <sstream>
#include "contentfactory.h"

ArticleTag::ArticleTag() : m_ArticlesPerPage(3)
{

}

ArticleTag::~ArticleTag()
{

}

void ArticleTag::SetLanguage(const Localization& localization, const std::string& language)
{
	m_CurrentLanguage = language;
	m_Localization = localization;
}

std::string ArticleTag::GetName()
{
	std::string result;
	const std::string id = "TAG_" + m_Name;

	if (!m_Localization.GetLocalizedStringFromId(id, result, m_CurrentLanguage))
		result = m_Name;

	return result;
}

std::string ArticleTag::GetPrettyName()
{
	std::string result = GetName();
	ContentFactory::ReplaceInString(result, " ", "&nbsp;");
	return result;
}

std::string ArticleTag::GetFileNameForPage(int page, const ProjectSettings& settings)
{
	std::stringstream ss;

	if (m_IsIndex)
	{
		if (page == 0)
			ss << "index." << settings.m_FileExtention;
		else
			ss << (page + 1) << "." << settings.m_FileExtention;
	}
	else
	{
		std::string name = GetName();
		ContentFactory::SanitizeString(name);

		if (page == 0)
			ss << name << "." << settings.m_FileExtention;
		else
			ss << name << (page + 1) << "." << settings.m_FileExtention;
	}

	return ss.str();
}

std::string ArticleTag::GetLinkForPage(int page, const ProjectSettings& settings)
{
	std::stringstream ss;

	if (m_IsIndex)
	{
		if (page == 0)
		{
			if (m_LocalPreview)
				ss << "index." << settings.m_FileExtention;
		}
		else
			ss << (page + 1) << (m_LocalPreview ? "." + settings.m_FileExtention : "");
	}
	else
	{
		std::string name = GetName();
		ContentFactory::SanitizeString(name);
		if (page == 0)
			ss << name << (m_LocalPreview ? "." + settings.m_FileExtention : "");
		else
			ss << name << (page + 1) << (m_LocalPreview ? "." + settings.m_FileExtention : "");
	}

	return ss.str();
}

std::string ArticleTag::GeneratePageList(int page, const ProjectSettings& settings)
{
	std::stringstream ss;
	ss << "<div class=\"page_list\">";

	for (int i = 0; i < GetPageCount(); i++)
	{
		if (i == page)
			ss << "<span class=\"current_page\">" << (i + 1) << "</span>";
		else
			ss << "<a href=\"@PWD@" << GetLinkForPage(i, settings) << "\">" << (i + 1) << "</a>";
	}

	ss << "</div>";

	return ss.str();
}

int ArticleTag::GetPageCount()
{
	if (m_Articles.size() == 0)
		return 1; // Always have one page so index.html is created without articles

	size_t count = ((m_Articles.size() - 1) / m_ArticlesPerPage) + 1;
	return (int)count;
}

std::vector<Article> ArticleTag::GetPageArticles(int page)
{
	size_t startIndex = 3 * page;
	size_t endIndex = 3 * page + 3;

	if (endIndex > m_Articles.size())
		endIndex = m_Articles.size();
	std::vector<Article> result(m_Articles.begin() + startIndex, m_Articles.begin() + endIndex);
	return result;
}

std::string ArticleTag::FormatArticleListPage(int page, const std::string & pageTemplate, const std::string & articleTemplate, const std::string & tagLinks, const std::string & pageLinks, const std::string & languageLinks, const ProjectSettings& settings)
{
	std::stringstream ss;
	std::vector<Article> articles = GetPageArticles(page);

	for (auto it = articles.begin(); it != articles.end(); it++)
		ss << it->FormatContent(articleTemplate, true, m_EnableComments, settings);

	std::string formatedArticles = pageTemplate;
	ContentFactory::ReplaceInString(formatedArticles, "<!-- content -->", ss.str());
	ContentFactory::ReplaceInString(formatedArticles, "<!-- head.m_Title -->", (m_IsIndex ? "" : " - " + GetName()) + (page > 0 ? " - page " + std::to_string(page + 1) : ""));

	std::string pageList = "";
	if (GetPageCount() > 1)
		pageList = GeneratePageList(page, settings);

	ContentFactory::ReplaceInString(formatedArticles, "<!-- pagelist -->", pageList);
	ContentFactory::ReplaceInString(formatedArticles, "<!-- taglinks -->", tagLinks);
	ContentFactory::ReplaceInString(formatedArticles, "<!-- pagelinks -->", pageLinks);
	ContentFactory::ReplaceInString(formatedArticles, "<!-- languagelinks -->", languageLinks);

	return formatedArticles;
}

bool ArticleTag::SortByTitle(const ArticleTag &lhs, const ArticleTag &rhs)
{
	if (lhs.m_Name == "Tous les billets")
		return true;
	else if (rhs.m_Name == "Tous les billets")
		return false;

	return lhs.m_Articles.size() > rhs.m_Articles.size();
}
