#include "LightObject.h"
#include <Objects/Components/PointLightComponent.h>
#include <Objects/Components/BillboardComponent.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Application.h>

void LightObject::Begin()
{
	Light = new PointLightComponent();
	Attach(Light);
	AddEditorProperty(Property("Intensity", NativeType::Float, &Intensity));
	AddEditorProperty(Property("Range", NativeType::Float, &Falloff));
	AddEditorProperty(Property("Color", NativeType::Vector3Color, &Color));
	OnPropertySet();

#if EDITOR
	Billboard = new BillboardComponent();
	Attach(Billboard);
	Billboard->Load(Application::GetEditorPath() + "/EditorContent/Images/Light.png");
	ModelGenerator::ModelData m;
	m.AddElement().MakeCube(2, 0);

	auto EditorCollision = new CollisionComponent();
	Attach(EditorCollision);
	EditorCollision->Load(m);
	EditorCollision->RelativeTransform.Scale = 0.25;
#endif
}

void LightObject::OnPropertySet()
{
	Light->SetColor(Color);
	Light->SetFalloff(Falloff);
	Light->SetIntensity(Intensity);
#if EDITOR
	if (Billboard)
	{
		Billboard->Color = Color;
	}
#endif
}
