#ifndef INCLUDE_MUXY_GAMELINK_HPP
#define INCLUDE_MUXY_GAMELINK_HPP

#include <fmt/format.h>
#include <queue>
#include <nlohmann/json.hpp>

namespace Gamelink {
	class Send {
		public:
			Send(std::string data) {
				this->data = data;
			}

            std::string data;
	};

    class SDK {
        public:
			SDK() {};
			~SDK() {
				// Clean up unsent messages
				while (HasSends()) {
					Send* send = _sendQueue.front();
					_sendQueue.pop();
					delete send;
				}
			}

            void ReceiveMessage(std::string message) {
            }

			bool HasSends() {
				return _sendQueue.size() > 0;
			}

			void ForeachSend(const std::function<void(Send* send)>& networkCallback) {
				while (HasSends()) {
					Send* send = _sendQueue.front();
					_sendQueue.pop();

					networkCallback(send);

					// Clean up send
					delete send;
				}
			}

			void authenticate(const std::string& pin) {
				auto json = nlohmann::json::parse(fmt::format(R"({{
                    "action": "authenticate",
                    "data": {{
                        "pin": "{}"
                    }}
                }})", pin));

				auto send = new Send(json.dump());
                _sendQueue.push(send);
			}

            void subscribeTo(const std::string& target, const std::string& id) {
			}

			void unsubscribeFrom(const std::string& target, const std::string& id) {
			}

            void broadcast(const std::string& message, const std::vector<std::string>& ids) {
			}

            void createPoll(const std::string& id, const std::string& prompt, const std::vector<std::string>& options) {
			}

		private:
			std::queue<Send*> _sendQueue;
	};
}

#endif
