#include "gamelink.h"

namespace gamelink
{
	namespace detail
	{
		bool SubscriptionSets::canRegisterSKU(const string& sku)
		{
			_lock.lock();
			for (size_t i = 0; i < _skus.size(); ++i)
			{
				if (_skus[i].target == sku && _skus[i].state == SubscriptionState::Active)
				{
					_lock.unlock();
					return false;
				}
			}

			_lock.unlock();
			return true;
		}

		void SubscriptionSets::registerSKU(const string& sku)
		{
			SubscriptionWithString sub;
			sub.state = SubscriptionState::Active;
			sub.target = sku;

			_lock.lock();
			_skus.push_back(sub);
			_lock.unlock();
		}

		void SubscriptionSets::unregisterSKU(const string& sku)
		{
			_lock.lock();
			auto it = std::remove_if(_skus.begin(), _skus.end(), [&](const SubscriptionWithString& value)
			{
				return value.target == sku && value.state == SubscriptionState::Active;
			});

			_skus.erase(it, _skus.end());
			_lock.unlock();
		}

		bool SubscriptionSets::canRegisterPoll(const string& pid)
		{
			_lock.lock();
			for (size_t i = 0; i < _polls.size(); ++i)
			{
				if (_polls[i].target == pid && _polls[i].state == SubscriptionState::Active)
				{
					_lock.unlock();
					return false;
				}
			}

			_lock.unlock();
			return true;
		}

		void SubscriptionSets::registerPoll(const string& pid)
		{
			SubscriptionWithString sub;
			sub.state = SubscriptionState::Active;
			sub.target = pid;


			_lock.lock();
			_polls.push_back(sub);
			_lock.unlock();
		}

		void SubscriptionSets::unregisterPoll(const string& pid)
		{
			_lock.lock();
			auto it = std::remove_if(_polls.begin(), _polls.end(), [&](const SubscriptionWithString& value)
			{
				return value.target == pid && value.state == SubscriptionState::Active;
			});

			_polls.erase(it, _polls.end());
			_lock.unlock();
		}

		bool SubscriptionSets::canRegisterStateUpdate(StateTarget target)
		{
			if (!IsValidStateTarget(target))
			{
				return false;
			}


			_lock.lock();
			if (_stateSubscriptions[static_cast<int>(target)].state == SubscriptionState::Inactive)
			{
				_lock.unlock();
				return true;
			}

			_lock.unlock();
			return false;
		}

		void SubscriptionSets::registerStateUpdates(StateTarget target)
		{
			if (!IsValidStateTarget(target))
			{
				return;
			}

			_lock.lock();
			_stateSubscriptions[static_cast<int>(target)].state = SubscriptionState::Active;
			_lock.unlock();
		}

		void SubscriptionSets::unregisterStateUpdates(StateTarget target)
		{
			if (!IsValidStateTarget(target))
			{
				return;
			}

			_lock.lock();
			_stateSubscriptions[static_cast<int>(target)].state = SubscriptionState::Inactive;
			_lock.unlock();
		}


		bool SubscriptionSets::canRegisterConfigurationChanges(ConfigTarget target)
		{
			if (!IsValidConfigTarget(target))
			{
				return false;
			}

			_lock.lock();
			if (_configurationChanges[static_cast<int>(target)].state == SubscriptionState::Inactive)
			{
				_lock.unlock();
				return true;
			}

			_lock.unlock();
			return false;
		}

		void SubscriptionSets::registerConfigurationChanges(ConfigTarget target)
		{
			if (!IsValidConfigTarget(target))
			{
				return;
			}

			_lock.lock();
			_configurationChanges[static_cast<int>(target)].state = SubscriptionState::Active;
			_lock.unlock();
		}

		void SubscriptionSets::unregisterConfigurationChanges(ConfigTarget target)
		{
			if (!IsValidConfigTarget(target))
			{
				return;
			}

			_lock.lock();
			_configurationChanges[static_cast<int>(target)].state = SubscriptionState::Inactive;
			_lock.unlock();
		}

		bool SubscriptionSets::canRegisterDatastream()
		{
			return _datastream.state == SubscriptionState::Inactive;
		}

		void SubscriptionSets::registerDatastream()
		{
			_lock.lock();
			_datastream.state = SubscriptionState::Active;
			_lock.unlock();
		}

		void SubscriptionSets::unregisterDatastream()
		{
			_lock.lock();
			_datastream.state = SubscriptionState::Inactive;
			_lock.unlock();
		}

		bool SubscriptionSets::canRegisterMatchmakingQueueInvite()
		{
			return _matchmakingInvite.state == SubscriptionState::Inactive;
		}

		void SubscriptionSets::registerMatchmakingQueueInvite()
		{
			_lock.lock();
			_matchmakingInvite.state = SubscriptionState::Active;
			_lock.unlock();
		}

		void SubscriptionSets::unregisterMatchmakingQueueInvite()
		{
			_lock.lock();
			_matchmakingInvite.state = SubscriptionState::Inactive;
			_lock.unlock();
		}
	}
}