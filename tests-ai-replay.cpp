#include "src/ai/AiReplayJson.hpp"
#include <cassert>
#include <iostream>
using namespace ai_macro;
int main() {
    std::vector<InputEvent> events{{18,1,true},{20,1,false},{54,1,true},{56,1,false}};
    auto json = makeGdrJson({12345,"A \"test\" <level>",100,240},events);
    assert(json && json->find("\"name\":\"A \\\"test\\\" <level>\"") != std::string::npos);
    assert(json->find("\"frame\":54,\"btn\":1,\"2p\":false,\"down\":true") != std::string::npos);
    assert(json->find("\"frameFixes\":[]") != std::string::npos);
    assert(!makeGdrJson({12345,"test",19,240},events)); // future events
    auto broken = events; broken.pop_back();
    assert(!makeGdrJson({12345,"test",100,240},broken));
    broken = events; broken[1].down = true;
    assert(!makeGdrJson({12345,"test",100,240},broken));
    assert(!makeGdrJson({12345,"test",100,0},events));
    std::cout << *json;
}
