#pragma once
#include <Rendering/Utility/ShaderManager.h>
#include <Rendering/Graphics.h>
#include <set>
#include <Engine/Stats.h>

class UIBox;
class UIRenderingException : public std::exception
{
public:
	UIRenderingException(std::string UIType, std::string ErrorType)
	{
		Exception = "UI rendering exception thrown: " + UIType + ": " + ErrorType;
	}

	virtual const char* what() const throw()
	{
		return Exception.c_str();
	}

	std::string Exception;
};

class UICanvas
{
public:
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

	virtual void Tick();

	virtual void OnButtonClicked(int Index);
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