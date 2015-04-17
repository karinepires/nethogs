#include <iostream>
#include <assert.h>
#include <malloc.h>
#include "nethogs.h"
#include "connection.h"

class ConnList
{
public:
	ConnList (Connection * m_val = NULL, ConnList * m_next = NULL)
	{
	    val = m_val; next = m_next;
	}
	Connection * val;
	ConnList * next;
};

ConnList * connections = NULL;

void PackList::add (Packet * p)
{
	if (content == NULL)
	{
		content = new PackListNode (new Packet (*p));
		return;
	}

	if (content->val->time.tv_sec == p->time.tv_sec)
	{
		content->val->len += p->len;
		return;
	}

	/* store copy of packet, so that original may be freed */
	content = new PackListNode(new Packet (*p), content);
}

/* sums up the total bytes used and removes 'old' packets */
u_int32_t PackList::sumanddel (timeval t)
{
	u_int32_t retval = 0;
	PackListNode * current = content;
	PackListNode * previous = NULL;

	while (current != NULL) 
	{
		//std::cout << "Comparing " << current->val->time.tv_sec << " <= " << t.tv_sec - PERIOD << endl;
		if (current->val->time.tv_sec <= t.tv_sec - PERIOD)
		{
			if (current == content)
				content = NULL;
			else if (previous != NULL)
				previous->next = NULL;
			delete current;
			return retval;
		}
		retval += current->val->len;
		previous = current;
		current = current->next;
	}
	return retval;
}

/* packet may be deleted by caller */
Connection::Connection (Packet * packet)
{
	assert (packet != NULL);
	connections = new ConnList (this, connections);
	sent_packets = new PackList ();
	recv_packets = new PackList ();
	sumSent = 0;
	sumRecv = 0;
	if (DEBUG)
	{
		std::cout << "New connection, with package len " << packet->len << std::endl;
	}
	if (packet->Outgoing())
	{
		sumSent += packet->len;
		sent_packets->add(packet);
		refpacket = new Packet (*packet);
	} else {
		sumRecv += packet->len;
		recv_packets->add(packet);
		refpacket = packet->newInverted();
	}
	lastpacket = packet->time.tv_sec;
	if (DEBUG)
		std::cout << "New reference packet created at " << refpacket << std::endl;
}

Connection::~Connection ()
{
	if (DEBUG)
		std::cout << "Deleting connection" << std::endl;
	/* refpacket is not a pointer to one of the packets in the lists
	 * so deleted */
	delete (refpacket);
    	if (sent_packets != NULL)
		delete sent_packets;
    	if (recv_packets != NULL)
		delete recv_packets;

	ConnList * curr_conn = connections;
	ConnList * prev_conn = NULL;
	while (curr_conn != NULL)
	{
		if (curr_conn->val == this)
		{
			ConnList * todelete = curr_conn;
			curr_conn = curr_conn->next;
			if (prev_conn == NULL)
			{
				connections = curr_conn;
			} else {
				prev_conn->next = curr_conn;
			}
			delete (todelete);
		}
		else
		{
			prev_conn = curr_conn;
			curr_conn = curr_conn->next;
		}
	}
}

/* the packet will be freed by the calling code */
void Connection::add (Packet * packet)
{
	lastpacket = packet->time.tv_sec;
	if (packet->Outgoing())
	{
		if (DEBUG)
		{
			std::cout << "Outgoing: " << packet->len << std::endl;
		}
		sumSent += packet->len;
		sent_packets->add (packet);
	} 
	else 
	{
		if (DEBUG)
		{
			std::cout << "Incoming: " << packet->len << std::endl;
		}
		sumRecv += packet->len;
		if (DEBUG)
		{
			std::cout << "sumRecv now: " << sumRecv << std::endl;
		}
		recv_packets->add (packet);
	}
}

/* 
 * finds connection to which this packet belongs.
 * a packet belongs to a connection if it matches
 * to its reference packet 
 */
Connection * findConnection (Packet * packet)
{
	ConnList * current = connections;
	while (current != NULL)
	{
		/* the reference packet is always *outgoing* */
		if (packet->match(current->val->refpacket))
		{
			return current->val;
		}

		current = current->next;
	}

	// Try again, now with the packet inverted:
	current = connections;
	Packet * invertedPacket = packet->newInverted();

	while (current != NULL)
	{
		/* the reference packet is always *outgoing* */
		if (invertedPacket->match(current->val->refpacket))
		{
			delete invertedPacket;
			return current->val;
		}

		current = current->next;
	}

	delete invertedPacket;
	return NULL;
}

/*
 * Connection::sumanddel
 *	
 * sums up the total bytes used
 * and removes 'old' packets. 
 *
 * Returns sum of sent packages (by address)
 *	   sum of recieved packages (by address)
 */
void Connection::sumanddel (timeval t, u_int32_t * recv, u_int32_t * sent)
{
    (*sent)=(*recv)=0;

    *sent = sent_packets->sumanddel(t);
    *recv = recv_packets->sumanddel(t);
}
