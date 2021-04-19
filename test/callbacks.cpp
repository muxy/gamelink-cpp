#include "catch2/catch.hpp"
#include "util.h"

#include "gamelink.h"

struct IntegerBody
{
    int value;
};

typedef gamelink::schema::ReceiveEnvelope<IntegerBody> IntegerResponse;

TEST_CASE("Adding a callback in a callback handler doesn't deadlock", "[sdk]")
{

    gamelink::detail::CallbackCollection<IntegerResponse, 5> collection;

    int total = 0;
    collection.set([&](const IntegerResponse& a) 
    {
        total += a.data.value;

        collection.set([&](const IntegerResponse& a)
        {
            total += a.data.value;
        }, gamelink::detail::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);
    }, gamelink::detail::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);


    IntegerResponse one;
    one.data.value = 1;
    collection.invoke(one);
    REQUIRE(total == 1);

    IntegerResponse two;
    two.data.value = 2;
    collection.invoke(two);
    REQUIRE(total == 5);  
}

TEST_CASE("Removing a callback in a callback handler doesn't double delete", "[sdk]")
{
    gamelink::detail::CallbackCollection<IntegerResponse, 5> collection;

    uint32_t h = 0;
    int total = 0;

    collection.set([&](const IntegerResponse& a)
    {
        total += a.data.value;
        collection.remove(h);
    }, gamelink::detail::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);


    h = collection.set([&](const IntegerResponse& a)
    {
        // This should not be called, since the previous handler will remove it.
        total += a.data.value;
    }, gamelink::detail::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

    IntegerResponse one;
    one.data.value = 1;
    collection.invoke(one);
    REQUIRE(total == 1);

    IntegerResponse two;
    two.data.value = 2;
    collection.invoke(two);
    REQUIRE(total == 3);  
}