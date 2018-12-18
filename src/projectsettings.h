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

	std::string m_Url;
	std::string m_DefaultLanguage;
	bool m_DefaultLanguageInRoot;
	std::string m_FileExtention;
};

#endif // PROJECTSETTINGS_H
