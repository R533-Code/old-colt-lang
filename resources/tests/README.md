# Testing the COLT Compiler:
To simplify the process of testing, CMake will recursively check for files ending with '.ct' in this folder.
For each of these file, a test will be generated. This test consist of passing the file path to the compiler so it can compile it.
Each of these file should start with a '//' followed by a string to search in the console output of the compilation.

Example: Check that `10 + 10 +;` outputs `Expected an expression!`:
```
//Expected an expression!
main() {
	10 + 10 +;
}
```