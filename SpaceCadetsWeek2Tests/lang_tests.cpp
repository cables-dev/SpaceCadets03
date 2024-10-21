#include "pch.h"
#include "../SpaceCadetsWeek2/lang.hpp"

namespace barebones_tests {
	using namespace bbones;
	TEST(TestBareBones, CopyTest) {
		BareBones bbones = BareBones::Create( Parser{}, nullptr );

		// create some variable in X
		auto* x = bbones.GetExecutionState().GetScope()->CreateVariable("X");
		x->SetValue(15);

		// check X in the copy instance
		BareBones bbones_copy = bbones.BuildCopy().Finish();
		ASSERT_TRUE(bbones_copy.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones_copy.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 15);

		// changes in the copy should not affect the original barebones instance 
		bbones_copy.GetExecutionState().GetScope()->GetVariable("X").value()->SetValue(24);

		ASSERT_TRUE(bbones_copy.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones_copy.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 24);
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 15);
	}

	TEST(TestBareBones, AliasTest) {
		BareBones bbones = BareBones::Create( Parser{}, nullptr );

		// create some variable in X
		auto* x = bbones.GetExecutionState().GetScope()->CreateVariable("X");
		x->SetValue(15);

		// check X in the alias
		BareBones bbones_alias = bbones.BuildAlias().Finish();
		ASSERT_TRUE(bbones_alias.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones_alias.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 15);

		// changes in the alias should be reflected in the original barebones instance 
		bbones_alias.GetExecutionState().GetScope()->GetVariable("X").value()->SetValue(24);
		ASSERT_TRUE(bbones_alias.GetExecutionState().GetScope()->GetVariable("X").has_value());
		ASSERT_EQ(bbones_alias.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 24);
		ASSERT_EQ(bbones.GetExecutionState().GetScope()->GetVariable("X").value()->GetValue(), 24);
	}
}
	