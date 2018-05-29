


#pragma once

#include "Common.h"

namespace dev
{

namespace p2p
{


class Peer: public Node
{
	friend class Session;			friend class Host;		
	friend class RLPXHandshake;

public:
		Peer(Node const& _node): Node(_node) {}

	Peer(Peer const&);
	
	bool isOffline() const { return !m_session.lock(); }

	virtual bool operator<(Peer const& _p) const;
	
		int rating() const { return m_rating; }
	
		bool shouldReconnect() const;
	
		int failedAttempts() const { return m_failedAttempts; }

		DisconnectReason lastDisconnect() const { return m_lastDisconnect; }
	
		void noteSessionGood() { m_failedAttempts = 0; }
	
protected:
		unsigned fallbackSeconds() const;

	std::atomic<int> m_score{0};										std::atomic<int> m_rating{0};										
		
	std::chrono::system_clock::time_point m_lastConnected;
	std::chrono::system_clock::time_point m_lastAttempted;
	std::atomic<unsigned> m_failedAttempts{0};
	DisconnectReason m_lastDisconnect = NoDisconnect;	
		std::weak_ptr<Session> m_session;
};
using Peers = std::vector<Peer>;

}
}
