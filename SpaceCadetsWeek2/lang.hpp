#pragma once
#include "common.hpp"
#include "parse.hpp"
#include "runtime.hpp"

namespace bbones {

// fwd decl
class ParserBuilder;
class IStatement;
class BareBones;

class Parser {
public:
	std::unordered_map<std::string, IStatement*> m_mappings{};
	std::optional<IStatement*> GetStatementFor(const std::string& keyword);
	std::vector<std::string> ParseArgs(int after_pos, const std::string& statement);

public:
	Parser() = default;

	struct ParserResult {
		std::string statement_name{};
		IStatement* statement{};
		std::vector<std::string> args{};
	};
	std::optional<ParserResult> Parse(const std::string& statement);
	void AddMapping(const std::string& keyword, IStatement* statement);

	static ParserBuilder Builder();
};

class ParserBuilder {
private:
	Parser m_acc{};

public:
	ParserBuilder() = default;

	ParserBuilder& AddMapping(const std::string& keyword, IStatement* statement);
	Parser Finish();
};

class BareBonesBuilder {
private:
	struct Prototype {
		Parser parser;
		std::shared_ptr<ExecutionState> cpu;
		IProgram* program;
	};

	Prototype m_proto;
	bool m_nx{};

public:
	BareBonesBuilder(Parser parser, std::shared_ptr<ExecutionState> state, IProgram* program);

	BareBonesBuilder& NonExecutable();
	BareBonesBuilder& WithNewBaseScope();
	BareBones Finish();
};

struct BareBonesStep {
	std::string statement_name{};
	std::vector<std::string> args{};
	ExecutionCursor ip_before{};
	ExecutionCursor ip_after{};
	size_t stack_depth_before{};
	size_t stack_depth_after{};
//
//public:
//	std::string GetStatementName() const;
//	std::vector<std::string> GetArgs() const;
//	ExecutionCursor GetCursorBefore() const;
//	ExecutionCursor GetCursorAfter() const;
//	size_t GetStackDepthBefore() const;
//	size_t GetStackDepthAfter() const;
};

class BareBones {
private:
	Parser m_parser{};
	std::shared_ptr<ExecutionState> m_cpu{};
	IProgram* m_program{};
	bool m_finished{};
	bool m_nx{};

	void Finish();
	void PrintState();
	bool DoExecution();

public:
	BareBones(const Parser& parser, std::shared_ptr<ExecutionState> state, IProgram* program, bool nx);

	static BareBones Create(const Parser& parser, IProgram* program);
	ExecutionState& GetExecutionState();
	Parser& GetParser();
	void Execute();
	std::optional<BareBonesStep> Step();
	bool IsFinished();

	BareBonesBuilder BuildAlias();
	BareBonesBuilder BuildCopy();
};

class IStatement 
{
public:
	virtual ~IStatement() = default;
	virtual void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) = 0;
	virtual void Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) = 0;
};

class SkippableStatement : public IStatement
{
public:
	virtual ~SkippableStatement() = default;
	void Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class NonSkippableStatement : public IStatement
{
public:
	virtual ~NonSkippableStatement() = default;
	void Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class ClearStatement : public SkippableStatement {
public:
	ClearStatement() = default;
	virtual ~ClearStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class IncrementStatement : public SkippableStatement {
public:
	IncrementStatement() = default;
	virtual ~IncrementStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class DecrementStatement : public SkippableStatement {
public:
	DecrementStatement() = default;
	virtual ~DecrementStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class CopyStatement : public SkippableStatement {
public:
	CopyStatement() = default;
	virtual ~CopyStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

//class WhileStatement : public SkippableStatement {
class WhileStatement : public IStatement {
public:
	WhileStatement() = default;
	virtual ~WhileStatement() = default;

	void Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class EndStatementException : public std::exception {
public:
	virtual ~EndStatementException() = default;

	const char* what() const noexcept override;
};

class EndStatement : public NonSkippableStatement {
public:
	virtual ~EndStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class InitStatement : public SkippableStatement {
public:
	virtual ~InitStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class SetStatement : public SkippableStatement {
public:
	virtual ~SetStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class IfStatement : public IStatement {
private:
	bool IsBranchStatement(const std::string& name);
	bool IsConditionTrue(BareBones& machine, const std::string& cmd, const std::vector<std::string>& args);

public:
	virtual ~IfStatement() = default;

	void Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class NoopStatement : public SkippableStatement {
public:
	virtual ~NoopStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class PrintStatement : public SkippableStatement {
public:
	virtual ~PrintStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class AddStatement : public SkippableStatement {
public:
	virtual ~AddStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class SubStatement : public SkippableStatement {
public:
	virtual ~SubStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class MulStatement : public SkippableStatement {
public:
	virtual ~MulStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class DivStatement : public SkippableStatement {
public:
	virtual ~DivStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class ModStatement : public SkippableStatement {
public:
	virtual ~ModStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class FunctionDefinitionStatement : public SkippableStatement {
private:
	std::vector<std::string> DecodeParams(std::vector<std::string>::const_iterator beg, std::vector<std::string>::const_iterator end);

public:
	virtual ~FunctionDefinitionStatement() = default;

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};

class FunctionCallStatement : public SkippableStatement {
private:
	std::vector<std::string> m_params{};
	ExecutionCursor m_address{};

public:
	virtual ~FunctionCallStatement() = default;
	FunctionCallStatement(const std::vector<std::string>& params, const ExecutionCursor& address);

	void Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args) override;
};
}