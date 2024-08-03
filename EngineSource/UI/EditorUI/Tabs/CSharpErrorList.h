#if EDITOR && ENGINE_CSHARP
#pragma once
#include "../EditorPanel.h"

class CSharpErrorList : public EditorPanel
{
public:
	CSharpErrorList();

	void Tick() override;
};

#endif