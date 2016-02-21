#ifndef ARTICLE_H
#define ARTICLE_H

#include <string>
#include <vector>
#include <boost/date_time.hpp>

class Article
{
public:
    Article();
    ~Article();

    bool LoadFromFile(std::string path);
    std::string GetFileName();
    std::string GetLink();
    std::string GetStandardDate();
    std::string FormatContent(const std::string & articleTemplate, bool isInList, bool enableComments);
    std::string FormatExcerpt();

    std::string Dump(bool showContent);

    bool m_Ignore;
    bool m_Hidden;
    bool m_LocalPreview;
    bool m_IsPage;
    std::string m_Title;
    boost::gregorian::date m_Date;

    std::vector<std::string> m_Tags;

    std::string m_Content;

    static bool SortByDate(const Article &lhs, const Article& rhs);
};

#endif // ARTICLE_H
