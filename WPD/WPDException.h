#pragma once

#include <exception>
#include <winerror.h>

using namespace std;

class WPDException : public exception {
	const char * s;
	HRESULT c;
public:
	WPDException(HRESULT code, const char * str) : c(code), s(str) {};
	const char * const what() { return s; }
	HRESULT code() { return c; }
};