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

template<>
struct std::hash<gamelink::string>
{
	inline std::size_t operator()(const gamelink::string& str) const
	{
		return std::hash<
			std::string_view
		>()(std::string_view(str.c_str(), str.size()));
	}
};
#endif

namespace nlohmann
{
	template<typename T>
	struct adl_serializer<std::unordered_map<gamelink::string, T>>
	{
		static inline void to_json(json& js, const std::unordered_map<gamelink::string, T>& s)
		{
			for (auto it = s.begin(); it != s.end(); ++it)
			{
				js[json::string_t(it->first.c_str())] = it->second;
			}
		}

		static inline void from_json(const json& j, std::unordered_map<gamelink::string, T>& s)
		{
			if (!j.is_object())
			{
				return;
			}

			s.clear();
			for (auto it = j.begin(); it != j.end(); ++it)
			{
				gamelink::string key(it.key().c_str());

				T value;
				it.value().get_to(value);

				s.insert(std::make_pair(key, std::move(value)));
			}
		}
	};
}