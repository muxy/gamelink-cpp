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
	collection.set(
		[&](const IntegerResponse& a) {
			total += a.data.value;

			collection.set([&](const IntegerResponse& a) { total += a.data.value; }, gamelink::ANY_REQUEST_ID,
						   gamelink::detail::CALLBACK_PERSISTENT);
		},
		gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

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

	collection.set(
		[&](const IntegerResponse& a) {
			total += a.data.value;
			collection.remove(h);
		},
		gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

	h = collection.set(
		[&](const IntegerResponse& a) {
			// This should not be called, since the previous handler will remove it.
			total += a.data.value;
		},
		gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

	IntegerResponse one;
	one.data.value = 1;
	collection.invoke(one);
	REQUIRE(total == 1);

	IntegerResponse two;
	two.data.value = 2;
	collection.invoke(two);
	REQUIRE(total == 3);
}

TEST_CASE("Callback handles are not valid between collections", "[sdk]")
{
	gamelink::detail::CallbackCollection<IntegerResponse, 5> collection;
	gamelink::detail::CallbackCollection<IntegerResponse, 6> otherCollection;

	int calls = 0;

	int32_t a = collection.set([&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);
	int32_t b = otherCollection.set([&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

	IntegerResponse one;
	one.data.value = 1;

	collection.invoke(one);
	otherCollection.invoke(one);

	REQUIRE(calls == 2);

	// Remove the 'wrong' code
	otherCollection.remove(a);
	collection.remove(b);

	collection.invoke(one);
	otherCollection.invoke(one);

	// Nothing was removed
	REQUIRE(calls == 4);

	// Remove the 'right' ones
	collection.remove(a);
	otherCollection.remove(b);

	collection.invoke(one);
	otherCollection.invoke(one);

	// Both were removed
	REQUIRE(calls == 4);
}

TEST_CASE("Set unique works as expected", "[sdk]")
{
	gamelink::detail::CallbackCollection<IntegerResponse, 5> collection;
	gamelink::detail::CallbackCollection<IntegerResponse, 6> otherCollection;

	int calls = 0;

	collection.setUnique("coolcat", [&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);
	collection.setUnique("coolcat", [&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);
	collection.setUnique("coolcat", [&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);
	collection.setUnique("coolcat", [&](const IntegerResponse& a) { calls++; }, gamelink::ANY_REQUEST_ID, gamelink::detail::CALLBACK_PERSISTENT);

	IntegerResponse one;
	one.data.value = 1;

	collection.invoke(one);
	REQUIRE(calls == 1);

	collection.removeByName("");
	collection.invoke(one);
	REQUIRE(calls == 2);

	collection.removeByName("coolcat");
	collection.invoke(one);
	REQUIRE(calls == 2);
}