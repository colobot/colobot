#include <compare>
#include <type_traits>
#include <version>

namespace
{

// Designated initializers
[[maybe_unused]] void test_di()
{
    struct Foo
    {
        int x;
        float y;
    };

    [[maybe_unused]] Foo foo = { .x = 23, .y = 34.5f };
}

// Template parameters for generic lambdas
[[maybe_unused]] void test_tpfgl()
{
    auto foo = []<typename T>([[maybe_unused]] T&& value)
    {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>);
    };

    foo(23);
    foo(45.6f);
    // foo('c');
}

// Concepts
template<typename T>
concept HasTest = requires
{
    typename T::test;
};

[[maybe_unused]] void test_concepts()
{
    struct Foo
    {
        using test = void;
    };

    struct Bar
    {
    };

    static_assert(HasTest<Foo>);
    static_assert(!HasTest<Bar>);
}

// Three-way comparison operator
static_assert((0 <=> 0) == 0);
static_assert((2 <=> 0) > 0);
static_assert((0 <=> 2) < 0);

} // namespace
