# clox — A Bytecode Compiler & Virtual Machine for Lox

![Language](https://img.shields.io/badge/language-C-00599C?style=flat-square)
![Build](https://img.shields.io/badge/build-CMake-orange?style=flat-square)
![Dependencies](https://img.shields.io/badge/dependencies-none-brightgreen?style=flat-square)

clox is the second Lox implementation from working through Robert Nystrom's *Crafting Interpreters*, following [jlox](https://github.com/Spiky2123/JLox), the tree-walking interpreter I built in Java. Where jlox parses source into an AST and walks it to evaluate a program, clox compiles Lox source directly into bytecode and executes that bytecode on a stack-based virtual machine, written from scratch in C with no external dependencies or generated tooling.

jlox proved I understood how a language executes. clox was about understanding *why* that execution was slow, and fixing it: same language, same semantics, but compiled and run the way production interpreters (CPython, Lua, MRI) actually work.

## Highlights

- **7x faster than jlox on average, up to 12.19x on closures**: same language, same semantics, compiled to bytecode and executed on a stack machine instead of walking a tree
- **Single-pass compilation**: a hand-written Pratt parser emits bytecode directly during parsing, with no intermediate AST
- **Manual memory management**: a mark-and-sweep garbage collector, an intrusive object list, and a from-scratch hash table for string interning and globals
- **Closures without a tree**: upvalues that start open (pointing into the stack) and close (copied onto the heap) once their scope exits
- **Real OO runtime**: classes, instances, bound methods, and single inheritance with `super`
- **Extended past the book's spec**: `final` variables, `switch`, `continue`, run-length line encoding, and native `hasField`/`deleteField` functions

## Performance

| Benchmark                | jlox   | clox   | Speedup    |
|---------------------------|--------|--------|-----------|
| Fibonacci recursion       | 2.447s | 0.445s | **5.50x**  |
| Method / instance calls   | 0.215s | 0.031s | **6.94x**  |
| Loop-heavy arithmetic     | 1.083s | 0.154s | **7.03x**  |
| String operations         | 0.077s | 0.023s | **3.35x**  |
| Closures                  | 0.256s | 0.021s | **12.19x** |

*Median of 5 runs per benchmark, measured with `clock()` on a Ryzen AI 9 365 / 32GB RAM. clox built in CMake's Release configuration (`-O2`).*

The gap comes down to what each interpreter does per operation: jlox re-walks and type-checks AST nodes on every evaluation and allocates a fresh `Environment` object per call frame, while clox compiles each expression to bytecode once and then dispatches flat instructions off a stack. The closure benchmark shows the widest gap because clox's open/closed upvalues capture variables without a heap allocation on every closure creation, unlike jlox's chained environments.

## What it does

clox implements Lox front-to-back as a compile-and-run pipeline:

- Single-pass compilation: a Pratt parser parses expressions by operator precedence and emits bytecode directly, with no intermediate AST
- A stack-based virtual machine that executes compiled bytecode via a fetch-decode-execute loop
- A tagged-union value representation for Lox's dynamic types (booleans, `nil`, numbers, and heap-allocated objects)
- A mark-and-sweep garbage collector that reclaims heap-allocated strings, closures, classes, and instances
- A hash table implementation used for string interning and for global variable storage/lookup
- Closures implemented via upvalues, both open (pointing into the stack) and closed (hoisted onto the heap once their scope ends)
- Classes, instances, bound methods, and single inheritance with `super` calls
- Control flow: `if`/`else`, `while`, `for`
- Functions with parameters, return values, and closures
- A REPL for interactive evaluation, alongside a script mode for running `.lox` files directly

## Beyond the book

Alongside the core implementation, I worked through a number of the book's end-of-chapter challenges. Each challenge can be found within its own branch:

- **Run-length line encoding**: chunks store line-number info more compactly instead of one `int` per bytecode instruction
- **Long constants**: an `OP_CONSTANT_LONG` instruction with a 24-bit operand, so a chunk isn't capped at 256 constants
- **Single-allocation strings**: `ObjString` uses a flexible array member so character data lives inline with the struct instead of needing a second heap allocation
- **Deduplicated identifier constants**: the compiler reuses an existing constant-table entry for a global's name instead of emitting a duplicate constant every time that name is referenced
- **`final` variables**: immutability enforced at compile time, rejecting reassignment to a variable declared `final`
- **`switch` statement**
- **`continue` statement** for loops
- **Field deletion**: a `deleteField` native function that removes a field from an instance at runtime
- **Field existence checking**: a `hasField` native function that checks whether an instance has a given field, instead of erroring (or returning `nil`) on a missing property

## Example

Classes and single inheritance with `super`:

```
class Animal {
  init(name) {
    this.name = name;
  }

  speak() {
    print this.name + " makes a sound.";
  }
}

class Dog < Animal {
  speak() {
    super.speak();
    print this.name + " barks.";
  }
}

var d = Dog("Rex");
d.speak();
// Rex makes a sound.
// Rex barks.
```

## Getting started

```bash
git clone https://github.com/Spiky2123/CLox.git
cd CLox
cmake -B build
cmake --build build

./build/CLox                      # start the REPL
./build/CLox path/to/script.lox   # run a Lox script
```

Requires a C compiler (GCC or Clang) and CMake. No other dependencies.

## Design & architecture

The pipeline is deliberately flatter than jlox's. A `Scanner` still turns source text into tokens, but there's no separate AST or resolver pass. The `Compiler` is a single-pass Pratt parser that parses and emits bytecode into a `Chunk` (a dynamic array of instructions, a constant pool, and line info) in the same walk. The `VM` then executes a `Chunk` directly off a value stack, dispatching each opcode in a loop rather than recursing over tree nodes.

**Values & objects.** `Value` is a tagged union representing Lox's dynamic typing without runtime type descriptors beyond a tag byte. Heap-allocated data (strings, functions, closures, classes, instances) are `Obj`s linked into a single intrusive list so the garbage collector can walk every live allocation.

**Garbage collection.** A mark-and-sweep collector marks everything reachable from the roots (the value stack, call frames, globals table, and open upvalues), sweeps everything that wasn't reached, and runs once heap usage crosses a growable threshold.

**Hash table.** `Table` is open-addressed with linear probing and backs two things: string interning, so equal strings are the same pointer and comparisons are O(1), and the globals table, so variable lookup by name doesn't require re-scanning anything.

**Closures.** Closures capture variables through upvalues instead of jlox's chained `Environment` objects. Each `ObjClosure` holds an array of upvalues that start open (pointing at a stack slot) and get closed (copied to the heap) when the variable's scope exits, what lets a closure keep working after its enclosing function has returned.

**Classes.** Classes and instances are built on the same object model, with methods stored in per-class hash tables and `super` calls resolved through an explicit superclass reference rather than walking a class hierarchy at call time.

## Tech stack

- C11
- GCC / Clang, CMake
- No external libraries: scanner, compiler, VM, hash table, and garbage collector are all implemented from scratch

## What I learned

**Compiler & VM implementation**
- Pratt parsing: parsing expressions by binding power/precedence, with prefix and infix parse functions per token type
- Single-pass compilation: generating bytecode directly during parsing instead of building and then walking an AST
- Bytecode design: instruction sets, opcode/operand encoding, and constant pools
- Stack-based execution: why dispatching a flat instruction stream is faster than recursively walking a tree

**Memory management**
- Manual memory management in C: `malloc`/`realloc`/`free` lifecycles for growable arrays and hash tables
- Garbage collection: mark-and-sweep from an explicit root set, and when a collector needs to run to keep memory bounded
- Allocation strategy: trade-offs like inlining string data into its struct to cut allocations in half

**Language implementation deep-dives**
- Closures without a tree: upvalues, the open/closed distinction, and why a VM needs a different closure mechanism than a tree-walker
- Hash tables from scratch: open addressing with linear probing, and string interning as an equality-check optimization
- Classes on a VM: method tables, bound methods, and resolving `super` without walking a class hierarchy per call

**C concepts**
- Memory layout: tagged unions for polymorphic values, structs with flexible array members
- Dynamic arrays and pointer arithmetic: growable buffers implemented from scratch with `realloc`
- Debugging tooling: a disassembler for inspecting compiled bytecode chunk by chunk