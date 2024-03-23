#include "SoundObject.h"
#include <Objects/Components/BillboardComponent.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Application.h>

void SoundObject::Begin()
{
	
	AddEditorProperty(Property("Sound:Sound File", NativeType::String, &Filename));
	AddEditorProperty(Property("Sound:Pitch", NativeType::Float, &Pitch));
	AddEditorProperty(Property("Sound:Volume", NativeType::Float, &Volume));
	AddEditorProperty(Property("Sound:Looping", NativeType::Bool, &IsLooping));

	AddEditorProperty(Property("3D-Sound:In 3D-Space", NativeType::Bool, &IsSpatialSound));
	AddEditorProperty(Property("3D-Sound:Falloff Range", NativeType::Float, &FalloffRange));

#if EDITOR
	auto EditorBillboard = new BillboardComponent();
	Attach(EditorBillboard);
	EditorBillboard->Load(Application::GetEditorPath() + "/EditorContent/Images/Sound.png");
	ModelGenerator::ModelData m;
	m.AddElement().MakeCube(2, 0);

	auto EditorCollision = new CollisionComponent();
	Attach(EditorCollision);
	EditorCollision->Load(m);
	EditorCollision->RelativeTransform.Scale = 0.25;
#endif
}

void SoundObject::Update()
{
}

void SoundObject::OnPropertySet()
{
	LoadSound(Filename);
}

void SoundObject::Destroy()
{
	if (Buffer != nullptr)
	{
		delete Buffer;
		Source.Stop();
	}
}

void SoundObject::LoadSound(std::string SoundName)
{
	Filename = SoundName;
	if (Buffer != nullptr)
	{
		delete Buffer;
		Source.Stop();
	}
#if !EDITOR
	Buffer = new Sound::SoundBuffer(Filename);
	if (IsSpatialSound && Buffer)
	{
		Source = Sound::PlaySound3D(Buffer, GetTransform().Position, FalloffRange, Pitch, Volume, IsLooping);
	}
	else if (Buffer)
	{
		Source = Sound::PlaySound2D(Buffer, Pitch, Volume, IsLooping);
	}
#endif
}
