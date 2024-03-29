#if !SERVER
#pragma once
#include <Math/Vector.h>
class ScrollObject
{
public:
	Vector2 Position;
	Vector2 Scale;
	float Percentage = 0;
	ScrollObject(Vector2 Position, Vector2 Scale, float MaxScroll);
	~ScrollObject();
	void ScrollUp();
	void ScrollDown();
	float Speed = 8;
	bool Active = true;
	float MaxScroll = 10;
};
#endif