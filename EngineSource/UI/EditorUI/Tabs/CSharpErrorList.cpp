#if EDITOR && ENGINE_CSHARP
#include "CSharpErrorList.h"

CSharpErrorList::CSharpErrorList()
	: EditorPanel(0, 0, "C# Error List", "csharp_errors")
{
	CanBeClosed = true;
}

void CSharpErrorList::Tick()
{
}

#endif