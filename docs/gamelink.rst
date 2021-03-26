gamelink - Documentation
====================================


Configuration
---------------
.. doxygentypedef:: string

Schema
-------
The schema library, contained in the namespace `gamelink::schema` are projections of the 
network protocol into C++ structs.

Unless otherwise stated, all strings are utf-8 encoded.

Serialization
~~~~~~~~~~~~~
To mark a type as serializable, use the macros ``MUXY_GAMELINK_SERIALIZE_[0..9]`` or
``MUXY_GAMELINK_SERIALIZE_INTRUSIVE_[0..9]``

These macros take arguments are similar to ``MUXY_GAMELINK_SERIALIZE_N(Type, Name1, Property1, Name2, Property2, ...)``

``MUXY_GAMELINK_SERIALIZE_`` should be placed in the same namespace as ``Type``.

``MUXY_GAMELINK_SERIALIZE_INTRUSIVE_`` should be placed in the declaration of the Type.


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

State
~~~~~~~
State types are designed to wrap a user-provided type. 

Each body and Request/Response type are designed to be templated
by the user-developer type.

.. doxygenvariable:: gamelink::schema::STATE_TARGET_CHANNEL
.. doxygenvariable:: gamelink::schema::STATE_TARGET_EXTENSION

.. doxygenstruct:: gamelink::schema::bodies::SetStateBody
    :members:

.. doxygenstruct:: gamelink::schema::bodies::StateResponse
    :members:

.. doxygenstruct:: gamelink::schema::SetStateRequest
    :members: SetStateRequest, data

.. doxygenstruct:: gamelink::schema::SetStateResponse
    :members: data

.. doxygenstruct:: gamelink::schema::GetStateRequest
    :members: GetStateRequest, data

.. doxygenstruct:: gamelink::schema::GetStateResponse
    :members: data

.. doxygenstruct:: gamelink::schema::bodies::UpdateOperation
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::UpdateStateBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::UpdateStateRequest
    :members: UpdateStateRequest, data

.. doxygenstruct:: gamelink::schema::bodies::SubscribeStateBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::StateSubscriptionUpdate
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::SubscribeStateRequest
    :members: SubscribeStateRequest, data

Polling 
~~~~~~~~~
.. doxygenstruct:: gamelink::schema::bodies::GetPollBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::CreatePollBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::CreateUserDataPollBody
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::GetPollRequest
    :members: GetPollRequest, data

.. doxygenstruct:: gamelink::schema::CreatePollRequest
    :members: CreatePollRequest, data

.. doxygenstruct:: gamelink::schema::CreateUserDataPollRequest
    :members: CreateUserDataPollRequest, data