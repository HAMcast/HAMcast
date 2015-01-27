/*
 * Dht.h
 *
 *  Created on: 20.06.2012
 *      Author: mario
 */

#ifndef DHT_H_
#define DHT_H_

#include "ariba/ariba.h"
#include "ariba/utility/system/SystemQueue.h"
#include "ariba/utility/logging/Logging.h"
#include "DhtAnswerInterface.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <set>

namespace ariba_service {
namespace dht {

using ariba::utility::SystemQueue;
using ariba::utility::SystemEvent;
using ariba::utility::SystemEventType;
using ariba::utility::SystemEventListener;

// Forward declarations to avoid adding messages/*.h to the public interface
class DhtMessage;

#define MEET_REPUBLISH_INTERVAL 10
#define MEET_DHT_TTL 30
#define CLEANUP_INTERVAL (5 * 60)

class Dht :
	public ariba::CommunicationListener,
	public ariba::utility::SystemEventListener,
	public boost::noncopyable
{
use_logging_h(Dht)
public:
    Dht(ariba::ServiceID serviceID, ariba::Node* node);
    virtual ~Dht();
    
    /**
     * Put the value into the DHT under the specified key
     * 
     * @param key
     *     Key to put the value under
     * @param value
     *     The value which is put
     * @param ttl
     *     The lifetime of the entry in seconds. The value will be removed
     *     automatically when it expires
     */
    void put(
    		const std::string& key,
    		const std::string& value,
    		uint16_t ttl);
    
    /**
     * Get the values specified by the key
     * 
     * @param key
     *      Key of the values which should be fetched
     */
    void get(const std::string& key);
    
    /**
     * Put and get in one single operation
     * 
     * @param key
     *     The key the value will be put under and retrieved from
     * @param value
     *     The value is first put then all values for that key, including the
     *     one just inserted will be sent back
     * @param ttl
     *     The lifetime of the entry in seconds. The value will be removed
     *     automatically when it expires
     */
    void atomic_put_and_get(
    		const std::string& key,
    		const std::string& value,
    		uint16_t ttl);
    
    /**
     * Periodically put and get the value
     * 
     * @param key
     *     The key the value will be put under and retrieved from
     * @param value
     *     The value that will be periodically put into the DHT. The value is
     *     first put then all values for that key, including the one just
     *     inserted will be sent back
     * @param ttl
     *     How long should we try to put the value periodically (measured in
     *     seconds). 0 means putting the value until stop_meet() is called
     */
    void meet(
    		const std::string& key,
    		const std::string& value,
    		uint16_t ttl);
    
    /**
     * Stop periodically pushing the value
     */
    void stop_meet(const std::string& key, const std::string& value);
    
    /**
     * Remove the value under the specified key
     */
    void remove(const std::string& key, const std::string& value);
    
    /**
     * Register a listener which is called when an answer is received
     */
    bool add_listener(DhtAnswerInterface* new_listener);

    /**
     * Unregister a listener
     * 
     * @returns true if the handler was successfully unregistered, false if the
     *     listener was not registered
     */
    bool remove_listener(DhtAnswerInterface* new_listener);
    
protected:
    /*** CommunicationListener interface ***/
    
    /**
     * Called when a message is incoming
     * @param msg The data message that is received
     * @param remote The remote node that sent the message
     * @param lnk The link id of the link where the message is received
     */
    virtual void onMessage(const ariba::DataMessage& msg, const ariba::NodeID& source,
            const ariba::LinkID& lnk = ariba::LinkID::UNSPECIFIED);
    
    
    /*** SystemEventListener interface ***/
    virtual void handleSystemEvent( const SystemEvent& event );


private:
    class ValueEntry {
	public:
		ValueEntry(const std::string& value, uint16_t ttl = 0);
		
		void refresh();
		
		const std::string& get_value() const;
		
		uint16_t get_age() const;
		
		uint16_t get_ttl() const;
		void set_ttl(uint16_t ttl);
		bool is_ttl_elapsed() const;
		uint16_t get_remaining_ttl() const;
		
		bool operator<(const ValueEntry& rhs) const;
		
	private:
		uint16_t ttl;
		boost::posix_time::ptime last_update;
		std::string value;
	};
    
    struct Key_Value
    {
        string key;
        string value;
    };
    

private:
    void handle_dht_message(const DhtMessage& message, const NodeID& source);
    
    void answer_dht_request(const std::string& key, const NodeID& source);
    void send_meet_message(const std::string& key, const std::string& value);
    void meet_update_event(const std::string& key, const std::string& value);
    
    // just for debug purpose
    void print_dht();
    
    
    ariba::ServiceID serviceID;
    ariba::Node* node;
    
    typedef std::map< std::string, std::vector<ValueEntry> > DhtTableType;
    DhtTableType table;
    DhtTableType meet_store;
    
    void insert_into_table(DhtTableType& table,
    		const std::string& key,
    		const vector<std::string>& values,
    		uint16_t ttl);
    void remove_from_table(DhtTableType& table,
    		const std::string& key,
    		const vector<std::string>& values);
    void cleanup_table(DhtTableType& table);
    void cleanup_entries(DhtTableType::mapped_type& entries);
    
    void schedule_cleanup_event(bool reschedule = false);
    bool cleanup_running;
    
    // AnswerListener
    DhtAnswerInterface* listener;
};


}} /* namespace ariba_service::dht */


#endif /* DHT_H_ */
