#ifndef CONTENTFACTORY_H
#define CONTENTFACTORY_H

#include <string>
#include <functional>

const std::string kCFYImageTag = "<image";
const std::string kCFYImageWithLegendTag = "<legendimage";
const std::string kCFYLinkTag = "<link";
const std::string kCFYMownLinkTag = "<mownlink";

const std::string kCFYCloseTag = ">";

class ContentFactory
{
public:
	ContentFactory();
	~ContentFactory();

	static std::string ReplaceImageTags(const std::string &input);
	static std::string ReplaceImageTags(const std::string &input, bool legend);
	static std::string ReplaceLinkTags(const std::string &input);
	static std::string ReplaceMownLinkTags(const std::string &input, std::function<bool(const std::string&, std::string&, std::string&)> getLink);
	static void ReplaceInString(std::string & source, std::string pattern, const std::string replacement);

	static void SanitizeString(std::string & source);
};

#endif // CONTENTFACTORY_H
