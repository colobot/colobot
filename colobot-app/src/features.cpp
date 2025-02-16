#include <type_traits>
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

// Template parameters for generic lambdas
void test_tpfgl()
{
    auto foo = []<typename T>(T&& value)
    {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>);
    };

    foo(23);
    foo(45.6f);
    // foo('c');
}
