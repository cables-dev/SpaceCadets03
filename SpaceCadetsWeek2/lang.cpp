#include "lang.hpp"
#include <iostream>

namespace bbones {

namespace macros {
	Scope* const GetScope(BareBones& machine)
	{
		return machine.GetExecutionState().GetScope();
	}
	
	// Throws runtime error on failure
	Variable* GetVariable(BareBones& machine, const std::string& name)
	{
		auto var_opt = GetScope(machine)->GetVariable(name);
		if (!var_opt.has_value())
			throw std::runtime_error("Error: tried to access variable \"" + name + "\" when it does not exist!");
		return var_opt.value();
	}

	bool DoesVariableExist(BareBones& machine, const std::string& name)
	{
		return machine.GetExecutionState().GetScope()->GetVariable(name).has_value();
	}

	Scope* const PushScope(BareBones& machine)
	{
		return machine.GetExecutionState().PushScope();
	}

	void PopScope(BareBones& machine)
	{
		machine.GetExecutionState().PopScope();
	}

	bool IsConditionTrue(
		BareBones& machine,
		const std::string& var_name,
		const std::string& operation,
		const std::string& comparison 
	)
	{
		static auto op_is = [](Variable* var, int comparison) -> bool { return var->GetValue() == comparison; };
		static auto op_not = [](Variable* var, int comparison) -> bool { return var->GetValue() != comparison; };
		static auto op_lt = [](Variable* var, int comparison) -> bool{ return var->GetValue() < comparison; };
		static auto op_lte = [](Variable* var, int comparison) -> bool { return var->GetValue() <= comparison; };
		static auto op_gt = [](Variable* var, int comparison) -> bool { return var->GetValue() > comparison; };
		static auto op_gte = [](Variable* var, int comparison) -> bool { return var->GetValue() >= comparison; };
		static std::unordered_map <std::string, std::function<bool(Variable*, int)>> ops{
			{"is", op_is},
			{"not", op_not},
			{"<", op_lt},
			{"<=", op_lte},
			{">", op_gt},
			{">=", op_gte}
		};

		auto* var = GetVariable(machine, var_name);
		auto op = ops[operation];
		auto comparison_val = std::stoi(comparison);
		return op(var, comparison_val);
	}
}

BareBonesBuilder::BareBonesBuilder(Parser parser, std::shared_ptr<ExecutionState> state, IProgram* program)
	: m_proto{parser, state, program}
{}

BareBonesBuilder& BareBonesBuilder::NonExecutable()
{
	m_nx = true;
	return *this;
}

BareBonesBuilder& BareBonesBuilder::WithNewBaseScope()
{
	auto rip = m_proto.cpu->GetCursor();
	m_proto.cpu = std::shared_ptr<ExecutionState>{ 
		new ExecutionState{}
	};
	m_proto.cpu->SetCursor(rip);
	return *this;
}

BareBones BareBonesBuilder::Finish()
{
	return BareBones{ m_proto.parser, m_proto.cpu, m_proto.program, m_nx };
}

bool BareBones::DoExecution()
{
	return !m_nx;
}

std::optional<BareBonesStep> BareBones::Step()
{
	auto rip = GetExecutionState().GetCursor();
	auto stack_depth_before = GetExecutionState().GetScope()->GetDepth();
	auto insn_str = m_program->Fetch(rip);
	if (!insn_str.has_value())
	{
		Finish();
		return std::nullopt;
	}

	auto insn_and_args = m_parser.Parse(insn_str.value());
	GetExecutionState().IncrementCursor();
	auto parse_result = insn_and_args.value();
	auto insn = parse_result.statement;
	auto args = parse_result.args;

	if (!insn_and_args.has_value())
		throw std::runtime_error{ "BareBones: instruction " + insn_str.value() + " is not recognised!" };
	if (DoExecution())
		insn->Execute(*this, rip, args);
	else
		insn->Skip(*this, rip, args);

	auto rip_after = GetExecutionState().GetCursor();
	auto stack_depth_after = GetExecutionState().GetScope()->GetDepth();
	return {{ parse_result.statement_name, args, rip, rip_after, stack_depth_before, stack_depth_after }};
}

void NoopStatement::Execute(BareBones& machine, const ExecutionCursor& cursor,const std::vector<std::string>& args)
{
}

void BareBones::Finish()
{
	m_finished = true;
}

bool BareBones::IsFinished()
{
	return m_finished;
}

BareBonesBuilder BareBones::BuildAlias()
{
	return BareBonesBuilder{ m_parser, m_cpu, m_program };
}

BareBonesBuilder BareBones::BuildCopy()
{
	// Changes made to exec state will not be reflected in the parent instance
	// since we are making a copy here.
	std::shared_ptr<ExecutionState> exec_state = std::shared_ptr<ExecutionState>{ new ExecutionState{GetExecutionState().DeepCopy()}};
	return BareBonesBuilder{ m_parser, exec_state, m_program };
}

void BareBones::PrintState()
{
	std::cout << ((m_nx) ? "Execution disabled. " : "") << m_cpu->GetStateString() << '\n';
}

BareBones BareBones::Create(const Parser& parser, IProgram* program)
{
	auto ptr_state = std::shared_ptr<ExecutionState>{ new ExecutionState{} };
	return BareBones{parser, ptr_state, program, false};
}

BareBones::BareBones(const Parser& parser, std::shared_ptr<ExecutionState> state, IProgram* program, bool nx)
	: m_parser{ parser }, m_program{ program }, m_cpu{ state }, m_nx{nx}
{
}

ExecutionState& BareBones::GetExecutionState()
{
	return *m_cpu;
}

Parser& BareBones::GetParser()
{
	return m_parser;
}

void BareBones::Execute()
{
	while (!IsFinished())
	{
		Step();
	}
}

ParserBuilder& ParserBuilder::AddMapping(const std::string& keyword, IStatement* statement)
{
	// lol
	m_acc.AddMapping(keyword, statement);
	return *this;
}

Parser ParserBuilder::Finish()
{
	auto tmp = m_acc;
	m_acc = {};
	return tmp;
}

std::optional<IStatement*> Parser::GetStatementFor(const std::string& keyword)
{
	if (m_mappings.find(keyword) == m_mappings.end())
		return std::nullopt;

	return std::optional<IStatement*>(m_mappings.at(keyword));
}

std::vector<std::string> Parser::ParseArgs(int after_pos, const std::string& statement)
{
	std::vector<std::string> result{};
	int l = after_pos;
	int r = after_pos;
	for (; r < statement.size(); r++)
	{
		if (statement[r] == ' ')
		{
			if (r != l)
				result.push_back(statement.substr(l, (r - l)));

			l = r + 1;
		}
	}
	if (l < r && l < statement.size() && r <= statement.size())
		result.push_back(statement.substr(l, r - l));

	return result;
}

void Parser::AddMapping(const std::string& keyword, IStatement* statement)
{
	m_mappings.insert({ keyword, statement });//std::make_shared<IStatement>(statement) });
}

ParserBuilder Parser::Builder()
{
	return ParserBuilder();
}

std::optional<Parser::ParserResult> Parser::Parse(const std::string& statement)
{
	// Find the first word of the statement
	std::string first_word{};
	int r = 0;

	// Skip leading whitespace
	while (statement[r] == ' ')
		r += 1;
	for (; r < statement.size(); r++)
	{
		if (statement[r] == ' ' && r > 0)
		{
			first_word = statement.substr(0, r);
			break;
		}
	}

	if (first_word.empty())
		first_word = statement;

	auto insn_statement = GetStatementFor(first_word);
	if (!insn_statement.has_value())
		return std::nullopt;

	auto args = ParseArgs(r+1, statement);
	return {{ first_word, insn_statement.value(), args }};
}

void ClearStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto target_name = args[0];
	auto* var = macros::GetVariable(machine, target_name);
	var->SetValue(0);
}

void IncrementStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto target_name = args[0];
	auto* var = macros::GetVariable(machine, target_name);
	var->SetValue(var->GetValue() + 1);
}

void DecrementStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto target_name = args[0];
	auto* var = macros::GetVariable(machine, target_name);
	var->SetValue(var->GetValue() - 1);
}

void CopyStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto src_name = args[0];
	auto dst_name = args[2];

	auto* src = macros::GetVariable(machine, src_name);
	auto* dst = macros::GetVariable(machine, dst_name);

	dst->SetValue(src->GetValue());
}

//void WhileStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
//{
//	auto& state = machine.GetExecutionState();
//	auto var_name = args[0];
//	auto* scope = state.GetScope();
//
//	auto var = scope->GetVariable(var_name);
//	if (!var.has_value())
//		throw std::runtime_error("Error: tried to access variable \"" + var_name + "\" when it does not exist!");
//
//	auto* var_ptr = var.value();
//	auto is_finished = [var_ptr]() {
//		return var_ptr->GetValue() == 0;
//	};
//	auto advance = []() {};
//	LoopCondition condition{ is_finished, advance, state.GetCursor() };
//	state.PushLoopCondition(condition);
//	state.PushScope();
//}

// simply wait for end statement and handle it
void WhileStatement::Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	try {
		while (true) {
			machine.Step();
		}
	}
	catch (EndStatementException e) {
	}
}

void WhileStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto& state = machine.GetExecutionState();
	auto var_name = args[0];
	auto bool_operation = args[1];
	auto comparison = args[2];
	auto loop_top = machine.GetExecutionState().GetCursor();

	auto machine_alias = machine.BuildAlias().Finish();
	while (!machine_alias.IsFinished())
	{
		machine_alias = (!macros::IsConditionTrue(machine_alias, var_name, bool_operation, comparison)) 
										? machine.BuildAlias()
												 .NonExecutable()
												 .Finish() 
										: machine.BuildAlias()
												 .Finish();
		machine_alias.GetExecutionState().PushScope();
		try {
			while (!machine_alias.IsFinished())
				machine_alias.Step();										// wait for end exception
		}
		catch (EndStatementException e) {
			if (macros::IsConditionTrue(machine_alias, var_name, bool_operation, comparison)) {
				machine_alias.GetExecutionState().SetCursor(loop_top);		// not efficient - always results in the last loop iter being a dud
				machine_alias.GetExecutionState().PopScope();
			}
			else {
				machine.GetExecutionState().SetCursor(machine_alias.GetExecutionState().GetCursor());
				break;
			}
		}
	}
}

void EndStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	throw EndStatementException{};
}

void InitStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto& state = machine.GetExecutionState();
	auto var_name = args[0];
	auto* scope = state.GetScope();

	auto var = scope->CreateVariable(var_name);
}

bool IfStatement::IsConditionTrue(BareBones& machine, const std::string& cmd, const std::vector<std::string>& args)
{
	if (cmd == "else")
		return true;

	return macros::IsConditionTrue(machine, args[0], args[1], args[2]);
}

bool IfStatement::IsBranchStatement(const std::string& name)
{
	static const std::unordered_set<std::string> branch_statements{ "if", "else", "elif" };
	return branch_statements.contains(name);
}

void IfStatement::Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	try {
		while (true) {
			machine.Step();
		}
	}
	catch (EndStatementException e) {
	}
}

void IfStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto machine_alias = machine
		.BuildAlias()
		.NonExecutable()
		.Finish();

	bool is_branch = true;
	bool found_branch = false;
	bool cleanup_scope = false;
	std::string condition_cmd = "if";
	std::vector<std::string> condition_args = args;

	try {
		while (true) {
			if (is_branch && !found_branch && IsConditionTrue(machine_alias, condition_cmd, condition_args)) {
				// enable execution again
				found_branch = true;
				machine_alias = machine
					.BuildAlias()
					.Finish();
				machine_alias.GetExecutionState().PushScope();
				cleanup_scope = true;
			}
			else if (is_branch) {
				auto rip = machine_alias.GetExecutionState().GetCursor();
				machine_alias = machine_alias.BuildAlias().NonExecutable().Finish();
				machine_alias.GetExecutionState().SetCursor(rip);
			}

			auto exec_result = machine_alias.Step();
			if (!exec_result.has_value())
				throw std::runtime_error{ "Its over..." };

			is_branch = IsBranchStatement(exec_result.value().statement_name);
			if (is_branch)
			{
				if (is_branch && cleanup_scope)
				{
					machine_alias.GetExecutionState().PopScope();
					cleanup_scope = false;
				}	
				condition_args = exec_result.value().args;
				condition_cmd = exec_result.value().statement_name;
			}
		}
	}
	catch (EndStatementException end) {
		if (cleanup_scope) {
			machine.GetExecutionState().PopScope();
		}
		machine.GetExecutionState().SetCursor(machine_alias.GetExecutionState().GetCursor());
	}
}

void PrintStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto var_name = args[0];
	auto* var = macros::GetVariable(machine, var_name);
	std::cout << var_name << " = " << var->GetValue() << '\n';
}

const char* EndStatementException::what() const noexcept
{
	return "Unexpected end statement encountered during execution.";
}

void SkippableStatement::Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	// noop
	return;
}

void NonSkippableStatement::Skip(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	// defer to Execute(...)
	Execute(machine, cursor, args);
}

// add X Y into Z;
void AddStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto lhs_name = args[0];
	auto rhs_name = args[1];
	auto into_keyword = args[2];
	auto into_name = args[3];

	auto* lhs = macros::GetVariable(machine, lhs_name);
	auto* rhs = macros::GetVariable(machine, rhs_name);
	auto* into = macros::GetVariable(machine, into_name);

	into->SetValue(lhs->GetValue() + rhs->GetValue());
}

// sub X Y into Z;
void SubStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto lhs_name = args[0];
	auto rhs_name = args[1];
	auto into_keyword = args[2];
	auto into_name = args[3];

	auto* lhs = macros::GetVariable(machine, lhs_name);
	auto* rhs = macros::GetVariable(machine, rhs_name);
	auto* into = macros::GetVariable(machine, into_name);

	into->SetValue(lhs->GetValue() - rhs->GetValue());
}

void MulStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto lhs_name = args[0];
	auto rhs_name = args[1];
	auto into_keyword = args[2];
	auto into_name = args[3];

	auto* lhs = macros::GetVariable(machine, lhs_name);
	auto* rhs = macros::GetVariable(machine, rhs_name);
	auto* into = macros::GetVariable(machine, into_name);

	into->SetValue(lhs->GetValue() * rhs->GetValue());
}

void DivStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto lhs_name = args[0];
	auto rhs_name = args[1];
	auto into_keyword = args[2];
	auto into_name = args[3];

	auto* lhs = macros::GetVariable(machine, lhs_name);
	auto* rhs = macros::GetVariable(machine, rhs_name);
	auto* into = macros::GetVariable(machine, into_name);

	into->SetValue(lhs->GetValue() / rhs->GetValue());
}

void ModStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto lhs_name = args[0];
	auto rhs_name = args[1];
	auto into_keyword = args[2];
	auto into_name = args[3];

	auto* lhs = macros::GetVariable(machine, lhs_name);
	auto* rhs = macros::GetVariable(machine, rhs_name);
	auto* into = macros::GetVariable(machine, into_name);

	into->SetValue(lhs->GetValue() % rhs->GetValue());
}

void SetStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto var_name = args[0];
	auto val_str = args[1];
	auto val = std::stoi(val_str);

	Variable* var = nullptr;
	if (macros::DoesVariableExist(machine, var_name)) {
		auto* var = macros::GetVariable(machine, var_name);
		var->SetValue(val);
	}
	else {
		var = macros::GetScope(machine)->CreateVariable(var_name);
	}

	var->SetValue(val);
}

std::vector<std::string> FunctionDefinitionStatement::DecodeParams(std::vector<std::string>::const_iterator beg, std::vector<std::string>::const_iterator end)
{
	std::vector<std::string> result{};
	bool started = false;
	for (beg; beg != end; beg++)
	{
		auto str = *beg;
		if (str == "(")
			started = true;
		else if (str == ")")
			return result;
		else if (started)
			result.push_back(str);
	}

	if (!started)
		throw std::runtime_error{ "Error in function definition: expected \"(\"." };
	else 
		throw std::runtime_error{ "Error in function definition: expected \")\"." };
}

void FunctionDefinitionStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	auto identifier = args[0];
	auto params = DecodeParams(args.begin() + 1, args.end());
	auto address = machine.GetExecutionState().GetCursor();

	auto machine_alias = machine.BuildAlias().NonExecutable().Finish();
	try {
		while (!machine.IsFinished()) {
			auto step_result = machine_alias.Step();
		}
	}
	catch (EndStatementException e) {
		// Finish - create call statement
		machine.GetParser().AddMapping(identifier, new FunctionCallStatement{ params, address });
	}
}

FunctionCallStatement::FunctionCallStatement(const std::vector<std::string>& params, const ExecutionCursor& address)
	: m_params{ params }, m_address { address }
{
}

void FunctionCallStatement::Execute(BareBones& machine, const ExecutionCursor& cursor, const std::vector<std::string>& args)
{
	if (args.size() != m_params.size())
		throw std::runtime_error{ "Incorrect number of arguments passed to function call." };		// TODO: a more helpful message here

	auto machine_alias = machine.BuildAlias()
								.WithNewBaseScope()
								.Finish();

	// Create scope
	auto* scope = macros::PushScope(machine_alias);

	// Load arguments in parameters
	// TODO: pass by ref
	for (int i{}; i < args.size(); i++) {
		auto arg_name = args[i];
		auto param_name = m_params[i];
	
		auto* arg = macros::GetVariable(machine, arg_name);
		auto* param = scope->CreateVariable(param_name);
		param->SetValue(arg->GetValue());
	}

	// Save return address on call stack
	auto return_address = machine_alias.GetExecutionState().GetCursor();

	// Jump to fn
	machine_alias.GetExecutionState().SetCursor(m_address);
	try {
		while (!machine_alias.IsFinished()) {
			machine_alias.Step();
		}
	}
	catch (EndStatementException e) {
		macros::PopScope(machine_alias);

		// Return to prior execution position
		machine.GetExecutionState().SetCursor(return_address);
	}
}

}
