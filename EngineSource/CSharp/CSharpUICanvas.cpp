#include "CSharpUICanvas.h"
#include "CSharpInterop.h"

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
		CSharp::StaticCall<void, int>(OnClicked, Index);
	}
}

void CSharpUICanvas::Tick()
{
	CSharp::StaticCall<void>(UpdateFunction);
}

CSharpUICanvas::~CSharpUICanvas()
{
	CSharp::StaticCall<void>(DestroyFunction);
}
