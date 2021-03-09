
#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_SERIALIZATION_H
#define MUXY_GAMELINK_SCHEMA_SERIALIZATION_H
#include <nlohmann/json.hpp>

#define MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, name, out, property) \
	{ nlohmann::json::const_iterator it = in.find(name); if (it != in.end()) { it->get_to(out. property); }}


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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
	}

#define MUXY_GAMELINK_SERIALIZE_2(Type, n1, p1, n2, p2) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
	}

#define MUXY_GAMELINK_SERIALIZE_3(Type, n1, p1, n2, p2, n3, p3) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
	}

#define MUXY_GAMELINK_SERIALIZE_4(Type, n1, p1, n2, p2, n3, p3, n4, p4) \
    inline void to_json(nlohmann::json& out, const Type& p) {      \
		out[n1] = p. p1; \
		out[n2] = p. p2; \
		out[n3] = p. p3; \
		out[n4] = p. p4; \
	} \
    inline void from_json(const nlohmann::json& in, Type& p) {     \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n6, p, p6) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n6, p, p6) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n7, p, p7) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n6, p, p6) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n7, p, p7) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n8, p, p8) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n6, p, p6) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n7, p, p7) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n8, p, p8) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n9, p, p9) \
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
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n1, p, p1) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n2, p, p2) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n3, p, p3) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n4, p, p4) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n5, p, p5) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n6, p, p6) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n7, p, p7) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n8, p, p8) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n9, p, p9) \
		MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, n10, p, p10) \
	}

#endif
