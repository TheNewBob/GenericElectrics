#pragma once

using namespace std;


class TestUtils
{
public:
	TestUtils();
	~TestUtils();

	static const wchar_t * Msg(string msg);

	/**
	 * \brief use from anywhere to set a message for the next test
	 */
	static void SetMessage(string msg);
	
	/**
	 * \brief use to retrieve the last message for assert statementes
	 */
	static const wchar_t *LastMessage();

	static bool TestUtils::IsNear(const double number, const double compare, const double inaccuracy);

	static bool TestUtils::IsEqual(const double n1, const double n2);
private:
	static string message;
};

