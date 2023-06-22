#ifndef RLP_DATA_HPP_fork_5
#define RLP_DATA_HPP_fork_5

#include <string>
#include <vector>

using namespace std;

namespace fork_5
{

enum RLPType
{
    rlpTypeUnknown = 0,
    rlpTypeString = 1,
    rlpTypeList = 2
};

// RLP data can be a string (data) or a list of other RLP data elements (rlpData)
class RLPData
{
public:
    RLPType type;
    string data;
    vector<RLPData> rlpData;
};

}

#endif