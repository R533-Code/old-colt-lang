# Testing the COLT Compiler:
To simplify the process of testing, CMake will recursively check for files ending with '.ct' in this folder.
For each of these file, a test will be generated. This test consist of passing the file path to the compiler so it can compile it.
Each of these file should start with a '//' followed by a regex string to search in the console output of the compilation.
The second line of the file might optionally be a positive integer representing the expected error resulting in compilation.
Semicolon should be escaped with a backslash.

Example: Check that `10 + 10 +;` outputs `Expected an expression!`:
```
//Expected an expression!
fn main() -> i32 {
	10 + 10 +;
}
```