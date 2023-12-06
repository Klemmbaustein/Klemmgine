#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class ClassCreator : public EditorPanel
{
public:
	ClassCreator();

	UITextField* ClassNameTextField = nullptr;

	UIText* PathText = nullptr;
	std::string PathString;

	UITextField* ClassFields[2];

	void UpdateLayout() override;
	void Tick() override;
	void OnButtonClicked(int Index) override;

	enum class ClassType
	{
		CPlusPlus,
#if ENGINE_CSHARP
		CSharp
#endif
	};

	static void Create(std::string Name, std::string Namespace, ClassType NewType);
};
#endif