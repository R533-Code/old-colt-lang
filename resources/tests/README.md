# Testing the COLT Compiler:
To simplify the process of testing, CMake will recursively check for files ending with '.ct' in this folder.
For each of these file, a test will be generated. This test consist of passing the file path to the compiler so it can compile it.
- Each of these file should start with a `//` followed by a regex string to search in the console output of the compilation. To interpret the string as non-regex, begin the comment with ``//```.
- The second line of the file might optionally be a positive integer representing the expected error resulting in compilation.

> **Warning:**
> Semicolon (`;`) should be escaped with a backslash even if a `` ` `` precedes the regex.

---

Example: Check that this code outputs `Expected an expression!`:
```
//Expected an expression!
fn main() -> i32 {
	10 + 10 +;
}
```
Example: Check that this code outputs `Expected an expression!` and in total only 1 error:
```
//Expected an expression!
//1
fn main() -> i32 {
	10 + 10 +;
}
```

Example: Check that this code outputs `Error: Dereference operator '*' can only be applied on variables!`:
The `` ` `` prevents the `*` to be interpreted as the regex Kleene Star.
```
//`Error: Dereference operator '*' can only be applied on variables!
fn main() -> i32 {
	*10; //<- error
}
```