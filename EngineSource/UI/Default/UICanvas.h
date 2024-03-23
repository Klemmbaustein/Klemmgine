#pragma once
#include <Rendering/ShaderManager.h>
#include <Rendering/Graphics.h>
#include <set>
#include <Engine/Stats.h>

class UIBox;

/**
* @brief
* A class for managing multiple UI elements.
* 
* This class responds to button events with the OnButtonClicked() function.
* 
* @ingroup UI
*/
class UICanvas
{
public:
	/**
	* @brief
	* Creates a new UICanvas. Returns `nullptr` if in editor or server.
	* 
	* @tparam T
	* The type of the new UICanvas.
	*/
	template<typename T>
	static T* CreateNewCanvas()
	{
#if SERVER
		return nullptr;
#endif
		if (IsInEditor)
		{
			return nullptr;
		}
		return new T();
	}
	UICanvas()
	{
		Graphics::UIToRender.push_back(this);
		//Insert all UI Elements in UIElements
	}

	/**
	* @brief
	* Called each frame.
	*/
	virtual void Tick();

	/**
	* @brief
	* Called when a UIButton belonging to this UICanvas gets clicked.
	* 
	* @param Index
	* The Index of the button that has been clicked.
	*/
	virtual void OnButtonClicked(int Index);

	/**
	* @brief
	* Called when a draggable UIButton belonging to this UICanvas gets dragged.
	* 
	* @param Index
	* The Index of the button that has been dragged.
	*/
	virtual void OnButtonDragged(int Index);
	virtual ~UICanvas();
protected:
};
struct ButtonEvent
{
	//if its a event corresponding to a uibox, then o will be a pointer to the box, otherwise c will have a pointer to a uicanvas
	UIBox* o = nullptr;
	UICanvas* c = nullptr;
	int Index;
	bool IsDraggedEvent = false;
};
inline bool operator<(const ButtonEvent& a, const ButtonEvent& b)
{
	return a.c < b.c;
}