#include "Exception.h"

std::string Exception::What()
{
    return Name + ": " + Text;
}

Exception::Exception(std::string Text, std::string Name)
{
    this->Text = Text;
    this->Name = Name;
}