gamelink - Documentation
====================================

General
-------

Envelope
~~~~~~~~~~~~~~
.. doxygenenum:: gamelink::schema::JsonAtomType

.. doxygenstruct:: gamelink::schema::JsonAtom 
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::Error 
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::ReceiveMeta
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::Envelope
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::ReceiveEnvelope
    :members:
    :undoc-members:

.. doxygenfunction:: gamelink::schema::to_json(nlohmann::json &out, const ReceiveEnvelope<T> &p)

.. doxygenstruct:: gamelink::schema::SetStateRequest
    :members:
    :undoc-members:

.. doxygenstruct:: gamelink::schema::bodies::SetStateBody
    :members:
    :undoc-members: