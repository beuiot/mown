#ifndef CONTENTFACTORY_H
#define CONTENTFACTORY_H

#include <string>

const std::string kCFYImageTag = "<image";
const std::string kCFYImageWithLegendTag = "<legendimage";
const std::string kCFYLinkTag = "<link";

const std::string kCFYCloseTag = ">";

class ContentFactory
{
public:
	ContentFactory();
	~ContentFactory();

	static std::string ReplaceImageTags(const std::string &input);
	static std::string ReplaceImageTags(const std::string &input, bool legend);
	static std::string ReplaceLinkTags(const std::string &input, bool local);
	static void ReplaceInString(std::string & source, std::string pattern, const std::string replacement);

	static void SanitizeString(std::string & source);
};

#endif // CONTENTFACTORY_H
