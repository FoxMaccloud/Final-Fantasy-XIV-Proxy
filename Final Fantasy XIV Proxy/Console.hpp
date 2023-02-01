#pragma once

#include <iostream>
#include <Windows.h>

#define LOG(...) printf(__VA_ARGS__)
#define ERROR_LOG(...) fprintf(stderr, __VA_ARGS__)

class Console
{
public:
	Console();
	~Console();

private:

};