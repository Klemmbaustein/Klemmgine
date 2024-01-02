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
#include <UI/EditorUI/EditorUI.h>

void UIVectorField::SendNotifyEvent()
{
	if (ParentUI)
		Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, this->Index));
}

UIVectorField* UIVectorField::SetValueType(VecType Type)
{
	this->Type = Type;
	Generate();
	return this;
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
	UpdateValues();
}

UIVectorField::UIVectorField(float Size, Vector3 StartValue, UICanvas* ParentUI, int Index, TextRenderer* Renderer) 
	: UIBox(UIBox::Orientation::Vertical, Position)
{
	this->Renderer = Renderer;
	Value = StartValue;
	this->ParentUI = ParentUI;
	this->Index = Index;
	this->Size = Size;
	Generate();
}

UIVectorField::~UIVectorField()
{
	for (auto i : Graphics::UIToRender)
	{
		//if (dynamic_cast<ColorPicker*>(i) && dynamic_cast<ColorPicker*>(i)->ColorPtr == this)
		//{
		//	delete i;
		//}
	}
}

void UIVectorField::Update()
{
}


void UIVectorField::UpdateValues()
{
	for (unsigned int i = 0; i < 3; i++)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << Value.at(i);
		TextFields[i]->SetText(stream.str());
	}
	if (ColorDisplay)
	{
		ColorDisplay->SetColor(Value.Length() > 1 ? Value.Normalize() : Value);
		ColorText->SetColor(std::max(Value.Length(), 0.0f) < 0.2f ? 1.0f : 0.0f);
	}
}

void UIVectorField::Generate()
{
	std::string DimensionStrings[3][3] =
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
		},
		{
			"P",
			"Y",
			"R"
		}
	};
	Vector3 Colors[3] =
	{
		Vector3(0.5f, 0.0f, 0.1f),
		Vector3(0.1f, 0.5f, 0.1f),
		Vector3(0.0f, 0.1f, 0.5f)

	};
	DeleteChildren();
	FieldBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	FieldBox->SetPadding(0);
	AddChild(FieldBox);
	float ElementSize = Size / 3.0f - 0.015f;
	for (int i = 0; i < 3; i++)
	{
		auto NewItemColor = new UIBackground(UIBox::Orientation::Vertical, 0, Colors[i], 0);
		NewItemColor
			->SetPadding(0)
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetHorizontalAlign(UIBox::Align::Centered)
			->SetMinSize(Vector2(0.015f, 0.04f))
			->SetMaxSize(Vector2(0.015f, 0.04f));
		auto ItemName = new UIText(0.4f, 1, DimensionStrings[(int)Type][i], Renderer);
		ItemName->SetPadding(0.0075f, 0.005f, 0.005f, 0.005f);
		auto NewTextField = new UITextField(0, EditorUI::UIColors[1], nullptr, i, Renderer);
		FieldBox->AddChild(NewItemColor);
		NewItemColor->AddChild(ItemName);
		NewTextField->HintText = DimensionStrings[(int)Type][i];
		NewTextField->SetPadding(0);
		NewTextField->SetTextColor(EditorUI::UIColors[2]);
		NewTextField->SetMinSize(Vector2(ElementSize, 0.04f));
		NewTextField->SetMaxSize(Vector2(ElementSize, 0.04f));
		NewTextField->SetTextSize(0.4f);
		NewTextField->ParentOverride = this;
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << Value.at(i);
		NewTextField->SetText(stream.str());
		TextFields[i] = NewTextField;
		FieldBox->AddChild(NewTextField);
	}
	if (Type == VecType::rgb)
	{
		ColorDisplay = new UIButton(UIBox::Orientation::Horizontal, 0, Value, nullptr, 3);
		ColorDisplay->ParentOverride = this;
		ColorDisplay
			->SetHorizontalAlign(UIBox::Align::Centered)
			->SetVerticalAlign(UIBox::Align::Centered);
		ColorText = new UIText(0.35f, std::max(Value.Length(), 0.0f) < 0.2f ? 1.0f : 0.0f, "Color picker", Renderer);
		ColorDisplay->AddChild(ColorText->SetPadding(0));
		AddChild(ColorDisplay);
		ColorDisplay
			->SetMinSize(Vector2(Size, 0.03f))
			->SetBorder(BorderType::Rounded, 0.25f)
			->SetTryFill(true)
			->SetPadding(0);
	}
}

void UIVectorField::OnChildClicked(int Index)
{
	try
	{
		//if (Index == 3)
		//{
		//	new ColorPicker(this);
		//	return;
		//}
		if (Value[Index] != std::stof(TextFields[Index]->GetText()))
		{
			Value[Index] = std::stof(TextFields[Index]->GetText());
			SendNotifyEvent();
		}
		UpdateValues();

	}
	catch (std::exception)
	{
	}
}
#endif