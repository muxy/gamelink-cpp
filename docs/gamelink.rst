gamelink - Documentation
====================================

General
-------

Configuration
~~~~~~~~~~~~~~
.. doxygentypedef:: string


Envelopes
~~~~~~~~~~~~~~
.. doxygenstruct:: gamelink::schema::ReceiveEnvelope
    :members:

.. doxygenstruct:: gamelink::schema::ReceiveMeta
    :members:

.. doxygenstruct:: gamelink::schema::Error 
    :members:

.. doxygenstruct:: gamelink::schema::SendEnvelope
    :members:

.. doxygenstruct:: gamelink::schema::SendParameters
    :members:

.. doxygenfunction:: gamelink::schema::to_json(nlohmann::json& out, const ReceiveEnvelope<T>& p)
.. doxygenfunction:: gamelink::schema::from_json(const nlohmann::json& in, ReceiveEnvelope<T>& p)

.. doxygenfunction:: gamelink::schema::to_json(nlohmann::json& out, const SendEnvelope<T>& p)
.. doxygenfunction:: gamelink::schema::from_json(const nlohmann::json& out, SendEnvelope<T>& p)

.. doxygenfunction:: gamelink::schema::ParseResponse
.. doxygenfunction:: gamelink::schema::ParseEnvelope

JSONAtom 
~~~~~~~~~~~~~
.. doxygenstruct:: gamelink::schema::JsonAtom
    :members:

.. doxygenenum:: gamelink::schema::JsonAtomType

.. doxygenfunction:: gamelink::schema::atomFromInteger
.. doxygenfunction:: gamelink::schema::atomFromDouble
.. doxygenfunction:: gamelink::schema::atomFromString
.. doxygenfunction:: gamelink::schema::atomNull


Generic Bodies
~~~~~~~~~~~~~~
.. doxygenstruct:: gamelink::schema::bodies::EmptyBody
.. doxygenstruct:: gamelink::schema::bodies::OKResponseBody
    :members:

Authentication
~~~~~~~~~~~~~~

.. doxygenstruct:: gamelink::schema::bodies::AuthenticateWithPINBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::AuthenticateWithJWTBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::JWTResponseBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::SubscribeAuthenticationRequest
    :members: data

.. doxygenstruct:: gamelink::schema::SubscribeAuthenticationResponse
    :members: data

.. doxygenstruct:: gamelink::schema::AuthenticateWithPINRequest
    :members: AuthenticateWithPINRequest, data

.. doxygenstruct:: gamelink::schema::AuthenticateWithJWTRequest
    :members: AuthenticateWithJWTRequest, data

.. doxygenstruct:: gamelink::schema::AuthenticateResponse
    :members: data