#pragma once
#include "common.hpp"

namespace bbones {

class Variable {
private:
	int m_val{};

public:
	Variable() = default;
	Variable(int x);
	int GetValue() const;
	void SetValue(int x);

	Variable& operator=(int x);
	Variable& operator=(const Variable& x);
};

class Scope {
private:
	std::unordered_map<std::string, Variable> m_vars{};			
	std::optional<Scope*> m_parent{};

	std::optional<Scope* const> GetParentScope() const;

public:
	Scope(std::optional<Scope*> parent = std::nullopt);
	Scope(const Scope& copy_from, std::optional<Scope*> parent = std::nullopt);

	std::optional<Variable*> GetVariable(const std::string& name);
	Variable* CreateVariable(const std::string& name);
	Variable* CreateReference(const std::string& name, Variable* ref);
	size_t GetDepth();

	// quick fix
	std::string GetStateString();
};

class ExecutionCursor {
private:
	size_t m_ord{};

public:	
	ExecutionCursor() = default;
	ExecutionCursor(size_t ord);
	size_t GetOrdinal() const;
};

class LoopCondition {
private:
	std::function<bool()> m_pred{};
	std::function<void()> m_next{};
	ExecutionCursor m_top{};

public:
	LoopCondition(const std::function<bool()>& condition, const std::function<void()>& advance, ExecutionCursor beginning);

	bool IsDone() const;
	void Advance();
	ExecutionCursor GetTop() const;
};

class ExecutionState {
private:
	std::vector<std::shared_ptr<Scope>> m_scope_stack{};
	ExecutionCursor m_ip;

public:
	ExecutionState();
	ExecutionState(
		const std::vector<std::shared_ptr<Scope>>& scopes, 
		const ExecutionCursor& ip
	);
	ExecutionState(const ExecutionState& other);

	ExecutionState DeepCopy();
	Scope* const PushScope();
	void PopScope();
	Scope* const GetScope();
	std::shared_ptr<Scope> GetGlobalScope();

	ExecutionCursor GetCursor();
	void IncrementCursor();
	void SetCursor(const ExecutionCursor& pos);

	// quick fix 
	std::string GetStateString();
};

class IProgram {
public:
	virtual ~IProgram() = default;
	virtual std::optional<std::string> Fetch(const ExecutionCursor& ip) = 0;
};

class BaseProgram : public IProgram {
private:
	std::string m_text{};

	std::string CleanInstruction(const std::string& insn);

public:
	BaseProgram(const std::string& program_text);
	virtual ~BaseProgram() = default;

	std::optional<std::string> Fetch(const ExecutionCursor& ip) override;
};

// INTERFACE fn
std::optional<BaseProgram*> CreateProgramFromFile(const std::string& path);

}