#ifndef ARTICLE_H
#define ARTICLE_H

#include <string>
#include <vector>
#include <map>
#include <boost/date_time.hpp>
#include "projectsettings.h"

class Article
{
public:
	Article();
	~Article();

	bool m_LocalPreview;
	bool m_IsPage;

	bool LoadFromFile(std::string path, std::vector<std::string>& languages);
	std::string GetSourceFilePath();
	std::string GetFileName();
	std::string GetFileNameForLanguage(const std::string& language);
	bool HasFileName(const std::string& fileName);
	std::string GetLink();
	std::string GetLinkForLanguage(const std::string& language);
	std::string GetStandardDate();
	std::string FormatContent(const std::string & articleTemplate, bool isInList, bool enableComments, const ProjectSettings& settings);
	std::string FormatExcerpt();

	bool SetLanguage(std::string language);
	bool HasCurrentLanguage();
	bool GetIgnore();
	bool GetHidden();
	bool GetIsHomepage();
	std::string GetTitle();
	const std::vector<std::string>& GetTags();

	std::string Dump(bool showContent);


	static bool SortByDate(const Article &lhs, const Article& rhs);
	static bool SortByOrder(const Article &lhs, const Article &rhs);
private:
	class ArticleData
	{
	public:
		ArticleData();
		~ArticleData();

		bool ParseYaml(const ArticleData& defaultValues);

		bool m_Ignore;
		bool m_Hidden;
		int m_Order;
		bool m_IsHomepage;
		std::string m_Title;
		std::string m_Language;
		std::string m_Link;
		bool m_ExternalLink;
		boost::gregorian::date m_Date;
		std::vector<std::string> m_Tags;

		std::string m_RawYaml;
		std::string m_Content;
	};
	std::string m_SourceFilePath;
	std::string m_CurrentLanguage;

	std::vector<ArticleData> m_Data;
	ArticleData m_CurrentData;
	bool m_HasCurrentLanguage;

	bool LoadFile();
	void AddArticleData(std::string rawYaml, std::string rawContent);
	bool SaveFile();
	bool FindData(std::string language, ArticleData& data);
};

#endif // ARTICLE_H
