#pragma once

#include <exception>

class CtfeException
{

};

class CtfeFailToEvaluateException : public CtfeException
{ 

};

class CtfeValueException : public CtfeException
{

};