#pragma once
#ifndef MUXY_GAMELINK_SCHEMA_ENVELOPE_H
#define MUXY_GAMELINK_SCHEMA_ENVELOPE_H
#include "config.h"
#include "schema/serialization.h"

namespace gamelink
{
	namespace schema
	{
		// A JSON atom reprsents one of the 4 supported base types of json value -
		// An integer, double, string, null
		enum JsonAtomType
		{
			JSON_ATOM_NULL,    //!< No fields are valid, the JSONAtom represents a null
			JSON_ATOM_INT64,   //!< The int64Value field is valid, and the JSONAtom represents an integer.
			JSON_ATOM_DOUBLE,  //!< The doubleValue field is valid, and the JSONAtom represents a double.
			JSON_ATOM_STRING,  //!< The stringValue field is valid, and the JSONAtom represents a string.
			JSON_ATOM_LITERAL, //!< The stringValue field is valid, and is a JSON encoded object or array.

			JSON_ATOM_FORCE_32 = 0xFFFFFFFF
		};

		/// JSONAtom is effectively a tagged union that can contain a signed 64-bit integer,
		/// a floating point double, a string, a json literal, or null. The type of a JSONAtom is stored in
		/// the `field` type.
		struct JsonAtom
		{
			/// Type of the contained value
			JsonAtomType type;

			/// Integer value
			int64_t int64Value;

			/// Double value
			double doubleValue;

			/// String value
			string stringValue;
		};

		/// Creates a JsonAtom that represents an integer
		/// @param[in] v Integer value
		/// @return JsonAtom that contains the input integer value
		JsonAtom atomFromInteger(int64_t v);

		/// Creates a JsonAtom that represents a double
		/// @param[in] d Double value
		/// @return JsonAtom that contains the input double value
		JsonAtom atomFromDouble(double d);

		/// Creates a JsonAtom that represents a string
		/// @param[in] str String value
		/// @return JsonAtom that contains the input string value
		JsonAtom atomFromString(const string& str);

		/// Creates a JsonAtom that represents an object
		/// @param[in] str JSON Literal
		/// @return JsonAtom that contains the input literal
		JsonAtom atomFromLiteral(const string& str);

		/// Creates a JsonAtom that represents null
		/// @return A null JsonAtom
		JsonAtom atomNull();

		void to_json(nlohmann::json& out, const JsonAtom& p);
		void from_json(const nlohmann::json& in, JsonAtom& p);

		/// Contains metadata fields about a response
		struct ReceiveMeta
		{
			ReceiveMeta();

			/// Request ID, as sent by the response
			uint16_t request_id;

			/// Action of the request
			string action;

			/// Target of the request
			string target;

			/// Unix timestamp in milliseconds since epoch.
			uint64_t timestamp;
		};

		MUXY_GAMELINK_SERIALIZE_4(ReceiveMeta, "request_id", request_id, "action", action, "target", target, "timestamp", timestamp)

		/// Error type, possibly returned by any API call.
		struct Error
		{
			/// Unsigned error code. Correlates to HTTP error codes.
			uint32_t number;

			/// Human readable error title.
			string title;

			/// Human readable detail field.
			string detail;
		};

		MUXY_GAMELINK_SERIALIZE_3(Error, "number", number, "title", title, "detail", detail)

		// Patch operation used to send patches
		struct PatchOperation
		{
			string operation;
			string path;
			JsonAtom value;

			MUXY_GAMELINK_SERIALIZE_INTRUSIVE_3(PatchOperation, "op", operation, "path", path, "value", value);
		};

		struct ReceiveEnvelopeCommon
		{
			/// Metadata about this response
			ReceiveMeta meta;

			/// Errors list. Mutually exclusive with data.
			/// May contain more than one error.
			std::vector<Error> errors;
		};

		/// ReceiveEnvelope
		template<typename T>
		struct ReceiveEnvelope : public ReceiveEnvelopeCommon
		{
			/// Data field. Will vary based on the response.
			/// See the type documentation for more details.
			T data;
		};

		template<typename T>
		void to_json(nlohmann::json& out, const ReceiveEnvelope<T>& p)
		{
			out["meta"] = p.meta;
			out["data"] = p.data;

			if (p.errors.size())
			{
				out["errors"] = p.errors;
			}
		}

		template<typename T>
		void from_json(const nlohmann::json& in, ReceiveEnvelope<T>& p)
		{
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "meta", p, meta);
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "data", p, data);
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "errors", p, errors);
		}

		struct SendParameters
		{
			SendParameters();

			/// Request ID. Will be echoed back in the response.
			/// By default, is 0xFFFF
			uint16_t request_id;

			/// Target field. Will vary in behavior based on action.
			string target;
		};

		inline void to_json(nlohmann::json& out, const SendParameters& p)
		{
			out["request_id"] = p.request_id;
			if (p.target.size())
			{
				out["target"] = p.target;
			}
		}

		inline void from_json(const nlohmann::json& in, SendParameters& p)
		{
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "request_id", p, request_id);
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "target", p, target);
		}

		template<typename T>
		struct SendEnvelope
		{
			/// The action of this request.
			string action;

			/// Parameters of this request.
			SendParameters params;

			/// Data field. Will vary based on the response.
			/// See the type documentation for more details.
			T data;
		};

		template<typename T>
		void to_json(nlohmann::json& out, const SendEnvelope<T>& p)
		{
			out["action"] = p.action;
			out["params"] = p.params;
			out["data"] = p.data;
		}

		template<typename T>
		void from_json(const nlohmann::json& in, SendEnvelope<T>& p)
		{
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "action", p, action);
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "params", p, params);
			MUXY_GAMELINK_DESERIALIZE_PROPERTY(in, "data", p, data);
		}

		template<typename T>
		std::string to_string(const SendEnvelope<T>& p)
		{
			nlohmann::json out;
			to_json(out, p);
			return out.dump();
		}

		/// The empty body. Has no members.
		struct EmptyBody
		{
		};

		/// OKResponseBody is sent back when a simple operation succeeds.
		struct OKResponseBody
		{
			/// Will always be 'true'. If an error occurred, then
			/// the errors array in the response would be set.
			bool ok;
		};

		MUXY_GAMELINK_SERIALIZE_1(OKResponseBody, "ok", ok)

		// Specialization for empty body serialization
		MUXY_GAMELINK_SERIALIZE_2(SendEnvelope<EmptyBody>, "action", action, "params", params);
		MUXY_GAMELINK_SERIALIZE_2(ReceiveEnvelope<EmptyBody>, "meta", meta, "errors", errors);

		/// Parse a response object
		/// @param[in] bytes JSON input bytes. Must not be null.
		/// @param[in] length Length of the bytes parameter
		/// @param[out] out Output object. Should be a ResponseEnvelope or a type inherited from ResponseEnvelope.
		/// @return true iff the input JSON parsed correctly, false otherwise
		template<typename T>
		bool ParseResponse(const char* bytes, uint32_t length, T& out)
		{
			nlohmann::json value = nlohmann::json::parse(bytes, bytes + length, nullptr, false);
			if (value.is_discarded())
			{
				return false;
			}

			from_json(value, out);
			return true;
		}

		/// Parses a ReceiveEnvelope only. Does not attempt to parse the body.
		/// @param[in] bytes JSON input bytes. Must not be null.
		/// @param[in] length Length of the bytes parameter
		/// @param[out] success Optional boolean to determine parse failure. Will be set to true iff the parse succeeded, false otherwise.
		/// @return A ReceiveEnvelope with no body, only metadata field and possibly errors.
		ReceiveEnvelope<EmptyBody> ParseEnvelope(const char* bytes, uint32_t length, bool* success = nullptr);
	}
}

#endif
