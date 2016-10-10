#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H
#include <string>

class ProjectSettings
{
public:
	ProjectSettings();
	~ProjectSettings();

	bool LoadFromFile(std::string path);
	bool SaveToFile(std::string path);
	void SetDefaultValues();

	std::string m_WebsiteName;
	std::string m_WebsiteDescription;
	std::string m_Url;
	std::string m_DefaultLanguage;
	bool m_DefaultLanguageInRoot;
};

#endif // PROJECTSETTINGS_H
