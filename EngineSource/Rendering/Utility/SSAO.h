#if !SERVER
#pragma once
namespace SSAO
{
	void Init();
	unsigned int Render(unsigned int NormalBuffer, unsigned int PositionBuffer);
	void ResizeBuffer(unsigned int X, unsigned int Y);
}
#endif