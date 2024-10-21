#include "pch.h"
#include "../SpaceCadetsWeek2/parse.hpp"
#include "../SpaceCadetsWeek2/parse.cpp"

namespace barebones_tests {
	using namespace bbones;
	TEST(TestParsing, TestGetArgs) {
		Parser parser{};
		std::string fake_args{ "one two three four five six" };

		auto result = parser.ParseArgs(0, fake_args);
		auto expected = std::vector<std::string>{ "one", "two", "three", "four", "five", "six" };

		ASSERT_EQ(expected, result);
	}

	TEST(TestParsing, TestGetArgs2) {
		Parser parser{};
		std::string fake_args{ "one" };

		auto result = parser.ParseArgs(0, fake_args);
		auto expected = std::vector<std::string>{ "one" };

		ASSERT_EQ(expected, result);
	}

	TEST(TestParsing, TestGetArgs3) {
		Parser parser{};
		std::string fake_args{ "" };

		auto result = parser.ParseArgs(0, fake_args);
		auto expected = std::vector<std::string>{ };

		ASSERT_EQ(expected, result);
	}

	TEST(TestParsing, TestParse) {
		Parser parser{};
		IStatement* statement = new ClearStatement{};

		parser.AddMapping("clear", statement);
		auto result = parser.Parse("clear X");
		auto expected_args = std::vector<std::string>{ "X" };

		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(result.value().statement, statement);
		ASSERT_EQ(result.value().args, expected_args);
	}

	TEST(TestParsing, TestParse2) {
		Parser parser{};
		IStatement* statement = new WhileStatement{};

		parser.AddMapping("while", statement);
		auto result = parser.Parse("while X not 0 do");
		auto expected_args = std::vector<std::string>{ "X", "not", "0", "do" };

		ASSERT_TRUE(result.has_value());
		ASSERT_EQ(result.value().statement, statement);
		ASSERT_EQ(result.value().args, expected_args);
	}

};
