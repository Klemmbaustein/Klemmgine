#pragma once
#include <UI/UICanvas.h>

class CSharpUICanvas : public UICanvas
{
public:
	void LoadCSharpFunctions(void* OnClicked, void* UpdateFunction, void* DestroyFunction);
	void OnButtonClicked(int Index) override;
	void Tick() override;
	virtual ~CSharpUICanvas() override;
private:
	void* OnClicked = nullptr, *UpdateFunction = nullptr, *DestroyFunction = nullptr;
};