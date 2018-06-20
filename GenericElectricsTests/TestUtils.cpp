#include "stdafx.h"
#include "stdincludes.h"
#include "TestUtils.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

string TestUtils::message = "";

TestUtils::TestUtils()
{
}


TestUtils::~TestUtils()
{
}


const wchar_t *TestUtils::Msg(string msg)
{
	wstring ret(msg.begin(), msg.end());
	return ret.c_str();
}

bool TestUtils::IsNear(const double number, const double compare, const double inaccuracy)
{
	return ((number >= compare - inaccuracy) && (number <= compare + inaccuracy));
}

bool TestUtils::IsEqual(const double n1, const double n2)
{
	return IsNear(n1, n2, 1e-9);
}


const wchar_t *TestUtils::LastMessage()
{
	string msg = message;
	message = "";
	return Msg(msg);
}

void TestUtils::SetMessage(string msg)
{
	Logger::WriteMessage(Msg(msg));
	message = msg;
}
