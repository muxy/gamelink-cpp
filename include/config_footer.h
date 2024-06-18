#pragma once
#ifdef MUXY_GAMELINK_REQUIRES_STRING_JSON_OVERLOAD
namespace nlohmann
{
	template<>
	struct adl_serializer<gamelink::string>
	{
		static void to_json(json& j, const gamelink::string& s)
		{
			j = json::string_t(s.c_str());
		}

		static void from_json(const json& j, gamelink::string& s)
		{
			s = gamelink::string(j.get<json::string_t>().c_str());
		}
	};
}
#endif