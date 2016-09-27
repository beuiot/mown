#include "contentfactory.h"
#include <iostream>

ContentFactory::ContentFactory()
{

}

ContentFactory::~ContentFactory()
{

}

std::string ContentFactory::ReplaceImageTags(const std::string& input)
{
	size_t currentIndex = 0;
	std::string result = input;

	currentIndex = result.find(kCFYImageTag, currentIndex);
	while (currentIndex < result.size())
	{

		size_t closingIndex = result.find(kCFYCloseTag, currentIndex);

		if (closingIndex < result.size())
		{

			size_t fileNameBeginIndex = result.find(" ", currentIndex);
			if (fileNameBeginIndex < closingIndex)
			{
				std::string imageTag = "";

				fileNameBeginIndex++;
				size_t fileNameEndIndex = result.find(" ", fileNameBeginIndex);

				if (fileNameEndIndex > closingIndex)
					fileNameEndIndex = closingIndex;

				std::string fileName = result.substr(fileNameBeginIndex, fileNameEndIndex - fileNameBeginIndex);

				std::string alt;
				if (fileNameEndIndex < closingIndex)
					alt = input.substr(fileNameEndIndex + 1, closingIndex - fileNameEndIndex - 1);
				else
					alt = "";

				imageTag.insert(imageTag.size(), "<p class=\"image\"><img src=\"images/");
				imageTag.insert(imageTag.end(), fileName.begin(), fileName.end());
				imageTag.insert(imageTag.size(), "\" alt=\"");
				imageTag.insert(imageTag.end(), alt.begin(), alt.end());
				imageTag.insert(imageTag.size(), "\" /></p><p class=\"image_legend\">");
				imageTag.insert(imageTag.end(), alt.begin(), alt.end());
				imageTag.insert(imageTag.size(), "</p>");

				result.erase(currentIndex, closingIndex - currentIndex + 1);
				result.insert(currentIndex, imageTag);
			}
		}

		currentIndex = result.find(kCFYImageTag, currentIndex + 1);
	}

	return result;
}

std::string ContentFactory::ReplaceLinkTags(const std::string &input, bool local)
{

	size_t currentIndex = 0;
	std::string result = input;

	currentIndex = result.find(kCFYLinkTag, currentIndex);
	while (currentIndex < result.size())
	{

		size_t closingIndex = result.find(kCFYCloseTag, currentIndex);

		if (closingIndex < result.size())
		{

			size_t fileNameBeginIndex = result.find(" ", currentIndex);
			if (fileNameBeginIndex < closingIndex)
			{
				std::string linkTag = "";

				fileNameBeginIndex++;
				size_t fileNameEndIndex = result.find(" ", fileNameBeginIndex);

				if (fileNameEndIndex > closingIndex)
					fileNameEndIndex = closingIndex;

				std::string fileName = result.substr(fileNameBeginIndex, fileNameEndIndex - fileNameBeginIndex);

				std::string title;
				if (fileNameEndIndex < closingIndex)
					title = result.substr(fileNameEndIndex + 1, closingIndex - fileNameEndIndex - 1);
				else
					title = "";

				if (local && fileName.find("http") == std::string::npos)
					fileName += ".html";

				linkTag.insert(linkTag.size(), "<a href=\"");
				linkTag.insert(linkTag.end(), fileName.begin(), fileName.end());
				linkTag.insert(linkTag.size(), "\">");
				linkTag.insert(linkTag.end(), title.begin(), title.end());
				linkTag.insert(linkTag.size(), "</a>");

				result.erase(currentIndex, closingIndex - currentIndex + 1);
				result.insert(currentIndex, linkTag);
			}
		}

		currentIndex = result.find(kCFYLinkTag, currentIndex + 1);
	}

	return result;
}

void ContentFactory::ReplaceInString(std::string &source, std::string search, const std::string replacement)
{
	size_t pos = 0;
	while ((pos = source.find(search, pos)) != std::string::npos)
	{
		source.replace(pos, search.length(), replacement);
		pos += replacement.length();
	}
}

void ContentFactory::SanitizeString(std::string &source)
{
	// Ouch. This hurts.
	ReplaceInString(source, " ", "-");
	ReplaceInString(source, "'", "-");
	ReplaceInString(source, "\\", "-");
	ReplaceInString(source, ":", "-");
	ReplaceInString(source, "ç", "c");
	ReplaceInString(source, "é", "e");
	ReplaceInString(source, "ê", "e");
	ReplaceInString(source, "à", "a");
	ReplaceInString(source, "è", "e");
	ReplaceInString(source, "--", "-");
	ReplaceInString(source, "--", "-");
	ReplaceInString(source, "--", "-");
}
