# SpaceCadets03

This is my continuation of last week's challenge. Files and code may still refer to the previous
week's challenge "SpaceCadets02" for this reason.

I have been pleasantly surprised at the success of my Statement structure for modelling instructions.
The statement model as I have defined it removes the responsibility of executing code from the 
"CPU" (which is analagous to the *ExecutionState* class as defined in runtime.hpp) and instead 
delegates it to individual instruction classes which all inherit from the base interface *IStatement*.

The *ExecutionState* class' interface exposes all possible ways in which an instruction can 
change the state of the machine and accumulates these changes from instruction to instruction.
This makes it easy to define new instructions provided no changes ought be made to the CPU interface.

In addition each *IStatement* child is passed the live interpreter instance in its *Execute* method meaning
that for statements which have intrinsic context that spans multiple instructions (while, ifs, functions)
then those statements have the ability to take control of execution for as long as they deem necessary.
This nicely allows functionality to be distributed among the call stack whereas it would otherwise need
to be maintained across function calls by a third party object.

Thank you for looking at my source code. If you look close enough you will find rough edges ;)

# New features

## Arithmetic
```
mul X Y into Z;
div X Y into Z;
add X Y into Z;
sub X Y into Z;
mod X Y into Z;
```

## Subroutines, Control Flow, Printing
```
function print_fib ( n ) do;
    set l 0;
    set r 1;
    set fib_result 1;

    if n <= 1 do;
        print l;
    elif n <= 3 do;
        print r;
    else do;
        while n > 2 do;
            add l r into fib_result;
            copy r to l;
            copy fib_result to r;
            decr n;
        end;

        print fib_result;
    end;
end;

set n 4;
print_fib n;
```
Produces:
```
fib_result = 2
```