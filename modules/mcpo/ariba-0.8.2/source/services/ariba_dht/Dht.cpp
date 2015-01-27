/*
 * Dht.cpp
 *
 *  Created on: 20.06.2012
 *      Author: mario
 */

#include "Dht.h"
#include "messages/DhtMessage.h"
#include <boost/date_time/time_clock.hpp>

namespace ariba_service {
namespace dht {

use_logging_cpp(Dht)

using namespace std;
using boost::date_time::second_clock;
using boost::posix_time::ptime;

SystemEventType DhtRepublishEvent("DhtRepublishEvent");
SystemEventType DhtCleanupEvent("DhtCleanupEvent");


Dht::Dht(ariba::ServiceID serviceID, ariba::Node* node)  :
        serviceID(serviceID),
        node(node),
        cleanup_running(false),
        listener(NULL)
{
    this->node->bind(this, serviceID);
}

Dht::~Dht()
{
    this->node->unbind(this, serviceID);
}



void Dht::put(const std::string& key, const std::string& value, uint16_t ttl)
{
    DhtMessage msg(DhtMessage::DhtPut, key, value, ttl);

    handle_dht_message(msg, NodeID::UNSPECIFIED);
}

void Dht::get(const std::string& key)
{
	DhtMessage msg(DhtMessage::DhtGet, key);
	
	handle_dht_message(msg, NodeID::UNSPECIFIED);
}

void Dht::atomic_put_and_get(const std::string& key, const std::string& value, uint16_t ttl)
{    
    DhtMessage msg(DhtMessage::DhtPutAndGet, key, value, ttl);

    handle_dht_message(msg, NodeID::UNSPECIFIED);
}

void Dht::meet(const std::string& key, const std::string& value, uint16_t ttl_in_sec)
{
    // insert into meet_store
    insert_into_table(meet_store,
    		key,
    		std::vector<std::string>(1, value),
    		ttl_in_sec);
	
    // send message (and program republishing)
    send_meet_message(key, value);
}

void Dht::stop_meet(const std::string& key, const std::string& value)
{
    remove_from_table(meet_store,
    		key,
    		std::vector<std::string>(1, value));
}

void Dht::remove(const std::string& key, const std::string& value)
{
    // send delete message
    DhtMessage msg(DhtMessage::DhtRemove, key, value);

    handle_dht_message(msg, NodeID::UNSPECIFIED);
}




bool Dht::add_listener(DhtAnswerInterface* new_listener)
{
    if ( listener == NULL )
    {
        listener = new_listener;
        
        return true;
    }
    else
    {
        return false;
    }
}

bool Dht::remove_listener(DhtAnswerInterface* new_listener)
{
	if (listener == new_listener) {
		listener = NULL;
		return true;
		
	} else {
		return false;
	}
}




//** PRIVATE FUNCTIONS **//

void Dht::handle_dht_message(const DhtMessage& message, const NodeID& source)
{
    // send message closer to hashed key
    NodeID addr = message.getHashedKey();

    logging_debug("Processing DHT message...");
    
    logging_debug("Dest Addr: " << addr.toString());

    // * send closer, if possible *
    const ariba::NodeID dest = node->sendMessageCloserToNodeID(message, addr, this->serviceID);
    
    logging_debug("Closer Node: " << dest.toString());
    
    // couldn't send closer, so we are the closest node
    //   ---> * handle dht request * (store value, etc.)
    if ( dest == NodeID::UNSPECIFIED )
    {
    	logging_debug("DHT: We are the closest node!");
        
        switch (message.getType())
        {
            case DhtMessage::DhtPut:
            {
                insert_into_table(
                		table,
                		message.getKey(),
                		message.getValues(),
                		message.getTTL());
                
                break;
            }
            
            case DhtMessage::DhtGet:
            {
                answer_dht_request(message.getKey(), source);

                break;
            }
            
            case DhtMessage::DhtPutAndGet:
            {
                insert_into_table(
                		table,
                		message.getKey(),
                		message.getValues(),
                		message.getTTL());
                answer_dht_request(message.getKey(), source);
                
                break;
            }
            
            case DhtMessage::DhtRemove:
            {
                remove_from_table(table, message.getKey(), message.getValues());
                
                break;
            }
        }
    }
}


void Dht::insert_into_table(DhtTableType& table,
		const std::string& key,
		const vector<std::string>& values,
		uint16_t ttl)
{
	DhtTableType::mapped_type& value_entries = table[key];
	
	BOOST_FOREACH(const std::string& value, values) {
		
		// Debug output
		logging_info("DHT: Inserting (" << key << ", " << value << ")");
		
		// push the value for the given key (into the vector)
		bool entry_updated = false;
		for (
				DhtTableType::mapped_type::iterator position = value_entries.begin();
				position != value_entries.end();
				++position)
		{
			if (position->get_value() == value) {
				position->set_ttl(ttl);
				entry_updated = true;
				break;
			}
		}
		
		if (!entry_updated) {
			value_entries.push_back(ValueEntry(value, ttl));
		}
	}
	
	schedule_cleanup_event();
}


void Dht::remove_from_table(DhtTableType& table,
		const std::string& key,
		const vector<std::string>& values)
{
	logging_debug("DHT: trying to delete some values for key " << key);
	// find key
	DhtTableType::iterator key_position = table.find(key);
	if (key_position == table.end()) {
		return;
	}
	
	// delete values from set of values
	DhtTableType::mapped_type& entries = key_position->second;
	BOOST_FOREACH(const std::string& value, values) {
		for (
				DhtTableType::mapped_type::iterator entry = entries.begin();
				entry != entries.end();
				++entry)
		{
			if (entry->get_value() == value) {
				logging_info("DHT: Deleting "
						"(" <<key << ", " << entry->get_value() << ")");
				entries.erase(entry);
				break;
			}
		}
	}
	
    // the key could empty now
    //   ---> remove it
    if ( entries.size() == 0 )
    {
        table.erase(key_position);
    }
}


void Dht::cleanup_table(DhtTableType& table)
{
	logging_debug("DHT: cleaning up table");
	
	vector<std::string> to_be_deleted;
	
	for (
			DhtTableType::iterator position = table.begin();
			position != table.end();
			++position)
	{
		cleanup_entries(position->second);
		
		// mark entry container for removal if empty
		if (position->second.size() == 0) {
			to_be_deleted.push_back(position->first);
		}
	}
	
	BOOST_FOREACH(const std::string& key, to_be_deleted) {
		table.erase(key);
	}
}

void Dht::cleanup_entries(DhtTableType::mapped_type& entries)
{
	DhtTableType::mapped_type::iterator position = entries.begin();
	while (position != entries.end()) {
		
		if (position->is_ttl_elapsed()) {
			// remove stale entry
			position = entries.erase(position);
			
		} else {
			// move on otherwise
			++position;
		}
	}
}


void Dht::answer_dht_request(const std::string& key, const NodeID& source)
{
    // get entries from table
    const DhtTableType::mapped_type& entries = table[key];
    
    // need to convert value entries to strings
	vector<std::string> values;
	values.reserve(entries.size());
	BOOST_FOREACH(const ValueEntry& entry, entries) {
		
		if (!entry.is_ttl_elapsed()) {
			values.push_back(entry.get_value());
		}
		
	}
    
    // BRANCH: request comes from another node
    //   ---> send answer message
    if ( source != NodeID::UNSPECIFIED )
    {
        // create answer message
        DhtMessage msg(DhtMessage::DhtAnswer, key, values);
        
        // * send answer *
        node->sendMessage(msg, source, serviceID);
    }
    
    // BRANCH: local request
    //   ---> inform listeners directly (TODO code duplicates...)
    else
    {
        logging_debug("DHT: Answering request for key '" << key << "' locally");

        // * inform listeners *
        if ( listener )
        {
            listener->handle_dht_answer(key, values);
        }
    }
    
    
    // an empty key could have been created
    //   ---> remove it
    if ( entries.size() == 0 )
    {
        table.erase(key);
    }
}


void Dht::send_meet_message(const std::string& key, const std::string& value)
{
    // send put&get message
    DhtMessage msg(DhtMessage::DhtPutAndGet, key, value, MEET_DHT_TTL);
    
    handle_dht_message(msg, NodeID::UNSPECIFIED);
    
    // program timer for republish (or deletion)
    Key_Value* kv = new Key_Value;
    kv->key = key;
    kv->value = value;
    
    SystemQueue::instance().scheduleEvent( 
            SystemEvent( this, DhtRepublishEvent, kv),
            MEET_REPUBLISH_INTERVAL * 1000 );
}


void Dht::meet_update_event(const std::string& key, const std::string& value)
{
    // get entries from table
    DhtTableType::mapped_type& entries = meet_store[key];
    
    cleanup_entries(entries);
    
    // find right entry
    BOOST_FOREACH(const ValueEntry& entry, entries) {
    	if (entry.get_value() == value) {
    		
    		// republish value
    		logging_debug("DHT: Republishing "
    				"(" << key << ", " << entry.get_value() << ")");
    		send_meet_message(key, entry.get_value());
    	}
    }
    
    // an empty key could have been created
    //   ---> remove it
    if ( entries.size() == 0 )
    {
        meet_store.erase(key);
    }
}

void Dht::schedule_cleanup_event(bool reschedule)
{
	if (reschedule || !cleanup_running) {
		SystemQueue::instance().scheduleEvent(
				SystemEvent(this, DhtCleanupEvent),
				CLEANUP_INTERVAL * 1000);
		cleanup_running = true;
	}
}


void Dht::print_dht()
{
	logging_debug("======== DHT ========");
    for ( DhtTableType::iterator dht_it = table.begin(); dht_it != table.end(); dht_it++)
    {
    	logging_debug("Key: " << dht_it->first);
        
        for ( DhtTableType::mapped_type::iterator value_it = dht_it->second.begin();
                value_it != dht_it->second.end();
                value_it++ )
        {
        	logging_debug("--> " << value_it->get_value());
        }
        
        logging_debug("- - - - -");
    }
    
    logging_debug("======== [DHT] ========");
}



void Dht::onMessage(const ariba::DataMessage& msg, const ariba::NodeID& source,
        const ariba::LinkID& lnk)
{
	logging_debug("DHT: Incoming message...");
    
    DhtMessage* mess = msg.getMessage()->convert<DhtMessage> ();
    
    // handle message
    switch (mess->getType())
    {
        // BRANCH: Message is an Answer for our request
        case DhtMessage::DhtAnswer:
        {
        	logging_debug("DHT: Got answer for key '" << mess->getKey() << "'");
            
            BOOST_FOREACH(string str, mess->getValues())
            {
            	logging_debug("--> Value: '" << str << "'");
            }
            
            // * inform listeners *
            if ( listener )
            {
                listener->handle_dht_answer(mess->getKey(), mess->getValues());
            }

            break;
        }
        
        // BRANCH: Message is a Request
        //   ---> route or handle
        default:
        {
            handle_dht_message(*mess, source);
            
            break;
        }
    }

    delete mess;
}


void Dht::handleSystemEvent( const SystemEvent& event )
{
	
	if (event.getType() == DhtRepublishEvent) {
		logging_debug("DHT: Meet republish event!");
		
		// republish meet entry
		Key_Value* kv = event.getData<Key_Value>();
		meet_update_event(kv->key, kv->value);
		delete kv;
		
	} else if (event.getType() == DhtCleanupEvent) {
		logging_debug("DHT: Cleanup event!");
		
		cleanup_table(table);
		schedule_cleanup_event(true);
	}
}


/**************
 * ValueEntry *
 **************/

Dht::ValueEntry::ValueEntry(
		const std::string& value,
		uint16_t ttl) :
	ttl(ttl),
	last_update(second_clock<ptime>::universal_time()),
	value(value)
{
}


void Dht::ValueEntry::refresh() {
	last_update = second_clock<ptime>::universal_time();
}


const std::string& Dht::ValueEntry::get_value() const {
	return value;
}

uint16_t Dht::ValueEntry::get_age() const
{
    boost::posix_time::time_duration diff = 
            second_clock<ptime>::universal_time() - last_update;
    
    return diff.total_seconds();
}

uint16_t Dht::ValueEntry::get_ttl() const {
	return ttl;
}

void Dht::ValueEntry::set_ttl(uint16_t ttl) {
	this->refresh();
	this->ttl = ttl;
}

bool Dht::ValueEntry::is_ttl_elapsed() const {
	// ttl == 0 signals infinite lifetime
	if (ttl == 0) {
		return false;
	}
	
	return second_clock<ptime>::universal_time() >= 
			(last_update + boost::posix_time::seconds(ttl));
}

uint16_t Dht::ValueEntry::get_remaining_ttl() const
{
    if ( ttl == 0 )
        return -1;
    
    if ( is_ttl_elapsed() )
        return 0;
    
    boost::posix_time::time_duration diff = 
            (last_update + boost::posix_time::seconds(ttl)) -
            second_clock<ptime>::universal_time();
    
    return ttl - get_age();
}

}} /* namespace ariba_service::dht */
