#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class DialogBox : public EditorPanel
{
public:
	struct Answer
	{
		Answer(std::string Name, void (*OnPressed)())
		{
			this->Name = Name;
			this->OnPressed = OnPressed;
		}
		std::string Name;
		void (*OnPressed)();
	};
	DialogBox(std::string Title, Vector2 Position, std::string Message, std::vector<Answer> Answers);
	void UpdateLayout() override;
	~DialogBox();
	void OnButtonClicked(int Index) override;
	void Tick() override;
protected:
	std::vector<Answer> Answers;

};
#endif