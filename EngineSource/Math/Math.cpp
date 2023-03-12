#include "Math.h"
#include <iostream>
const char* expressionToParse;
char peek()
{
    return *expressionToParse;
}

char get()
{
    return *expressionToParse++;
}

int expression();

int number()
{
    int result = get() - '0';
    while (peek() >= '0' && peek() <= '9')
    {
        result = 10 * result + get() - '0';
    }
    return result;
}

int factor()
{
    if (peek() >= '0' && peek() <= '9')
        return number();
    else if (peek() == '(')
    {
        get(); // '('
        int result = expression();
        get(); // ')'
        return result;
    }
    else if (peek() == '-')
    {
        get();
        return -factor();
    }
    return 0; // error
}

int term()
{
    int result = factor();
    while (peek() == '*' || peek() == '/')
        if (get() == '*')
            result *= factor();
        else
            result /= factor();
    return result;
}

int expression()
{
    int result = term();
    while (peek() == '+' || peek() == '-')
        if (get() == '+')
            result += term();
        else
            result -= term();
    return result;
}
bool Maths::IsPointIn2DBox(Vector2 BoxA, Vector2 BoxB, Vector2 Point)
{
	bool InArea = false;
	if (BoxA.X > BoxB.X)
	{
		InArea = (BoxA.X > Point.X && BoxA.Y > Point.Y && BoxB.X < Point.X && BoxB.Y < Point.Y);
	}
	else
	{
		InArea = (BoxA.X < Point.X && BoxA.Y < Point.Y && BoxB.X > Point.X && BoxB.Y > Point.Y);
	}
	return InArea;
}
int Maths::SolveExpr(std::string expr)
{
    expressionToParse = expr.c_str();
	return expression();
}

bool Maths::NearlyEqual(float A, float B, float epsilon)
{
    return (fabs(A - B) < epsilon);
}
