#include "XML.h"

XML::XML(std::string Name)
{
	this->Name = Name;
}

XML::XML(std::string Name, std::string Content)
{
	this->Name = Name;
	this->Content = Content;
}

std::string XML::Write()
{
	return Write(0);
}

XML& XML::Add(XML NewXML)
{
	Children.push_back(NewXML);
	return *this;
}

XML& XML::AddTag(std::string Name, std::string Content)
{
	Tags.push_back(std::pair<std::string, std::string>(Name, Content));
	return *this;
}

std::string XML::GetTags()
{
	std::string TagString;
	for (auto& i : Tags)
	{
		TagString.append(" " + i.first + "=\"" + i.second + "\"");
	}
	return TagString;
}

std::string XML::Write(size_t Depth)
{
	std::string UsedContent = Content;
	if (Content.empty() && !Children.empty())
	{
		for (auto& i : Children)
		{
			for (size_t j = 0; j < Depth + 1; j++)
			{
				UsedContent.append("  ");
			}
			UsedContent.append(i.Write(Depth + 1));
		}
	}
	else if (Children.empty() && Content.empty())
	{
		return "<" + Name + GetTags() + "/>\n";
	}

	std::string Indent;
	if (Content.empty())
	{
		for (size_t j = 0; j < Depth; j++)
		{
			Indent.append("  ");
		}
		UsedContent = "\n" + UsedContent;
	}
	return "<" + Name + GetTags() + ">" + UsedContent + Indent + "</" + Name + ">\n";
}
