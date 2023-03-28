#if EDITOR
#include "UIVectorField.h"
#include <UI/UITextField.h>
#include <UI/UIBackground.h>
#include <UI/UIText.h>
#include <UI/UIButton.h>
#include <iomanip>
#include <sstream>
#include <Engine/Log.h>
#include <Engine/Application.h>

void UIVectorField::SetValueType(EFieldValueType Type)
{
	this->Type = Type;
	Generate();
}

Vector3 UIVectorField::GetValue()
{
	return Value;
}

void UIVectorField::SetValue(Vector3 NewValue)
{
	Value = NewValue;
	unsigned int Index = 0;
	for (auto i : TextFields)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << Value.at(Index);
		i->SetText(stream.str());
		Index++;
	}
}

UIVectorField::UIVectorField(Vector2 Position, Vector3 StartValue, UICanvas* ParentUI, int Index, TextRenderer* Renderer) : UIBox(false, Position)
{
	this->Renderer = Renderer;
	Value = StartValue;
	this->ParentUI = ParentUI;
	this->Index = Index;
	Generate();
}

UIVectorField::~UIVectorField()
{
}

void UIVectorField::Update()
{
}


void UIVectorField::Generate()
{
	std::string DimensionStrings[2][3] =
	{
		{
			"X",
			"Y",
			"Z"
		},
		{
			"R",
			"G",
			"B"
		}
	};
	Vector3 Colors[3] =
	{
		Vector3(0.5, 0.0, 0.1),
		Vector3(0.1, 0.5, 0.1),
		Vector3(0.0, 0.1, 0.5)

	};
	DeleteChildren();
	FieldBox = new UIBox(true, 0);
	FieldBox->SetPadding(0);
	//TODO: add color picker to editor UI in some way
	if (Type == E_RGB)
	{
		UIBackground* ColorPickerButton = new UIBackground(true, 0, Value);
		AddChild(ColorPickerButton);
		ColorPickerButton->SetTryFill(true);
		ColorPickerButton->SetMinSize(Vector2(0, 0.03));
		ColorPickerButton->SetBorder(E_ROUNDED, 0.25);
		ColorPickerButton->SetPadding(0);
	}
	AddChild(FieldBox);
	for (int i = 0; i < 3; i++)
	{
		auto NewItemColor = new UIBackground(false, 0, Colors[i], 0);
		NewItemColor->SetPadding(0);
		NewItemColor->SetMinSize(Vector2(0, 0.05));
		auto ItemName = new UIText(0.5, 1, DimensionStrings[Type][i], Renderer);
		ItemName->SetPadding(0.0075, 0.005, 0.005, 0.005);
		NewItemColor->Align = UIBox::E_REVERSE;
		NewItemColor->SetBorder(UIBox::E_ROUNDED, 0.25);
		auto NewTextField = new UITextField(true, 0, Vector3(0.2), nullptr, i, Renderer);
		FieldBox->AddChild(NewItemColor);
		NewItemColor->AddChild(ItemName);
		NewTextField->HintText = DimensionStrings[Type][i];
		NewTextField->SetPadding(0);
		NewTextField->SetMinSize(Vector2(0.07, 0.05));
		NewTextField->SetMaxSize(Vector2(0.07, 0.05));
		NewTextField->SetBorder(UIBox::E_DARKENED_EDGE, 0.25);
		NewTextField->SetTextSize(0.4);
		NewTextField->ParentOverride = this;
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << Value.at(i);
		NewTextField->SetText(stream.str());
		TextFields[i] = NewTextField;
		FieldBox->AddChild(NewTextField);
		this->SetMaxSize(Vector2(0.3, 0.075));
	}
}

void UIVectorField::OnChildClicked(int Index)
{
	try
	{
		if (Index == 3)
		{
			Log::Print("Todo: do something");
			return;
		}
		if (Value[Index] != std::stof(TextFields[Index]->GetText()))
		{
			Value[Index] = std::stof(TextFields[Index]->GetText());
			if (ParentUI) Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, this->Index));
		}
		Generate();
	}
	catch (std::exception)
	{
	}
}
#endif