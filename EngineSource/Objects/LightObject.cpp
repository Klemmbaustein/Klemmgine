#include "LightObject.h"
#include <Objects/Components/PointLightComponent.h>
#include <Objects/Components/BillboardComponent.h>
#include <Objects/Components/CollisionComponent.h>

void LightObject::Begin()
{
	Light = new PointLightComponent();
	Attach(Light);
	Properties.push_back(Objects::Property("Intensity", Type::E_FLOAT, &Intensity));
	Properties.push_back(Objects::Property("Range", Type::E_FLOAT, &Falloff));
	Properties.push_back(Objects::Property("Color", Type::E_VECTOR3_COLOR, &Color));
	OnPropertySet();

#if EDITOR
	auto EditorBillboard = new BillboardComponent();
	Attach(EditorBillboard);
	EditorBillboard->Load("EditorContent/Images/Light.png");
	ModelGenerator::ModelData m;
	m.AddElement().MakeCube(2, 0);

	auto EditorCollision = new CollisionComponent();
	Attach(EditorCollision);
	EditorCollision->Init(m.GetMergedVertices(), m.GetMergedIndices());
#endif
}

void LightObject::OnPropertySet()
{
	Light->SetColor(Color);
	Light->SetFalloff(Falloff);
	Light->SetIntensity(Intensity);
}
