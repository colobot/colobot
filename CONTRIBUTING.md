# How to contribute

So you want to contribute to Colobot: Gold Edition? That's awesome! Before you start, read this page, it contains a lot of useful information on how to do so.

## General information

Before you start, read more about technical details of the project. They can be found in:

* [Developer README](README-dev.md)
* [Doxygen documentation (doc)](https://github.com/colobot/colobot/actions)
* [Working with translations](https://github.com/colobot/colobot/wiki/Working-with-translations)
* [Working with models](https://github.com/colobot/colobot/wiki/Working-with-Colobot:-Gold-Edition-models)
* [Common issues](https://github.com/colobot/colobot/wiki/Common-Issues)

## Before you start coding

* If you want to fix a bug, please first check the related [issue on GitHub's bug tracker](https://github.com/colobot/colobot/issues). If there isn't one, make it.
* If you want to add a new feature or make any change to gameplay, please first discuss it either [on Discord](https://discord.gg/56Fm9kb) or in the related issue on GitHub. When your issue gets *accepted* label, that means that your suggestion got accepted and is waiting for somebody to work on it. Always wait for your suggestion to be accepted before you start writing any code.
* Before you start, check *"Assignee"* field in the issue and read the comments to see if nobody else is working on the same issue. If somebody is assigned to it, but there was no activity for a long time, you can take it over. Also, please post a comment on the issue that you want to help us, so other people don't waste time working at that issue in the same time.

## Coding style

When writing code, please adhere to the following rules:

* Indent with spaces, 1 indentation level = 4 spaces. Unix line endings. And don't leave whitespace at the end of lines. Thank you.
* Put braces in new lines.

Like that:

```c++
   if (a == b)
   {
      // ...
   }
   else
   {
      // ...
   }
```

NOT like that:

```c++
   if (a == b) {
      // ...
   } else {
      // ...
   }
```

You may omit braces if there is only one line in block:

```c++
    if (a == b)
      doStuff();
```

* Name functions beginning with upper case, e.g. `FooBar()`
* Name classes beginning with C, e.g. `class CSomeClass`
* Name accessors like so: `SetValue()` and `GetValue()`
* Name structs and enums beginning with uppercase letter, e.g. `struct SomeStruct`
* Enum values should begin with a prefix and underscore and should be all uppercase, e.g. `SOME_ENUM_VALUE`
* Use constant values instead of #define's, e.g. `const int MAX_SPACE = 1000;` instead of `#define MAX_SPACE 1000` (names should be all uppercase as in example)
* Don't use C-style casts, e.g. `(type)(foo)`. Use new C++-style casts, e.g. `static_cast<type>(foo)`.
* Don't use global variables - use static class members whenever possible.
* Provide full namespace qualifier wherever possible, e.g. Math::MultiplyMatrices to avoid confusion.
* Document the new code in Doxygen. Even a short description is better than nothing at all. Also, if you are changing/rewriting old code, please do the same.
* Put comments in your code but not explaining its structure or what it does (Doxygen is for that), but **why** it does so.
* Whenever possible, please write unit tests for your code. Tests should go into `test/` subdirectory in each of the code directories.
* You can use STL classes where needed.
* Throwing exceptions is allowed, with the exception of CBot code (which has no automated memory management yet, so memory leaks could possibly occur)

Also, when writing `#include`s:

* first - in `.cpp` modules - associated `.h` header, e.g. `#include "app/app.h"` in `app.cpp`
* then - local includes, e.g. `#include "common/logger.h"` (listed in alphabetical order for clarity)
* and last - global includes, e.g. `#include <SDL/SDL.h>`, `#include <vector>`

We also have an automated tool for checking the code style. See [colobot-lint repository](https://github.com/colobot/colobot-lint) for details.

If your pull request breaks the coding style, you will have to fix it before it gets merged.

## Commiting rules

Please adhere to the following rules:
* Commits should have meaningful descriptions.
* Commits should not break the build nor tests.
* Changes in one commit should not be too extensive, unless necessary.
* Merges to *master* branch must be discussed beforehand and should include fully finished features if possible.

## Submitting pull requests

After you finish working on your issue and want your code to be merged into the main repository, you should submit a **pull request**. Go to [this page](https://github.com/colobot/colobot/pulls) and select "New pull request". All pull requests should ALWAYS be submitted to the *dev* branch. After your PR gets reviewed by our development team, it will be merged to *dev* branch, and on the next release - to the *master* branch.

If you need more help, see [GitHub's help page on Pull Requests](https://help.github.com/articles/using-pull-requests/).

## Need help?

Ask on our [Discord server](https://discord.gg/56Fm9kb) or [GitHub Discussions](https://github.com/colobot/colobot/discussions). We're here to help :)
