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
	gamelink::SDK sdk;
	gamelink::Event<IntegerResponse> collection(&sdk, "test_collection", 5);

	int total = 0;
	collection.Add([&](const IntegerResponse& a) {
		total += a.data.value;
		collection.Add([&](const IntegerResponse& a) { total += a.data.value; });
	});

	IntegerResponse one;
	one.data.value = 1;
	collection.Invoke(one);
	REQUIRE(total == 1);

	IntegerResponse two;
	two.data.value = 2;
	collection.Invoke(two);
	REQUIRE(total == 5);
}

TEST_CASE("Removing a callback in a callback handler doesn't double delete", "[sdk]")
{
	gamelink::SDK sdk;
	gamelink::Event<IntegerResponse> collection(&sdk, "test_collection", 6);

	uint32_t h = 0;
	int total = 0;

	collection.Add(
		[&](const IntegerResponse& a) {
			total += a.data.value;
			collection.Remove(h);
		});

	h = collection.Add(
		[&](const IntegerResponse& a) {
			// This should not be called, since the previous handler will remove it.
			total += a.data.value;
		});

	IntegerResponse one;
	one.data.value = 1;
	collection.Invoke(one);
	REQUIRE(total == 1);

	IntegerResponse two;
	two.data.value = 2;
	collection.Invoke(two);
	REQUIRE(total == 3);
}

TEST_CASE("Callback handles are not valid between collections", "[sdk]")
{
	gamelink::SDK sdk;
	gamelink::Event<IntegerResponse> collection(&sdk, "test_collection", 5);
	gamelink::Event<IntegerResponse> otherCollection(&sdk, "test_collection", 6);

	int calls = 0;

	int32_t a = collection.Add([&](const IntegerResponse& a) { calls++; });
	int32_t b = otherCollection.Add([&](const IntegerResponse& a) { calls++; });

	IntegerResponse one;
	one.data.value = 1;

	collection.Invoke(one);
	otherCollection.Invoke(one);

	REQUIRE(calls == 2);

	// Remove the 'wrong' code
	otherCollection.Remove(a);
	collection.Remove(b);

	collection.Invoke(one);
	otherCollection.Invoke(one);

	// Nothing was removed
	REQUIRE(calls == 4);

	// Remove the 'right' ones
	collection.Remove(a);
	otherCollection.Remove(b);

	collection.Invoke(one);
	otherCollection.Invoke(one);

	// Both were removed
	REQUIRE(calls == 4);
}

TEST_CASE("Set unique works as expected", "[sdk]")
{
	gamelink::SDK sdk;
	gamelink::Event<IntegerResponse> collection(&sdk, "test_collection", 5);

	int calls = 0;

	collection.AddUnique("coolcat", [&](const IntegerResponse& a) { calls++; });
	collection.AddUnique("coolcat", [&](const IntegerResponse& a) { calls++; });
	collection.AddUnique("coolcat", [&](const IntegerResponse& a) { calls++; });
	collection.AddUnique("coolcat", [&](const IntegerResponse& a) { calls++; });

	IntegerResponse one;
	one.data.value = 1;

	collection.Invoke(one);
	REQUIRE(calls == 1);

	collection.RemoveByName("");
	collection.Invoke(one);
	REQUIRE(calls == 2);

	collection.RemoveByName("coolcat");
	collection.Invoke(one);
	REQUIRE(calls == 2);
}