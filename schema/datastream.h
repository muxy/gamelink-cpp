#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_DATASTREAM_H
#define MUXY_GAMELINK_SCHEMA_DATASTREAM_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		struct DatastreamEvent
		{
			nlohmann::json event;
			int64_t timestamp;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(DatastreamEvent, "event", event, "timestamp", timestamp);
		};

		struct DatastreamUpdateBody
		{
			std::vector<DatastreamEvent> events;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DatastreamUpdateBody, "events", events);
		};

		struct DatastreamUpdate : ReceiveEnvelope<DatastreamUpdateBody>
		{
		};

		struct SubscribeDatastreamRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			/// Creates a SubscribeDatastreamRequest
			SubscribeDatastreamRequest();
		};

		struct UnsubscribeDatastreamRequest : SendEnvelope<UnsubscribeTopicRequestBody>
		{
			/// Creates an UnsubscribeDatastreamRequest
			UnsubscribeDatastreamRequest();
		};
	}
}

#endif
