#ifdef ENGINE_CSHARP
#include "CSharpUICanvas.h"
#include <Engine/Subsystem/CSharpInterop.h>

void CSharpUICanvas::LoadCSharpFunctions(void* OnClicked, void* UpdateFunction, void* DestroyFunction)
{
	this->OnClicked = OnClicked;
	this->UpdateFunction = UpdateFunction;
	this->DestroyFunction = DestroyFunction;
}

void CSharpUICanvas::OnButtonClicked(int Index)
{
	if (OnClicked)
	{
		CSharpInterop::StaticCall<void, int>(OnClicked, Index);
	}
}

void CSharpUICanvas::Tick()
{
	CSharpInterop::StaticCall<void>(UpdateFunction);
}

CSharpUICanvas::~CSharpUICanvas()
{
	CSharpInterop::StaticCall<void>(DestroyFunction);
}
#endif