#include "LightObject.h"
#include <Objects/Components/PointLightComponent.h>

void LightObject::Begin()
{
	Light = new PointLightComponent();
	Attach(Light);
	Properties.push_back(Objects::Property("Intensity", Type::E_FLOAT, &Intensity));
	Properties.push_back(Objects::Property("Range", Type::E_FLOAT, &Falloff));
	Properties.push_back(Objects::Property("Color", Type::E_VECTOR3_COLOR, &Color));
	OnPropertySet();
}

void LightObject::OnPropertySet()
{
	Light->SetColor(Color);
	Light->SetFalloff(Falloff);
	Light->SetIntensity(Intensity);
}
