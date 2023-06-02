#include "SoundObject.h"
#include <Objects/Components/BillboardComponent.h>
#include <Objects/Components/CollisionComponent.h>
void SoundObject::Begin()
{
	Properties.push_back(Objects::Property("Sound:Sound File", Type::E_STRING, &Filename));
	Properties.push_back(Objects::Property("Sound:Pitch", Type::E_FLOAT, &Pitch));
	Properties.push_back(Objects::Property("Sound:Volume", Type::E_FLOAT, &Volume));
	Properties.push_back(Objects::Property("Sound:Looping", Type::E_BOOL, &IsLooping));

	Properties.push_back(Objects::Property("3D-Sound:In 3D-Space", Type::E_BOOL, &IsSpacialSound));
	Properties.push_back(Objects::Property("3D-Sound:Falloff Range", Type::E_FLOAT, &FalloffRange));

#if EDITOR
	auto EditorBillboard = new BillboardComponent();
	Attach(EditorBillboard);
	EditorBillboard->Load("EditorContent/Images/Sound.png");
	ModelGenerator::ModelData m;
	m.AddElement().MakeCube(2, 0);

	auto EditorCollision = new CollisionComponent();
	Attach(EditorCollision);
	EditorCollision->Init(m.GetMergedVertices(), m.GetMergedIndices());
#endif
}

void SoundObject::Tick()
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
	Buffer = Sound::LoadSound(Filename);
	if (IsSpacialSound)
	{
		Source = Sound::PlaySound3D(Buffer, GetTransform().Location, FalloffRange, Pitch, Volume, IsLooping);
	}
	else
	{
		Source = Sound::PlaySound2D(Buffer, Pitch, Volume, IsLooping);
	}
#endif
}
