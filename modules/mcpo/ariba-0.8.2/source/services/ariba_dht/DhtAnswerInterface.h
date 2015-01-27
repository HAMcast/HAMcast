#ifndef DHT_ANSWER_INTERFACE_H_
#define DHT_ANSWER_INTERFACE_H_

namespace ariba_service {
namespace dht {

class DhtAnswerInterface
{
public:
    virtual void handle_dht_answer(const std::string& key, const std::vector<std::string>& values) = 0;
    
    virtual ~DhtAnswerInterface() {}
};

}} /* namespace ariba_service::dht */

#endif /* DHT_ANSWER_INTERFACE_H_ */
