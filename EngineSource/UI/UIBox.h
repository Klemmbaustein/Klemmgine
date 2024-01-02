#if !SERVER
#pragma once
#include <Math/Vector.h>
#include <set>
#include <UI/Default/ScrollObject.h>

class UIScrollBox;
class UIButton;

/**
* @defgroup UI
* 
* @brief
* Functions/classes related to UI.
*/

/**
* @brief
* UIBox class. Parent class for all UI elements.
* 
* The UIBox itself is not visible.
* 
* C# equivalent: Engine.UI.UIBox
* 
* @ingroup UI
*/
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
	/**
	 * @brief
	 * Controls the orientation for all children.
	 * 
	 * If Orientation is Horizontal, the children of this UIBox will be aligned horizontally.
	 * 
	 * ```
	 * Example:
	 * 
	 *  ____________________________________    ____________________________________
	 * | _______   _______                  |  | _______                            |
	 * ||Child 1| |Child 2|                 |  ||Child 1|                           |
	 * ||_______| |_______|                 |  ||_______|  Parent box               |
	 * |                                    |  | _______   Orientation: Vertical    |
	 * |      Parent box                    |  ||Child 2|                           |
	 * |      Orientation: Horizontal       |  ||_______|                           |
	 * |____________________________________|  |____________________________________|
	 * ```
	 */
	enum class Orientation
	{
		/// Vertical orientation for all children.
		Horizontal,
		/// Horizontal orientation for all children.
		Vertical
	};

	/**
	* @brief
	* Describes the way size should be calculated for a UIBox.
	*/
	enum class SizeMode
	{
		/**
		* @brief
		* Default value. Size should be relative to the screen.
		*
		* A box with the position x=-1, y=-1 (bottom left corner) and a size of x=1, y=2 will always fill half the screen.
		*
		* A box where `size x` = `size y` will only be square if the screen itself is square.
		*/
		ScreenRelative = 0,

		/**
		* @brief
		* Size should be relative to the aspect ratio/pixels.
		*
		* A box where `size x` = `size y` is guaranteed to be square.
		*
		* A PixelRelative box with the size x=1, x=1 has the same size as a ScreenRelative box with the size x=1/AspectRatio, y=1.
		*/
		PixelRelative = 1
	};

	virtual std::string GetAsString();
	void DebugPrintTree(uint8_t Depth = 0);

	UIBox* SetSizeMode(SizeMode NewMode);
	BorderType BoxBorder = BorderType::None;
	SizeMode PaddingSizeMode = SizeMode::ScreenRelative;
	float BorderRadius = 0;
	Align HorizontalBoxAlign = Align::Default;
	Align VerticalBoxAlign = Align::Reverse;

	/**
	 * @brief
	 * Constructs UIBox with the given orientation and position.
	 */
	UIBox(Orientation BoxOritentation, Vector2 Position);
	/**
	* @brief
	* When deleted, a UIBox will also delete all children.
	*/
	virtual ~UIBox();

	virtual void OnAttached();
	void InvalidateLayout();
	UIBox* AddChild(UIBox* NewChild);
	UIBox* GetAbsoluteParent();
	UIBox* SetHorizontalAlign(Align NewAlign);
	UIBox* SetVerticalAlign(Align NewAlign);
	static void DrawAllUIElements();
	void DeleteChildren();

	bool IsVisibleInHierarchy();
	
	/**
	 * @brief
	 * Sets the maximum size this UIBox can occupy.
	 *
	 * @param NewMaxSize
	 * The new maximum size the UIBox should occupy.
	 *
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetMaxSize(Vector2 NewMaxSize);
	Vector2 GetMaxSize();

	/**
	 * @brief
	 * Sets the minimum size this UIBox can occupy.
	 * 
	 * @param NewMinSize
	 * The new minimum size the UIBox should occupy.
	 * 
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetMinSize(Vector2 NewMinSize);
	Vector2 GetMinSize();
	UIBox* SetPosition(Vector2 NewPosition);
	Vector2 GetPosition();

	/**
	 * @brief
	 * Sets the padding of a UIBox in each direction.
	 *
	 * Padding works like margin in CSS.
	 *
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetPadding(float Up, float Down, float Left, float Right);

	/**
	 * @brief
	 * Sets the padding of a UIBox, in all directions.
	 *
	 * Padding works like margin in CSS.
	 *
	 * @return
	 * A pointer to this UIBox.
	 */
	UIBox* SetPadding(float AllDirs);
	UIBox* SetTryFill(bool NewTryFill);
	UIBox* SetPaddingSizeMode(SizeMode NewSizeMode);
	UIBox* SetOrientation(Orientation NewOrientation);
	Orientation GetOrientation();
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
	
	/**
	 * @brief
	 * Gets the used size of the UIBox, the size that the UIBox occupies.
	 *
	 * @return
	 * The used size of the box.
	 */
	virtual Vector2 GetUsedSize();
	ScrollObject* CurrentScrollObject = nullptr;

	bool IsChildOf(UIBox* Parent);
	bool HasMouseCollision = false;
	void UpdateSelfAndChildren();
	std::vector<UIBox*> GetChildren();
	void SetRenderOrderIndex(size_t OrderIndex);
	size_t GetRenderOrderIndex();

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
	void UpdateScale();
	void UpdatePosition();
private:
	float GetVerticalOffset();
	float GetHorizontalOffset();
	Vector2 GetLeftRightPadding(UIBox* Target);
	void DrawThisAndChildren();
	Orientation ChildrenOrientation = Orientation::Horizontal;
	bool PrevIsVisible = true;
};

namespace UI
{
	extern UIBox* HoveredBox;
	extern UIBox* NewHoveredBox;
}
#endif