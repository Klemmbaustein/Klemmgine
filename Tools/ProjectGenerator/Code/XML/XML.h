#pragma once
#include <string>
#include <vector>

struct XML
{
	XML(std::string Name);
	XML(std::string Name, std::string Content);

	// Converts this XML structure into a readable string.
	std::string Write();

	// Adds a child to this XML element.
	// @return A reference to this element.
	XML& Add(XML NewXML);

	// Adds a tag to this XML element.
	// @return A reference to this element.
	XML& AddTag(std::string Name, std::string Content);

	// Tags to this XML element.
	// <Name Tag1.first="Tag1.second" Tag2.first="Tag2.second" ... />
	std::vector<std::pair<std::string, std::string>> Tags;

	// Children to this XML element.
	// <this>
	// |  <child1/>
	// |  <child2>
	// |   |  <child3/>
	// |  </child2>
	// </this>
	std::vector<XML> Children;
	// The name of the XML element. <Name>Content</Name>
	std::string Name;
	// The content of the XML element. <Name>Content</Name>
	std::string Content;
private:
	std::string GetTags();
	std::string Write(size_t Depth);
};