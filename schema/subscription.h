#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_SUBSCRIPTION_H
#define MUXY_GAMELINK_SCHEMA_SUBSCRIPTION_H

namespace gamelink
{
	namespace schema
	{
		struct MUXY_GAMELINK_API SubscribeTopicRequestBody
		{
			string topic_id;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribeTopicRequestBody, "topic_id", topic_id);
		};

		struct MUXY_GAMELINK_API UnsubscribeTopicRequestBody
		{
			string topic_id;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(UnsubscribeTopicRequestBody, "topic_id", topic_id);
		};
	}
}

#endif
