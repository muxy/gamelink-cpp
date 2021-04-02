#include "websocket.h"

#include <libwebsockets.h>
#include <mutex>

// Most of this implementation is referenced from
// https://github.com/warmcat/libwebsockets/blob/master/minimal-examples/ws-client/minimal-ws-client-echo/protocol_lws_minimal_client_echo.c

// Debugging info
const char* reasonToString(uint32_t reason)
{
	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		return "LWS_CALLBACK_PROTOCOL_INIT";
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		return "LWS_CALLBACK_PROTOCOL_DESTROY";
	case LWS_CALLBACK_WSI_CREATE:
		return "LWS_CALLBACK_WSI_CREATE";
	case LWS_CALLBACK_WSI_DESTROY:
		return "LWS_CALLBACK_WSI_DESTROY";
	case LWS_CALLBACK_WSI_TX_CREDIT_GET:
		return "LWS_CALLBACK_WSI_TX_CREDIT_GET";
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
		return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS";
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
		return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS";
	case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
		return "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION";
	case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
		return "LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY";
	case LWS_CALLBACK_SSL_INFO:
		return "LWS_CALLBACK_SSL_INFO";
	case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
		return "LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION";
	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		return "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED";
	case LWS_CALLBACK_HTTP:
		return "LWS_CALLBACK_HTTP";
	case LWS_CALLBACK_HTTP_BODY:
		return "LWS_CALLBACK_HTTP_BODY";
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		return "LWS_CALLBACK_HTTP_BODY_COMPLETION";
	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
		return "LWS_CALLBACK_HTTP_FILE_COMPLETION";
	case LWS_CALLBACK_HTTP_WRITEABLE:
		return "LWS_CALLBACK_HTTP_WRITEABLE";
	case LWS_CALLBACK_CLOSED_HTTP:
		return "LWS_CALLBACK_CLOSED_HTTP";
	case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
		return "LWS_CALLBACK_FILTER_HTTP_CONNECTION";
	case LWS_CALLBACK_ADD_HEADERS:
		return "LWS_CALLBACK_ADD_HEADERS";
	case LWS_CALLBACK_VERIFY_BASIC_AUTHORIZATION:
		return "LWS_CALLBACK_VERIFY_BASIC_AUTHORIZATION";
	case LWS_CALLBACK_CHECK_ACCESS_RIGHTS:
		return "LWS_CALLBACK_CHECK_ACCESS_RIGHTS";
	case LWS_CALLBACK_PROCESS_HTML:
		return "LWS_CALLBACK_PROCESS_HTML";
	case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
		return "LWS_CALLBACK_HTTP_BIND_PROTOCOL";
	case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
		return "LWS_CALLBACK_HTTP_DROP_PROTOCOL";
	case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
		return "LWS_CALLBACK_HTTP_CONFIRM_UPGRADE";
	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
		return "LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP";
	case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
		return "LWS_CALLBACK_CLOSED_CLIENT_HTTP";
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
		return "LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ";
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
		return "LWS_CALLBACK_RECEIVE_CLIENT_HTTP";
	case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
		return "LWS_CALLBACK_COMPLETED_CLIENT_HTTP";
	case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
		return "LWS_CALLBACK_CLIENT_HTTP_WRITEABLE";
	case LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL:
		return "LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL";
	case LWS_CALLBACK_CLIENT_HTTP_DROP_PROTOCOL:
		return "LWS_CALLBACK_CLIENT_HTTP_DROP_PROTOCOL";
	case LWS_CALLBACK_ESTABLISHED:
		return "LWS_CALLBACK_ESTABLISHED";
	case LWS_CALLBACK_CLOSED:
		return "LWS_CALLBACK_CLOSED";
	case LWS_CALLBACK_SERVER_WRITEABLE:
		return "LWS_CALLBACK_SERVER_WRITEABLE";
	case LWS_CALLBACK_RECEIVE:
		return "LWS_CALLBACK_RECEIVE";
	case LWS_CALLBACK_RECEIVE_PONG:
		return "LWS_CALLBACK_RECEIVE_PONG";
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		return "LWS_CALLBACK_WS_PEER_INITIATED_CLOSE";
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		return "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION";
	case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
		return "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY";
	case LWS_CALLBACK_WS_SERVER_BIND_PROTOCOL:
		return "LWS_CALLBACK_WS_SERVER_BIND_PROTOCOL";
	case LWS_CALLBACK_WS_SERVER_DROP_PROTOCOL:
		return "LWS_CALLBACK_WS_SERVER_DROP_PROTOCOL";
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		return "LWS_CALLBACK_CLIENT_CONNECTION_ERROR";
	case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
		return "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH";
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		return "LWS_CALLBACK_CLIENT_ESTABLISHED";
	case LWS_CALLBACK_CLIENT_CLOSED:
		return "LWS_CALLBACK_CLIENT_CLOSED";
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		return "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER";
	case LWS_CALLBACK_CLIENT_RECEIVE:
		return "LWS_CALLBACK_CLIENT_RECEIVE";
	case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
		return "LWS_CALLBACK_CLIENT_RECEIVE_PONG";
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		return "LWS_CALLBACK_CLIENT_WRITEABLE";
	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		return "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED";
	case LWS_CALLBACK_WS_EXT_DEFAULTS:
		return "LWS_CALLBACK_WS_EXT_DEFAULTS";
	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
		return "LWS_CALLBACK_FILTER_NETWORK_CONNECTION";
	case LWS_CALLBACK_WS_CLIENT_BIND_PROTOCOL:
		return "LWS_CALLBACK_WS_CLIENT_BIND_PROTOCOL";
	case LWS_CALLBACK_WS_CLIENT_DROP_PROTOCOL:
		return "LWS_CALLBACK_WS_CLIENT_DROP_PROTOCOL";
	case LWS_CALLBACK_GET_THREAD_ID:
		return "LWS_CALLBACK_GET_THREAD_ID";
	case LWS_CALLBACK_ADD_POLL_FD:
		return "LWS_CALLBACK_ADD_POLL_FD";
	case LWS_CALLBACK_DEL_POLL_FD:
		return "LWS_CALLBACK_DEL_POLL_FD";
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
		return "LWS_CALLBACK_CHANGE_MODE_POLL_FD";
	case LWS_CALLBACK_LOCK_POLL:
		return "LWS_CALLBACK_LOCK_POLL";
	case LWS_CALLBACK_UNLOCK_POLL:
		return "LWS_CALLBACK_UNLOCK_POLL";
	case LWS_CALLBACK_CGI:
		return "LWS_CALLBACK_CGI";
	case LWS_CALLBACK_CGI_TERMINATED:
		return "LWS_CALLBACK_CGI_TERMINATED";
	case LWS_CALLBACK_CGI_STDIN_DATA:
		return "LWS_CALLBACK_CGI_STDIN_DATA";
	case LWS_CALLBACK_CGI_STDIN_COMPLETED:
		return "LWS_CALLBACK_CGI_STDIN_COMPLETED";
	case LWS_CALLBACK_CGI_PROCESS_ATTACH:
		return "LWS_CALLBACK_CGI_PROCESS_ATTACH";
	case LWS_CALLBACK_SESSION_INFO:
		return "LWS_CALLBACK_SESSION_INFO";
	case LWS_CALLBACK_GS_EVENT:
		return "LWS_CALLBACK_GS_EVENT";
	case LWS_CALLBACK_HTTP_PMO:
		return "LWS_CALLBACK_HTTP_PMO";
	case LWS_CALLBACK_RAW_PROXY_CLI_RX:
		return "LWS_CALLBACK_RAW_PROXY_CLI_RX";
	case LWS_CALLBACK_RAW_PROXY_SRV_RX:
		return "LWS_CALLBACK_RAW_PROXY_SRV_RX";
	case LWS_CALLBACK_RAW_PROXY_CLI_CLOSE:
		return "LWS_CALLBACK_RAW_PROXY_CLI_CLOSE";
	case LWS_CALLBACK_RAW_PROXY_SRV_CLOSE:
		return "LWS_CALLBACK_RAW_PROXY_SRV_CLOSE";
	case LWS_CALLBACK_RAW_PROXY_CLI_WRITEABLE:
		return "LWS_CALLBACK_RAW_PROXY_CLI_WRITEABLE";
	case LWS_CALLBACK_RAW_PROXY_SRV_WRITEABLE:
		return "LWS_CALLBACK_RAW_PROXY_SRV_WRITEABLE";
	case LWS_CALLBACK_RAW_PROXY_CLI_ADOPT:
		return "LWS_CALLBACK_RAW_PROXY_CLI_ADOPT";
	case LWS_CALLBACK_RAW_PROXY_SRV_ADOPT:
		return "LWS_CALLBACK_RAW_PROXY_SRV_ADOPT";
	case LWS_CALLBACK_RAW_PROXY_CLI_BIND_PROTOCOL:
		return "LWS_CALLBACK_RAW_PROXY_CLI_BIND_PROTOCOL";
	case LWS_CALLBACK_RAW_PROXY_SRV_BIND_PROTOCOL:
		return "LWS_CALLBACK_RAW_PROXY_SRV_BIND_PROTOCOL";
	case LWS_CALLBACK_RAW_PROXY_CLI_DROP_PROTOCOL:
		return "LWS_CALLBACK_RAW_PROXY_CLI_DROP_PROTOCOL";
	case LWS_CALLBACK_RAW_PROXY_SRV_DROP_PROTOCOL:
		return "LWS_CALLBACK_RAW_PROXY_SRV_DROP_PROTOCOL";
	case LWS_CALLBACK_RAW_RX:
		return "LWS_CALLBACK_RAW_RX";
	case LWS_CALLBACK_RAW_CLOSE:
		return "LWS_CALLBACK_RAW_CLOSE";
	case LWS_CALLBACK_RAW_WRITEABLE:
		return "LWS_CALLBACK_RAW_WRITEABLE";
	case LWS_CALLBACK_RAW_ADOPT:
		return "LWS_CALLBACK_RAW_ADOPT";
	case LWS_CALLBACK_RAW_CONNECTED:
		return "LWS_CALLBACK_RAW_CONNECTED";
	case LWS_CALLBACK_RAW_SKT_BIND_PROTOCOL:
		return "LWS_CALLBACK_RAW_SKT_BIND_PROTOCOL";
	case LWS_CALLBACK_RAW_SKT_DROP_PROTOCOL:
		return "LWS_CALLBACK_RAW_SKT_DROP_PROTOCOL";
	case LWS_CALLBACK_RAW_ADOPT_FILE:
		return "LWS_CALLBACK_RAW_ADOPT_FILE";
	case LWS_CALLBACK_RAW_RX_FILE:
		return "LWS_CALLBACK_RAW_RX_FILE";
	case LWS_CALLBACK_RAW_WRITEABLE_FILE:
		return "LWS_CALLBACK_RAW_WRITEABLE_FILE";
	case LWS_CALLBACK_RAW_CLOSE_FILE:
		return "LWS_CALLBACK_RAW_CLOSE_FILE";
	case LWS_CALLBACK_RAW_FILE_BIND_PROTOCOL:
		return "LWS_CALLBACK_RAW_FILE_BIND_PROTOCOL";
	case LWS_CALLBACK_RAW_FILE_DROP_PROTOCOL:
		return "LWS_CALLBACK_RAW_FILE_DROP_PROTOCOL";
	case LWS_CALLBACK_TIMER:
		return "LWS_CALLBACK_TIMER";
	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
		return "LWS_CALLBACK_EVENT_WAIT_CANCELLED";
	case LWS_CALLBACK_CHILD_CLOSING:
		return "LWS_CALLBACK_CHILD_CLOSING";
	case LWS_CALLBACK_VHOST_CERT_AGING:
		return "LWS_CALLBACK_VHOST_CERT_AGING";
	case LWS_CALLBACK_VHOST_CERT_UPDATE:
		return "LWS_CALLBACK_VHOST_CERT_UPDATE";
	case LWS_CALLBACK_MQTT_NEW_CLIENT_INSTANTIATED:
		return "LWS_CALLBACK_MQTT_NEW_CLIENT_INSTANTIATED";
	case LWS_CALLBACK_MQTT_IDLE:
		return "LWS_CALLBACK_MQTT_IDLE";
	case LWS_CALLBACK_MQTT_CLIENT_ESTABLISHED:
		return "LWS_CALLBACK_MQTT_CLIENT_ESTABLISHED";
	case LWS_CALLBACK_MQTT_SUBSCRIBED:
		return "LWS_CALLBACK_MQTT_SUBSCRIBED";
	case LWS_CALLBACK_MQTT_CLIENT_WRITEABLE:
		return "LWS_CALLBACK_MQTT_CLIENT_WRITEABLE";
	case LWS_CALLBACK_MQTT_CLIENT_RX:
		return "LWS_CALLBACK_MQTT_CLIENT_RX";
	case LWS_CALLBACK_MQTT_UNSUBSCRIBED:
		return "LWS_CALLBACK_MQTT_UNSUBSCRIBED";
	case LWS_CALLBACK_MQTT_DROP_PROTOCOL:
		return "LWS_CALLBACK_MQTT_DROP_PROTOCOL";
	case LWS_CALLBACK_MQTT_CLIENT_CLOSED:
		return "LWS_CALLBACK_MQTT_CLIENT_CLOSED";
	case LWS_CALLBACK_MQTT_ACK:
		return "LWS_CALLBACK_MQTT_ACK";
	case LWS_CALLBACK_MQTT_RESEND:
		return "LWS_CALLBACK_MQTT_RESEND";
	case LWS_CALLBACK_USER:
		return "LWS_CALLBACK_USER";
	}
	return "UNKNOWN";
}

struct Message
{
	std::vector<uint8_t> data;
	bool binary;
	bool first;
	bool final;
};

int callbackEmptyProtocol(lws* wsi, lws_callback_reasons reasons, void* user, void* in, size_t len);
void connect(lws_sorted_usec_list_t* queue);

struct Impl
{
	Impl(const std::string& dHost, uint16_t dPort, WebsocketConnection* conn)
		: lwsCtx(nullptr)
		, client(nullptr)
	{
		memset(&queue, 0, sizeof(queue));

		// Setup the blank protocol to be handled by a generic callback
		protocols[0] = {"", callbackEmptyProtocol, 0, 0};
		protocols[1] = {NULL, NULL, 0, 0};

		// Setup retry policy
		memset(&retryPolicy, 0, sizeof(retryPolicy));
		retryPolicy.secs_since_valid_ping = 3;
		retryPolicy.secs_since_valid_hangup = 10;

		// Creation options
		lws_context_creation_info creation;
		memset(&creation, 0, sizeof(creation));

		creation.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		creation.port = CONTEXT_PORT_NO_LISTEN;
		creation.protocols = protocols;
		creation.fd_limit_per_thread = 3;

		lwsCtx = lws_create_context(&creation);
		if (!lwsCtx)
		{
			// Error, not recoverable.
			std::terminate();
		}

		// Schedule a connection in 100 usecs.
		lws_sul_schedule(lwsCtx, 0, &queue, connect, 100);

		host = dHost;
		port = dPort;
	}

	// Very important that this member is first, because lws_sul_schedule doesn't
	// have a user pointer, but rather gives a pointer to the usec_list, which
	// can then be cast into a pointer to Impl.
	lws_sorted_usec_list_t queue;

	lws_context* lwsCtx;
	lws* client;

	lws_retry_bo_t retryPolicy;
	lws_protocols protocols[2];

	// I don't want to deal with the lws ring buffer, so here is instead a locked
	// vector to simulate one.
	std::mutex lock;
	std::vector<std::unique_ptr<Message>> messages;

	std::function<void(const char*, uint32_t)> callback;

	std::string host;
	uint16_t port;
};

int callbackEmptyProtocol(lws* wsi, lws_callback_reasons reasons, void* user, void* in, size_t len)
{
	Impl* impl = reinterpret_cast<Impl*>(lws_get_opaque_user_data(wsi));

	switch (reasons)
	{
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		// TODO: Handle connection error
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE: {
		// Pop a value and write.
		std::unique_ptr<Message> msg;
		if (impl->messages.size())
		{
			impl->lock.lock();
			msg = std::move(impl->messages.front());
			impl->messages.erase(impl->messages.begin());
			impl->lock.unlock();
		}

		if (msg)
		{
			// The data parameter to lws_write is a pointer to the
			// data bytes, without the LWS_PRE buffer.

			// The length parameter to lws_write is the length of
			// the data bytes, without the LWS_PRE bytes
			int res = lws_write(impl->client, msg->data.data() + LWS_PRE, msg->data.size() - LWS_PRE,
								msg->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);

			if (res < msg->data.size() - LWS_PRE)
			{
				// TODO: Handle error writing to socket
				return -1;
			}
		}

		// Keep servicing this callback
		lws_callback_on_writable(impl->client);
		break;
	}
	case LWS_CALLBACK_CLIENT_RECEIVE: {
		const char* begin = reinterpret_cast<const char*>(in);
		if (impl->callback)
		{
			impl->callback(begin, len);
		}

		break;
	}
	default:
		break;
	}
	return lws_callback_http_dummy(wsi, reasons, user, in, len);
}

void connect(lws_sorted_usec_list_t* queue)
{
	Impl* impl = reinterpret_cast<Impl*>(queue);

	// Setup connection information
	lws_client_connect_info info;
	memset(&info, 0, sizeof(info));

	info.context = impl->lwsCtx;

	info.port = impl->port;
	info.address = impl->host.c_str();

	// Likely need to attempt to do this and then fallback to insecure if needed.
	// info.ssl_connection = LCCSCF_USE_SSL;

	// Setup a user_data that points to the impl instance for access in the protocol callbacks
	info.opaque_user_data = impl;

	info.path = "/";
	info.host = info.address;
	info.origin = info.address;

	info.protocol = "";
	info.alpn = "h2;http/1.1";
	info.local_protocol_name = "";
	info.pwsi = &impl->client;
	info.retry_and_idle_policy = &impl->retryPolicy;

	if (!lws_client_connect_via_info(&info))
	{
		// Failed, retry in 5 seconds. There is no functionality to stop connection attempts.
		lws_sul_schedule(impl->lwsCtx, 0, queue, connect, 5 * LWS_USEC_PER_SEC);
	}
}

WebsocketConnection::~WebsocketConnection()
{
	delete impl;
}

WebsocketConnection::WebsocketConnection(const std::string& host, uint16_t port)
{
	impl = new Impl(host, port, this);
}

int WebsocketConnection::run()
{
	// Pump the event loop
	return lws_service(impl->lwsCtx, 0);
}

void WebsocketConnection::terminate()
{
	lws_context_destroy(impl->lwsCtx);
}

void WebsocketConnection::send(const char* data, uint32_t length)
{
	// Don't send more than 8mb in a single send. This value
	// should be closer to 8k, but larger packets should be fine
	// up to a point.
	if (length > 1024 * 1024 * 8)
	{
		return;
	}

	Message* msg = new Message();
	// The documentation for lws_write on websockets requires a LWS_PRE
	// amount of space before the message proper to insert protocol information.
	// Allocate that here, and place the data after the LWS_PRE bytes for access later.
	msg->data.resize(length + LWS_PRE);
	memcpy(&msg->data[0] + LWS_PRE, data, length);
	msg->binary = false;

	// Unsure if these are actually needed, but are in the example.
	// msg->first = lws_is_first_fragment(impl->client);
	// msg->final = lws_is_final_fragment(impl->client);

	impl->lock.lock();
	impl->messages.emplace_back(std::unique_ptr<Message>(msg));
	impl->lock.unlock();

	// Request a writable callback.
	if (impl->client)
	{
		lws_callback_on_writable(impl->client);
	}
}

void WebsocketConnection::onMessage(std::function<void(const char*, uint32_t)> cb)
{
	impl->callback = cb;
}
