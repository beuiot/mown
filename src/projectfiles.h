#ifndef PROJECTFILES_H
#define PROJECTFILES_H
#include <string>

const std::string kPFMainTemplateFileName = "_template.html";
const std::string kPFArticleTemplateFileName = "_article_template.html";
const std::string kPFPageTemplateFileName = "_page_template.html";
const std::string kPFCommentsTemplateFileName = "_comments_template.html";

class ProjectFiles
{
public:
	ProjectFiles();
	~ProjectFiles();

	void Configure(std::string projectPath, bool createIfMissing);

	bool LoadMainTemplate(std::string websiteRoot);
	bool LoadArticleTemplate();
	bool LoadPageTemplate();
	bool LoadCommentsTemplate();
	bool CreateStyleSheetIfNecessary();

	std::string GetMainTemplate();
	std::string GetArticleTemplate();
	std::string GetPageTemplate();
	std::string GetCommentsTemplate();

private:
	bool LoadTemplate(std::string path, std::string &target, std::function<std::string()> defaultContent);
	std::string LoadFile(std::string fileName);
	void WriteFile(std::string fileName, std::string content);

	std::string m_ProjectPath;
	bool m_CreateIfMissing;


	std::string GetMainTemplateDefaultContent();
	std::string GetArticleTemplateDefaultContent();
	std::string GetPageTemplateDefaultContent();
	std::string GetCommentsTemplateDefaultContent();

	std::string m_MainTemplate;
	std::string m_ArticleTemplate;
	std::string m_PageTemplate;
	std::string m_CommentsTemplate;
};

#endif // PROJECTFILES_H
