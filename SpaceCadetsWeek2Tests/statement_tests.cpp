#include "pch.h"
#include "../SpaceCadetsWeek2/runtime.hpp"
#include "../SpaceCadetsWeek2/lang.hpp"
#include "../SpaceCadetsWeek2/lang.cpp"
#include <memory>

namespace barebones_tests {
	using namespace bbones;

	bbones::Parser CreateParser()
	{
		auto parser = bbones::Parser::Builder();
		parser.AddMapping("init", new bbones::InitStatement{});
		parser.AddMapping("incr", new bbones::IncrementStatement{});
		parser.AddMapping("decr", new bbones::DecrementStatement{});
		parser.AddMapping("clear", new bbones::ClearStatement{});
		parser.AddMapping("while", new bbones::WhileStatement{});
		parser.AddMapping("copy", new bbones::CopyStatement{});
		parser.AddMapping("end", new bbones::EndStatement{});

		return parser.Finish();
	}

	TEST(StatementTests, ClearTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);
		auto& state = bbones.GetExecutionState();

		auto* clear_me = state.GetScope()->CreateVariable("ClearMe");
		auto* save_me = state.GetScope()->CreateVariable("SaveMe");
		*clear_me = 100;
		*save_me = 200;

		ASSERT_EQ(state.GetScope()->GetVariable("ClearMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("ClearMe").value()->GetValue(), 100);
		ASSERT_EQ(state.GetScope()->GetVariable("SaveMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("SaveMe").value()->GetValue(), 200);

		auto* clear_statement = new ClearStatement{};
		clear_statement->Execute(bbones, {}, {"ClearMe"});

		ASSERT_EQ(state.GetScope()->GetVariable("ClearMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("ClearMe").value()->GetValue(), 0);
		ASSERT_EQ(state.GetScope()->GetVariable("SaveMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("SaveMe").value()->GetValue(), 200);

		delete clear_statement;
	}

	TEST(StatementTests, IncrementTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);
		auto& state = bbones.GetExecutionState();

		auto* incr_me = state.GetScope()->CreateVariable("IncrMe");
		*incr_me = 250;

		ASSERT_EQ(state.GetScope()->GetVariable("IncrMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("IncrMe").value()->GetValue(), 250);

		auto* incr_statement = new IncrementStatement{};
		incr_statement->Execute(bbones, {}, { "IncrMe" });

		ASSERT_EQ(state.GetScope()->GetVariable("IncrMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("IncrMe").value()->GetValue(), 251);

		delete incr_statement;
	}

	TEST(StatementTests, DecrementTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);
		auto& state = bbones.GetExecutionState();

		auto* decr_me = state.GetScope()->CreateVariable("DecrMe");
		*decr_me = 250;

		ASSERT_EQ(state.GetScope()->GetVariable("DecrMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("DecrMe").value()->GetValue(), 250);

		auto* decr_statement = new DecrementStatement{};
		decr_statement->Execute(bbones, {}, { "DecrMe" });

		ASSERT_EQ(state.GetScope()->GetVariable("DecrMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("DecrMe").value()->GetValue(), 249);

		delete decr_statement;
	}

	TEST(StatementTests, CopyTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);
		auto& state = bbones.GetExecutionState();

		auto* copy_me = state.GetScope()->CreateVariable("CopyMe");
		auto* write_me = state.GetScope()->CreateVariable("WriteMe");
		*copy_me = 100;
		*write_me = 200;

		ASSERT_EQ(state.GetScope()->GetVariable("CopyMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("CopyMe").value()->GetValue(), 100);
		ASSERT_EQ(state.GetScope()->GetVariable("WriteMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("WriteMe").value()->GetValue(), 200);

		auto* clear_statement = new CopyStatement{};
		clear_statement->Execute(bbones, {}, {"CopyMe", "to", "WriteMe"});

		ASSERT_EQ(state.GetScope()->GetVariable("CopyMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("CopyMe").value()->GetValue(), 100);
		ASSERT_EQ(state.GetScope()->GetVariable("WriteMe").has_value(), true);
		ASSERT_EQ(state.GetScope()->GetVariable("WriteMe").value()->GetValue(), state.GetScope()->GetVariable("CopyMe").value()->GetValue());

		delete clear_statement;
	}

	TEST(StatementTests, AddTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* scope = bbones.GetExecutionState().GetScope();
		auto* x = scope->CreateVariable("X");
		auto* y = scope->CreateVariable("Y");
		auto* z = scope->CreateVariable("Z");
		
		x->SetValue(15);
		y->SetValue(10);

		auto* add_statement = new AddStatement{};
		add_statement->Execute(bbones, {}, { "X", "Y", "into", "Z" });
		delete add_statement;

		ASSERT_EQ(z->GetValue(), 25);
	}

	TEST(StatementTests, SubTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* scope = bbones.GetExecutionState().GetScope();
		auto* x = scope->CreateVariable("X");
		auto* y = scope->CreateVariable("Y");
		auto* z = scope->CreateVariable("Z");
		
		x->SetValue(15);
		y->SetValue(10);

		auto* sub_statement = new SubStatement{};
		sub_statement->Execute(bbones, {}, { "X", "Y", "into", "Z" });
		delete sub_statement;

		ASSERT_EQ(z->GetValue(), 5);
	}

	TEST(StatementTests, MulTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* scope = bbones.GetExecutionState().GetScope();
		auto* x = scope->CreateVariable("X");
		auto* y = scope->CreateVariable("Y");
		auto* z = scope->CreateVariable("Z");
		
		x->SetValue(15);
		y->SetValue(10);

		auto* mul_statement = new MulStatement{};
		mul_statement->Execute(bbones, {}, { "X", "Y", "into", "Z" });
		delete mul_statement;

		ASSERT_EQ(z->GetValue(), 150);
	}

	TEST(StatementTests, DivTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* scope = bbones.GetExecutionState().GetScope();
		auto* x = scope->CreateVariable("X");
		auto* y = scope->CreateVariable("Y");
		auto* z = scope->CreateVariable("Z");
		
		x->SetValue(20);
		y->SetValue(10);

		auto* div_statement = new DivStatement{};
		div_statement->Execute(bbones, {}, { "X", "Y", "into", "Z" });
		delete div_statement;

		ASSERT_EQ(z->GetValue(), 2);
	}

	TEST(StatementTests, ModTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* scope = bbones.GetExecutionState().GetScope();
		auto* x = scope->CreateVariable("X");
		auto* y = scope->CreateVariable("Y");
		auto* z = scope->CreateVariable("Z");
		
		x->SetValue(5);
		y->SetValue(2);

		auto* mod_statement = new ModStatement{};
		mod_statement->Execute(bbones, {}, { "X", "Y", "into", "Z" });
		delete mod_statement;

		ASSERT_EQ(z->GetValue(), 1);
	}

	TEST(StatementTests, SetTest) {
		BareBones bbones = BareBones::Create(Parser{}, nullptr);

		// Create two vars X & Y as well as the output X
		auto* set_statement = new SetStatement{};
		set_statement->Execute(bbones, {}, { "X", "120" });

		ASSERT_TRUE(bbones.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 120);
	}

	TEST(StatementTests, WhileTest) {
		/*
		(X = 100)
		while X not 0 do;
			decr X;
		end;
		*/
		BaseProgram* prog = new BaseProgram{"while X not 0 do;\ndecr X;\nend;"};
		auto parser = CreateParser();
		parser.AddMapping("while", new bbones::WhileStatement{});
		parser.AddMapping("end", new bbones::EndStatement{});

		BareBones bbones = BareBones::Create(parser, prog);
		auto* x = bbones.GetExecutionState().GetScope()->CreateVariable("X");
		x->SetValue(100);

		bbones.Execute();
		ASSERT_TRUE(bbones.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 0);
	}

	TEST(StatementTests, IfTest) {
		/*
		init X;	
		init Y;
		init Z;
		clear X;
		clear Y;
		clear Z;
		if X is 0 do;
			incr Y;
		elif X is 1 do;
			incr Z;
		end;
		*/
		BaseProgram* prog = new BaseProgram{"init X;\ninit Y;\ninit Z;\nclear X;\nclear Y;\nclear Z;\nif X is 0 do;\nincr Y;\nelif X is 1 do;\nincr Z;\nend;"};
		auto parser = CreateParser();
		parser.AddMapping("if", new bbones::IfStatement{});
		parser.AddMapping("elif", new bbones::NoopStatement{});

		BareBones bbones = BareBones::Create(parser, prog);
		bbones.Execute();
		ASSERT_TRUE(bbones.GetExecutionState().GetScope()->GetVariable("Y").has_value());
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("Y").value()->GetValue(), 1);
	}

	TEST(StatementTests, ElifTest) {
		/*
		init X;	
		init Y;
		init Z;
		clear X;
		clear Y;
		clear Z;
		if X is 1 do;
			incr Y;
		elif X is 0 do;
			incr Z;
		end;
		*/
		BaseProgram* prog = new BaseProgram{"init X;\ninit Y;\ninit Z;\nclear X;\nclear Y;\nclear Z;\nif X is 1 do;\nincr Y;\nelif X is 0 do;\nincr Z;\nend;"};
		auto parser = CreateParser();
		parser.AddMapping("if", new bbones::IfStatement{});
		parser.AddMapping("elif", new bbones::NoopStatement{});

		BareBones bbones = BareBones::Create(parser, prog);
		bbones.Execute();
		ASSERT_TRUE(bbones.GetExecutionState().GetScope()->GetVariable("Z").has_value());
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("Z").value()->GetValue(), 1);
	}

	TEST(StatementTests, ElseTest) {
		/*
		init X;	
		init Y;
		init Z;
		clear X;
		clear Y;
		clear Z;
		if X is 1 do;
			incr Y;
		elif X is 2 do;
			incr Z;
		else do;
			incr X;
		end;
		*/
		BaseProgram* prog = new BaseProgram{"init X;\ninit Y;\ninit Z;\nclear X;\nclear Y;\nclear Z;\nif X is 1 do;\nincr Y;\nelif X is 2 do;\nincr Z;\nelse do;\nincr X;\nend;"};
		auto parser = CreateParser();
		parser.AddMapping("if", new bbones::IfStatement{});
		parser.AddMapping("elif", new bbones::NoopStatement{});
		parser.AddMapping("else", new bbones::NoopStatement{});

		BareBones bbones = BareBones::Create(parser, prog);
		bbones.Execute();
		ASSERT_TRUE(bbones.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 1);
	}
}
