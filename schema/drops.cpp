#include "schema/drops.h"
#include "config.h"

namespace gamelink
{
	namespace schema
	{
		GetDropsRequest::GetDropsRequest(const string& status)
		{
			action = string("get");
			params.target = string("drops");
			data.status = status;
		}

		ValidateDropsRequest::ValidateDropsRequest(const string& id)
		{
			action = string("validate");
			params.target = string("drops");
			data.id = id;
		}
	}
}
