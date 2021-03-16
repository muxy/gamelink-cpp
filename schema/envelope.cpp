#include "schema/envelope.h"

namespace gamelink
{
	namespace schema
	{
		JsonAtom atomFromInteger(int64_t v)
		{
			JsonAtom atom;

			atom.type = JSON_ATOM_INT64;
			atom.int64Value = v;

			return atom;
		}

		JsonAtom atomFromDouble(double v)
		{
			JsonAtom atom;

			atom.type = JSON_ATOM_DOUBLE;
			atom.doubleValue = v;

			return atom;
		}

		JsonAtom atomFromString(const std::string& str)
		{
			JsonAtom atom;

			atom.type = JSON_ATOM_STRING;
			atom.stringValue = str;

			return atom;
		}

		JsonAtom atomNull(int64_t v)
		{
			JsonAtom atom;
			atom.type = JSON_ATOM_NULL;
			return atom;
		}

		void to_json(nlohmann::json& out, const JsonAtom& p)
		{
			switch (p.type)
			{
			case JSON_ATOM_NULL:
				out = nlohmann::json();
				break;
			case JSON_ATOM_INT64:
				out = p.int64Value;
				break;
			case JSON_ATOM_DOUBLE:
				out = p.doubleValue;
				break;
			case JSON_ATOM_STRING:
				out = p.stringValue;
				break;
			default:
				// Bad
				out = nlohmann::json();
				break;
			}
		}

		void from_json(const nlohmann::json& n, JsonAtom& p)
		{
			p.type = JSON_ATOM_NULL;
			if (n.is_null())
			{
				p.type = JSON_ATOM_NULL;
				return;
			}
			else if (n.is_string())
			{
				p.stringValue = n.get<string>();
				return;
			}
			else if (n.is_number_integer())
			{
				p.int64Value = n.get<int64_t>();
				return;
			}
			else if (n.is_number())
			{
				p.doubleValue = n.get<double>();
			}
		}

		ReceiveMeta::ReceiveMeta()
			: request_id(0)
			, timestamp(0)
		{
		}

		SendParameters::SendParameters()
			: request_id(0xFFFF)
		{
		}

		ReceiveEnvelope<bodies::EmptyBody> ParseEnvelope(std::string jsonString)
		{
			ReceiveEnvelope<bodies::EmptyBody> out;
			ParseResponse(jsonString, out);
			return out;
		}
	}
}
