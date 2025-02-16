#include <version>

// Designated initializers
void test_di()
{
    struct Foo
    {
        int x;
        float y;
    };

    Foo foo = { .x = 23, .y = 34.5f };
}
