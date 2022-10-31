#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_DATASTREAM_H
#define MUXY_GAMELINK_SCHEMA_DATASTREAM_H
#include "schema/envelope.h"
#include "schema/subscription.h"

namespace gamelink
{
	namespace schema
	{
		struct MUXY_GAMELINK_API DatastreamEvent
		{
			// Event
			nlohmann::json event;

			// Unix seconds
			int64_t timestamp;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_2(DatastreamEvent, "event", event, "timestamp", timestamp);
		};

		struct MUXY_GAMELINK_API DatastreamUpdateBody
		{
			std::vector<DatastreamEvent> events;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DatastreamUpdateBody, "events", events);
		};

		struct MUXY_GAMELINK_API DatastreamUpdate : ReceiveEnvelope<DatastreamUpdateBody>
		{
		};

		struct MUXY_GAMELINK_API SubscribeDatastreamRequest : SendEnvelope<SubscribeTopicRequestBody>
		{
			/// Creates a SubscribeDatastreamRequest
			SubscribeDatastreamRequest();
		};

		struct MUXY_GAMELINK_API UnsubscribeDatastreamRequest : SendEnvelope<UnsubscribeTopicRequestBody>
		{
			/// Creates an UnsubscribeDatastreamRequest
			UnsubscribeDatastreamRequest();
		};
	}
}

#endif
