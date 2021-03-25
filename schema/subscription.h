#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_SUBSCRIPTION_H
#define MUXY_GAMELINK_SCHEMA_SUBSCRIPTION_H

namespace gamelink
{
	namespace schema
	{
		namespace bodies
		{
			struct SubscribeTopicBody
			{
				string topic_id;

				MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(SubscribeTopicBody, "topic_id", topic_id);
			};
		}
	}
}

#endif
