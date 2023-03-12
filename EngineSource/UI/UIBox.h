#pragma once
#include <Math/Vector.h>
#include <set>
#include <UI/Default/ScrollObject.h>

class UIScrollBox;
class UIButton;
class UIBox
{
public:	
	bool IsVisible = true;
	enum E_UIAlign
	{
		E_DEFAULT,
		E_CENTERED,
		E_REVERSE
	};
	enum E_BorderType
	{
		E_NONE = 0,
		E_ROUNDED = 1,
		E_DARKENED_EDGE = 2
	};
	enum E_SizeMode
	{
		E_SCREEN_RELATIVE = 0,
		E_PIXEL_RELATIVE = 1
	};

	UIBox* SetSizeMode(E_SizeMode NewMode);
	E_BorderType BorderType;
	float BorderRadius;
	E_UIAlign Align = E_DEFAULT;

	UIBox(bool Horizontal, Vector2 Position);
	virtual ~UIBox();
	virtual void OnAttached();
	void InvalidateLayout();
	UIBox* AddChild(UIBox* NewChild);
	UIBox* GetAbsoluteParent();
	static void DrawAllUIElements();
	void DrawThisAndChildren();
	void DeleteChildren();

	bool IsVisibleInHierarchy();

	UIBox* SetMaxSize(Vector2 NewMaxSize);
	Vector2 GetMaxSize();

	UIBox* SetMinSize(Vector2 NewMinSize);
	Vector2 GetMinSize();
	UIBox* SetPosition(Vector2 NewPosition);
	Vector2 GetPosition();
	UIBox* SetPadding(float Up, float Down, float Left, float Right);
	UIBox* SetPadding(float AllDirs);
	UIBox* SetTryFill(bool NewTryFill);
	UIBox* SetHorizontal(bool IsHorizontal);
	bool GetTryFill();
	friend UIScrollBox;
	virtual void OnChildClicked(int Index);
	UIBox* SetBorder(UIBox::E_BorderType Type, float Size);
	static void ForceUpdateUI();
	static void InitUI();
	static unsigned int GetUIFramebuffer();
	static void RedrawUI();
	static void ClearUI();
	Vector2 GetScreenPosition();
	virtual Vector2 GetUsedSize();
protected:
	bool ShouldBeTicked = true;
	bool TryFill = false;
	virtual void Update();
	virtual void Draw();
	virtual void Tick();
	virtual void UpdateTickState();
	
	Vector2 Position;
	Vector2 OffsetPosition;
	Vector2 MaxSize = Vector2(2, 2);
	Vector2 MinSize = Vector2(0, 0);

	float UpPadding = 0.02;
	float DownPadding = 0.02;
	float RightPadding = 0.02;
	float LeftPadding = 0.02;
	Vector2 Size;
	E_SizeMode SizeMode;

	std::vector<UIBox*> Children;
	UIBox* Parent = nullptr;
	ScrollObject* CurrentScrollObject = nullptr;
	void UpdateSelfAndChildren();
private:
	bool ChildrenHorizontal;
};

namespace UI
{
	extern UIButton* HoveredButton;
	extern UIButton* NewHoveredButton;
}
