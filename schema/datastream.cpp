#include "schema/datastream.h"
#include "config.h"

namespace gamelink
{
	namespace schema
	{
		SubscribeDatastreamRequest::SubscribeDatastreamRequest()
		{
			action = string("subscribe");
			params.target = string("datastream");
		}
	}
}
