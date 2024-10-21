#include "lang.hpp"
#include <iostream>

bbones::Parser CreateParser()
{
	return bbones::Parser::Builder()
		.AddMapping("init", new bbones::InitStatement{})
		.AddMapping("incr", new bbones::IncrementStatement{})
		.AddMapping("decr", new bbones::DecrementStatement{})
		.AddMapping("clear", new bbones::ClearStatement{})
		.AddMapping("while", new bbones::WhileStatement{})
		.AddMapping("copy", new bbones::CopyStatement{})
		.AddMapping("end", new bbones::EndStatement{})
		.AddMapping("if", new bbones::IfStatement{})
		.AddMapping("elif", new bbones::NoopStatement{})
		.AddMapping("else", new bbones::NoopStatement{})
		.AddMapping("print", new bbones::PrintStatement{})
		.AddMapping("add", new bbones::AddStatement{})
		.AddMapping("sub", new bbones::SubStatement{})
		.AddMapping("mul", new bbones::MulStatement{})
		.AddMapping("div", new bbones::DivStatement{})
		.AddMapping("mod", new bbones::ModStatement{})
		.AddMapping("set", new bbones::SetStatement{})
		.AddMapping("function", new bbones::FunctionDefinitionStatement{})
		.Finish();
}

std::string GetFilePathFromUser()
{
	std::string result{};
	std::cout << "Enter the file path of a BareBones program: ";
	std::getline(std::cin, result);
	return result;
}

int main()
{
	auto parser = CreateParser();
	auto program_result = bbones::CreateProgramFromFile(GetFilePathFromUser());
	if (!program_result.has_value())
		throw std::runtime_error("Error: the provided filepath could not be opened!");
	auto* program = program_result.value();

	auto bones_instance = bbones::BareBones::Create(parser, program);
	bones_instance.Execute();
	std::cout << "success!\n";

	return 0;
}
