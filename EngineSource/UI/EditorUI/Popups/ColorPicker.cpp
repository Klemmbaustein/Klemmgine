#if EDITOR
#include "ColorPicker.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>
#include <Engine/Input.h>
#include <UI/EditorUI/UIVectorField.h>

typedef struct RgbColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RgbColor;

typedef struct HsvColor
{
	unsigned char h;
	unsigned char s;
	unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
	RgbColor rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.s == 0)
	{
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
	case 0:
		rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
	case 1:
		rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
	case 2:
		rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
	case 3:
		rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
	default:
		rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
	}

	return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
	HsvColor hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0)
	{
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0)
	{
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

	return hsv;
}



ColorPicker::ColorPicker(UIVectorField* Color)
	: EditorPopup(0, Vector2(0.425f, 0.45f), "Color picker")
{

	SetOptions({
	PopupOption("OK"),
	PopupOption("Cancel")
		});


	SelectedColor = Color->GetValue();
	ColorPtr = Color;

	if (SelectedColor.X > 1 || SelectedColor.Y > 1 || SelectedColor.Z > 1)
	{
		SelectedColor = SelectedColor / SelectedColor.Length();
	}
	UIBox* PickerBackground = new UIBox(UIBox::Orientation::Horizontal, 0);
	ColorPickerShaders[0] = new Shader("Shaders/UI/uishader.vert", "Shaders/Editor/color_picker.frag");
	ColorPickerShaders[0]->Bind();

	ColorPickerShaders[1] = new Shader("Shaders/UI/uishader.vert", "Shaders/Editor/color_picker.frag");
	ColorPickerShaders[1]->Bind();

	ColorPickerBackgrounds[0] = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.3f, ColorPickerShaders[0]);
	ColorPickerBackgrounds[0]->SetSizeMode(UIBox::SizeMode::AspectRelative);
	ColorPickerBackgrounds[0]->SetPadding(0.01f, 0.01f, 0.02f, 0.01f);
	ColorPickerBackgrounds[1] = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, Vector2(0.025f, 0.3f), ColorPickerShaders[1]);
	ColorPickerBackgrounds[1]->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
	ColorPickerBackgrounds[2] = new UIBackground(UIBox::Orientation::Horizontal, 0, SelectedColor, Vector2(0.1f, 0.075f));
	ColorPickerBackgrounds[2]->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);

	RGBBox = new UIBox(UIBox::Orientation::Vertical, 0);
	auto PreviewBox = new UIBox(UIBox::Orientation::Vertical, 0);
	PreviewBox->AddChild(RGBBox);
	PreviewBox->AddChild(ColorPickerBackgrounds[2]);
	PickerBackground->AddChild(ColorPickerBackgrounds[0]);
	PickerBackground->AddChild(ColorPickerBackgrounds[1]);
	PickerBackground->AddChild(PreviewBox);

	PopupBackground->AddChild(PickerBackground);

	auto c = RgbToHsv(RgbColor((uint8_t)(SelectedColor.X * 255), (uint8_t)(SelectedColor.Y * 255), (uint8_t)(SelectedColor.Z * 255)));
	SelectedHue = c.h / 255.f;
	SV = Vector2(c.s / 255.f, c.v / 255.f);
	UpdateColors();
	GenerateRGBDisplay();
}

void ColorPicker::UpdateColors()
{
	auto c = HsvToRgb(HsvColor((uint8_t)(SelectedHue * 255), 255, 255));
	SelectedColor = Vector3::Lerp(0, Vector3::Lerp(1, Vector3(c.r, c.g, c.b) / 255.0f, SV.X), SV.Y);

	size_t it = 0;
	for (auto i : ColorPickerShaders)
	{
		i->Bind();
		auto c = HsvToRgb(HsvColor((uint8_t)(SelectedHue * 255), 255, 255));
		i->SetVector3("selectedHue", Vector3(c.r, c.g, c.b) / 255.0f);
		i->SetVector2("selectedPos", SV);
		i->SetInt("mode", (int)it++);
	}
	ColorPickerBackgrounds[2]->SetColor(SelectedColor);
	GenerateRGBDisplay();
}

void ColorPicker::Tick()
{
	if (DeleteSoon)
	{
		ColorPtr->SetValue(SelectedColor);
		ColorPtr->SendNotifyEvent();
		delete this;
		return;
	}
	TickPopup();
	if (ColorPickerBackgrounds[0]->IsHovered())
	{
		Vector2 RelativeMousePosition = ((Input::MouseLocation - ColorPickerBackgrounds[0]->GetPosition()) / ColorPickerBackgrounds[0]->GetUsedSize());

		if (Input::IsLMBDown)
		{
			SV = RelativeMousePosition;
			UpdateColors();
		}
		EditorUI::CurrentCursor = EditorUI::CursorType::Cross;
	}

	if (ColorPickerBackgrounds[1]->IsHovered())
	{
		Vector2 RelativeMousePosition = ((Input::MouseLocation - ColorPickerBackgrounds[1]->GetPosition()) / ColorPickerBackgrounds[1]->GetUsedSize());

		if (Input::IsLMBDown)
		{
			SelectedHue = RelativeMousePosition.Y;
			UpdateColors();
		}
		EditorUI::CurrentCursor = EditorUI::CursorType::Cross;
	}
}
void ColorPicker::OnButtonClicked(int Index)
{
	switch (Index)
	{
	case 0:
		DeleteSoon = true;
		return;
	case 1:
		delete this;
		return;
	case 2:
	{
		for (size_t i = 0; i < 3; i++)
		{
			try
			{
				SelectedColor.at((unsigned int)i) = std::stof(RGBTexts[i]->GetText());
			}
			catch (std::exception) {}
		}
		auto c = RgbToHsv(RgbColor((uint8_t)(SelectedColor.X * 255), (uint8_t)(SelectedColor.Y * 255), (uint8_t)(SelectedColor.Z * 255)));
		SelectedHue = c.h / 255.f;
		SV = Vector2(c.s / 255.f, c.v / 255.f);
		UpdateColors();
	}
		return;
	default:
		return;
	}
}

void ColorPicker::GenerateRGBDisplay()
{
	RGBBox->DeleteChildren();
	
	std::vector<std::string> xyz = {"R:", "G:", "B:"};

	for (size_t i = 0; i < 3; i++)
	{
		RGBTexts[i] = new UITextField(0, EditorUI::UIColors[1], this, 2, EditorUI::Text);
		RGBTexts[i]
			->SetTextSize(0.4f)
			->SetMinSize(Vector2(0.06f, 0.04f))
			->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		RGBTexts[i]->SetText(EditorUI::ToShortString(SelectedColor[(int)i]));
		RGBBox->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
				
				->AddChild((new UIText(0.4f, EditorUI::UIColors[2], xyz[i], EditorUI::Text))
					->SetPadding(0.015f, 0.015f, 0.01f, 0.005f))
				->AddChild(RGBTexts[i]
					->SetPadding(0.01f, 0.01f, 0, 0)));
	}
	ColorPickerBackgrounds[0]->RedrawElement();
	ColorPickerBackgrounds[1]->RedrawElement();
}

ColorPicker::~ColorPicker()
{
	for (auto i : ColorPickerShaders)
	{
		delete i;
	}
}
#endif