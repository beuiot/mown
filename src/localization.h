#ifndef LOCALIZATION_H
#define LOCALIZATION_H
#include <string>
#include <map>

class Localization
{
public:
	Localization();
	~Localization();

	void Clear();

	bool LoadFromFile(std::string path);
	bool SaveToFile(std::string path);
	void SetDefaultValues();
	void ProcessString(std::string& text, const std::string& language, const std::string& defaultLanguage);
	bool GetLocalizedStringFromId(const std::string& id, std::string& result, const std::string& language) const;

private:
	std::string GetLocalizedString(const std::map<std::string, std::string>& translations, const std::string& language, const std::string& defaultLanguage);

	std::map<std::string, std::map<std::string, std::string>> m_LocalizedIds;
};

#endif // LOCALIZATION_H
