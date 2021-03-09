
#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_SERIALIZATION_H
#define MUXY_GAMELINK_SCHEMA_SERIALIZATION_H
#include <nlohmann/json.hpp>


#define MUXY_GAMELINK_SERIALIZE_0(Type) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
	}

#define MUXY_GAMELINK_SERIALIZE_1(Type, n1, p1) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
	}

#define MUXY_GAMELINK_SERIALIZE_2(Type, n1, p1, n2, p2) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
	}

#define MUXY_GAMELINK_SERIALIZE_3(Type, n1, p1, n2, p2, n3, p3) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
	}

#define MUXY_GAMELINK_SERIALIZE_4(Type, n1, p1, n2, p2, n3, p3, n4, p4) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
	}

#define MUXY_GAMELINK_SERIALIZE_5(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
	}

#define MUXY_GAMELINK_SERIALIZE_6(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5, n6, p6) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
		out[n6] = p. p6; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
		if (in.contains(n6)) in.at(n6).get_to(p. p6); \
	}

#define MUXY_GAMELINK_SERIALIZE_7(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5, n6, p6, n7, p7) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
		out[n6] = p. p6; \
		out[n7] = p. p7; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
		if (in.contains(n6)) in.at(n6).get_to(p. p6); \
		if (in.contains(n7)) in.at(n7).get_to(p. p7); \
	}

#define MUXY_GAMELINK_SERIALIZE_8(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5, n6, p6, n7, p7, n8, p8) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
		out[n6] = p. p6; \
		out[n7] = p. p7; \
		out[n8] = p. p8; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
		if (in.contains(n6)) in.at(n6).get_to(p. p6); \
		if (in.contains(n7)) in.at(n7).get_to(p. p7); \
		if (in.contains(n8)) in.at(n8).get_to(p. p8); \
	}

#define MUXY_GAMELINK_SERIALIZE_9(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5, n6, p6, n7, p7, n8, p8, n9, p9) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
		out[n6] = p. p6; \
		out[n7] = p. p7; \
		out[n8] = p. p8; \
		out[n9] = p. p9; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
		if (in.contains(n6)) in.at(n6).get_to(p. p6); \
		if (in.contains(n7)) in.at(n7).get_to(p. p7); \
		if (in.contains(n8)) in.at(n8).get_to(p. p8); \
		if (in.contains(n9)) in.at(n9).get_to(p. p9); \
	}

#define MUXY_GAMELINK_SERIALIZE_10(Type, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5, n6, p6, n7, p7, n8, p8, n9, p9, n10, p10) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
		out[n5] = p. p5; \
		out[n6] = p. p6; \
		out[n7] = p. p7; \
		out[n8] = p. p8; \
		out[n9] = p. p9; \
		out[n10] = p. p10; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		if (in.contains(n1)) in.at(n1).get_to(p. p1); \
		if (in.contains(n2)) in.at(n2).get_to(p. p2); \
		if (in.contains(n3)) in.at(n3).get_to(p. p3); \
		if (in.contains(n4)) in.at(n4).get_to(p. p4); \
		if (in.contains(n5)) in.at(n5).get_to(p. p5); \
		if (in.contains(n6)) in.at(n6).get_to(p. p6); \
		if (in.contains(n7)) in.at(n7).get_to(p. p7); \
		if (in.contains(n8)) in.at(n8).get_to(p. p8); \
		if (in.contains(n9)) in.at(n9).get_to(p. p9); \
		if (in.contains(n10)) in.at(n10).get_to(p. p10); \
	}

#endif
