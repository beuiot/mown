#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H
#include <string>

class ProjectSettings
{
public:
	ProjectSettings();
	~ProjectSettings();

	bool LoadFromFile(std::string path);

	std::string m_WebsiteName;
	std::string m_WebsiteDescription;
	std::string m_Url;
};

#endif // PROJECTSETTINGS_H