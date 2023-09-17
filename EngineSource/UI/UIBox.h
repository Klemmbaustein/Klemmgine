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
	enum class Align
	{
		Default,
		Centered,
		Reverse
	};
	enum class BorderType
	{
		None = 0,
		Rounded = 1,
		DarkenedEdge = 2
	};

	enum class SizeMode
	{
		ScreenRelative = 0,
		PixelRelative = 1
	};

	UIBox* SetSizeMode(SizeMode NewMode);
	BorderType BoxBorder = BorderType::None;
	float BorderRadius = 0;
	Align BoxAlign = Align::Default;

	UIBox(bool Horizontal, Vector2 Position);
	virtual ~UIBox();
	virtual void OnAttached();
	void InvalidateLayout();
	UIBox* AddChild(UIBox* NewChild);
	UIBox* GetAbsoluteParent();
	UIBox* SetAlign(Align NewAlign);
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
	UIBox* SetBorder(UIBox::BorderType Type, float Size);
	static void ForceUpdateUI();
	static void InitUI();
	static unsigned int* GetUITextures();
	static void RedrawUI();
	static void ClearUI();
	bool IsHovered();
	virtual Vector2 GetUsedSize();
	ScrollObject* CurrentScrollObject = nullptr;

	bool IsChildOf(UIBox* Parent);
	bool HasMouseCollision = false;

protected:
	bool ShouldBeTicked = true;
	bool TryFill = false;
	virtual void Update();
	virtual void Draw();
	virtual void Tick();
	virtual void UpdateTickState();
	void UpdateHoveredState();
	
	Vector2 Position;
	Vector2 OffsetPosition;
	Vector2 MaxSize = Vector2(2, 2);
	Vector2 MinSize = Vector2(0, 0);

	float UpPadding = 0.02f;
	float DownPadding = 0.02f;
	float RightPadding = 0.02f;
	float LeftPadding = 0.02f;
	Vector2 Size;
	SizeMode BoxSizeMode = SizeMode::ScreenRelative;

	std::vector<UIBox*> Children;
	UIBox* Parent = nullptr;
	void UpdateSelfAndChildren();
	void UpdateScale();
	void UpdatePosition();
private:
	bool ChildrenHorizontal = true;
	bool PrevIsVisible = true;
};

namespace UI
{
	extern UIBox* HoveredBox;
	extern UIBox* NewHoveredBox;
}
