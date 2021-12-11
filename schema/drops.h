#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_DROPS_H
#define MUXY_GAMELINK_SCHEMA_DROPS_H
#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{
		struct MUXY_GAMELINK_API Drop
		{
			string id;
			string benefitId;
			string userId;
			string status;
			string service;
			string updatedAt;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_6(Drop, 
				"id", id, 
				"benefit_id", benefitId, 
				"user_id", userId, 
				"fulfillment_status", status, 
				"service", service,
				"last_updated", updatedAt);
		};

		struct MUXY_GAMELINK_API DropsContainer
		{
			std::vector<Drop> drops; 

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(DropsContainer, 
				"drops", drops);
		};

		struct MUXY_GAMELINK_API GetDropsResponse : ReceiveEnvelope<DropsContainer>
		{};

		struct MUXY_GAMELINK_API GetDropsRequestBody 
		{
			string status;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(GetDropsRequestBody, "status", status);
		};

		struct MUXY_GAMELINK_API GetDropsRequest : SendEnvelope<GetDropsRequestBody>
		{
			explicit GetDropsRequest(const string& status);	
		};

		struct MUXY_GAMELINK_API ValidateDropsRequestBody
		{
			string id;
			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_1(ValidateDropsRequestBody, "id", id);
		};

		struct MUXY_GAMELINK_API ValidateDropsRequest : SendEnvelope<ValidateDropsRequestBody>
		{
			explicit ValidateDropsRequest(const string& id);	
		};
	}
}
#endif