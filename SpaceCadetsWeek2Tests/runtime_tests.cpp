#include "pch.h"
#include "../SpaceCadetsWeek2/runtime.hpp"

namespace barebones_tests {
	using namespace bbones;
	TEST(TestVariable, TestVariableInit) {
		// Initialise a variable
		Variable x{15};
		ASSERT_EQ(x.GetValue(), 15);

		// Test set
		x.SetValue(x.GetValue() * 2);
		ASSERT_EQ(x.GetValue(), 30);
	}

	TEST(TestVariable, TestVariableOperators) {
		// Initialise a variable
		Variable x{15};
		ASSERT_EQ(x.GetValue(), 15);

		// Test set #1 
		x = Variable{ 21 };
		ASSERT_EQ(x.GetValue(), 21);

		// Test set #2
		x = Variable{ 0 };
		ASSERT_EQ(x.GetValue(), 0);

	}

	TEST(TestScopes, TestScopeVariableLookup) {
		Scope scope{};
		Variable* x = scope.CreateVariable("Helloworld");
		
		constexpr int val = 51 * 71023;
		x->SetValue(val);

		auto var = scope.GetVariable("Helloworld");
		ASSERT_EQ(var.has_value(), true);
		ASSERT_EQ(var.value()->GetValue(), val);
	}

	TEST(TestScopes, TestScopeNestedVariableLookup) {
		std::shared_ptr<Scope> parent_scope = std::shared_ptr<Scope>{ new Scope{} };
		Variable* x = parent_scope->CreateVariable("Helloworld");
		Variable* y = parent_scope->CreateVariable("x123");
		*x = 51;

		std::shared_ptr<Scope> child_scope = std::shared_ptr<Scope>{ std::shared_ptr<Scope>{parent_scope} };
		Variable* z = child_scope->CreateVariable("testing");

		// Find variable within the parent scope from the child scope
		auto result_var = child_scope->GetVariable("Helloworld");
		ASSERT_EQ(result_var.has_value(), true);
		ASSERT_EQ(result_var.value()->GetValue(), 51);
	}

	TEST(TestExecutionState, TestExecutionStateScopes) {
		ExecutionState state{};

		// Access global scope, TestVar = 100
		auto* global_scope = state.GetScope();
		auto* test_var = global_scope->CreateVariable("TestVar");
		*test_var = 100;

		// Create local scope
		auto* new_scope = state.PushScope();
		auto* test_var_2 = new_scope->CreateVariable("TestVar2");
		*test_var_2 = 500;

		ASSERT_EQ(state.GetScope()->GetVariable("TestVar").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar").value()->GetValue(), 100);
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar2").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar2").value()->GetValue(), 500);

		// Pop local scope
		state.PopScope();
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar").value()->GetValue(), 100);
		ASSERT_EQ(state.GetScope()->GetVariable("TestVar2").has_value(), false);
	}

	TEST(TestProgram, TestProgramFetch) {
		std::string program_txt = "incr X;clear Y;decr X;";
		BaseProgram prog = BaseProgram{ program_txt };

		EXPECT_EQ(prog.Fetch(2).has_value(), true);
		EXPECT_EQ(prog.Fetch(2).value(), "decr X");
		EXPECT_EQ(prog.Fetch(0).has_value(), true);
		EXPECT_EQ(prog.Fetch(0).value(), "incr X");
		EXPECT_EQ(prog.Fetch(1).has_value(), true);
		EXPECT_EQ(prog.Fetch(1).value(), "clear Y");
	}
	
	TEST(TestProgram, TestProgramFetchFilterWhitespace) {
		std::string program_txt = "incr X;\nclear Y;\ndecr X;";
		BaseProgram prog = BaseProgram{ program_txt };

		EXPECT_EQ(prog.Fetch(2).has_value(), true);
		EXPECT_EQ(prog.Fetch(2).value(), "decr X");
		EXPECT_EQ(prog.Fetch(0).has_value(), true);
		EXPECT_EQ(prog.Fetch(0).value(), "incr X");
		EXPECT_EQ(prog.Fetch(1).has_value(), true);
		EXPECT_EQ(prog.Fetch(1).value(), "clear Y");
	}
}