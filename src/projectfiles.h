#ifndef PROJECTFILES_H
#define PROJECTFILES_H
#include <string>

class ProjectFiles
{
public:
	ProjectFiles();
	~ProjectFiles();

	void SetProjectPath(std::string projectPath);

	bool LoadMainTemplate(bool createIfMissing, std::string websiteRoot);
	bool LoadArticleTemplate(bool createIfMissing);
	bool LoadPageTemplate(bool createIfMissing);
	bool LoadCommentsTemplate(bool createIfMissing);

	std::string GetMainTemplate();
	std::string GetArticleTemplate();
	std::string GetPageTemplate();
	std::string GetCommentsTemplate();

private:
	std::string LoadFile(std::string fileName);
	void WriteFile(std::string fileName, std::string content);

	std::string m_ProjectPath;

	std::string m_MainTemplate;
	std::string m_ArticleTemplate;
	std::string m_PageTemplate;
	std::string m_CommentsTemplate;
};

#endif // PROJECTFILES_H
