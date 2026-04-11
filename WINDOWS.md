## Noelware.Violet | Windows
At the time of this document and editions (**11/04/26**), the core **Noelware.Violet** frameworks do not support Windows and there isn't such plans on support Windows right now because for our software and products written in C++, Windows is also not supported.

### Work Required
#### `VIOLET_TRY`/`VIOLET_TRY_VOID` uses the "Statement Expression" GCC extension
Right now, the implementation for the `VIOLET_TRY` and `VIOLET_TRY_VOID` macros uses GCC's statement expression extension. MSVC doesn't support this.

The fix *could be* to use lambdas so that we don't have to provide hacks for `VIOLET_TRY(auto x, {expr})` like **Claude Code** suggests when I ask questions about it, but that is too hacky in my own opinion.

#### ...
