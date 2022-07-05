#ifndef STARK_INFO_HPP
#define STARK_INFO_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "config.hpp"
#include "zkassert.hpp"
#include "goldilocks/goldilocks_base_field.hpp"

using json = nlohmann::json;
using namespace std;

class StepStruct
{
public:
    uint64_t nBits;
};

class StarkStruct
{
public:
    uint64_t nBits;
    uint64_t nBitsExt;
    uint64_t nQueries;
    string verificationHashType;
    vector<StepStruct> steps;
};

class PolsSections
{
public:
    uint64_t cm1_n;
    uint64_t cm2_n;
    uint64_t cm3_n;
    uint64_t exps_withq_n;
    uint64_t exps_withoutq_n;
    uint64_t cm1_2ns;
    uint64_t cm2_2ns;
    uint64_t cm3_2ns;
    uint64_t q_2ns;
    uint64_t exps_withq_2ns;
    uint64_t exps_withoutq_2ns;
    uint64_t getSection(string section) // TODO: Change string by int/enum for performance reasons
    {
        if (section=="cm1_n") return cm1_n;
        if (section=="cm2_n") return cm2_n;
        if (section=="cm3_n") return cm3_n;
        if (section=="exps_withq_n") return exps_withq_n;
        if (section=="exps_withoutq_n") return exps_withoutq_n;
        if (section=="cm1_2ns") return cm1_2ns;
        if (section=="cm2_2ns") return cm2_2ns;
        if (section=="cm3_2ns") return cm3_2ns;
        if (section=="q_2ns") return q_2ns;
        if (section=="exps_withq_2ns") return exps_withq_2ns;
        if (section=="exps_withoutq_2ns") return exps_withoutq_2ns;
        cerr << "Error: PolsSections::getSection() called with invalid section=" << section << endl;
        exit(-1);
    }
};

class PolsSectionsVector
{
public:
    vector<uint64_t> cm1_n;
    vector<uint64_t> cm2_n;
    vector<uint64_t> cm3_n;
    vector<uint64_t> exps_withq_n;
    vector<uint64_t> exps_withoutq_n;
    vector<uint64_t> cm1_2ns;
    vector<uint64_t> cm2_2ns;
    vector<uint64_t> cm3_2ns;
    vector<uint64_t> q_2ns;
    vector<uint64_t> exps_withq_2ns;
    vector<uint64_t> exps_withoutq_2ns;
};

class VarPolMap
{
public:
    string section;
    uint64_t dim;
    uint64_t sectionPos;
};

class PolInfo
{
public:
    VarPolMap map;
    uint64_t N;
    uint64_t offset;
    uint64_t size;
    Goldilocks::Element * pAddress;
    Goldilocks::Element * get(uint64_t step)
    {
        zkassert(map.dim==1);
        return pAddress + step*size;
    }
    Goldilocks::Element * get1(uint64_t step)
    {
        zkassert(map.dim==3);
        return pAddress + step*size;
    }
    Goldilocks::Element * get2(uint64_t step)
    {
        zkassert(map.dim==3);
        return pAddress + step*size + 1;
    }
    Goldilocks::Element * get3(uint64_t step)
    {
        zkassert(map.dim==3);
        return pAddress + step*size + 2;
    }
};

class PeCtx
{
public:
    uint64_t tExpId;
    uint64_t fExpId;
    uint64_t zId;
    uint64_t c1Id;
    uint64_t numId;
    uint64_t denId;
    uint64_t c2Id;
};

class PuCtx
{
public:
    uint64_t tExpId;
    uint64_t fExpId;
    uint64_t h1Id;
    uint64_t h2Id;
    uint64_t zId;
    uint64_t c1Id;
    uint64_t numId;
    uint64_t denId;
    uint64_t c2Id;
};
class CiCtx
{
public:
    uint64_t zId;
    uint64_t numId;
    uint64_t denId;
    uint64_t c1Id;
    uint64_t c2Id;
};

class EvMap
{
public:
    string type; // TODO: Replace by an enum for performance reasons; possible values: "const", "cm"
    uint64_t id;
    bool prime;
};

class StepType
{
public:
    string type; // TODO: Replace by an enum for performance reasons
    uint64_t id;
    bool prime;
    uint64_t p;
    string value;
};

class StepOperation
{
public:
    typedef enum
    {
        add = 0,
        sub = 1,
        mul = 2,
        copy = 3
    } eOperation;

    eOperation op; // TODO: Replace by an enum for performance reasons; possible values: "sub", "add", ...
    StepType dest;
    vector<StepType> src;
};

class Step
{
public:
    vector<StepOperation> first;
    vector<StepOperation> i;
    vector<StepOperation> last;
    uint64_t tmpUsed;
};

class Expression
{
public:
    bool isNull;
    uint64_t value;
};

class StarkInfo
{
    const Config &config;
public:
    StarkStruct starkStruct;
    uint64_t mapTotalN;
    uint64_t nConstants;
    uint64_t nCm1;
    uint64_t nCm2;
    uint64_t nCm3;
    uint64_t nCm4;
    uint64_t nQ;
    uint64_t friExpId;
    uint64_t nExps;

    PolsSections mapDeg;
    PolsSections mapOffsets;
    PolsSectionsVector mapSections;
    PolsSections mapSectionsN;
    PolsSections mapSectionsN1;
    PolsSections mapSectionsN3;

    vector<VarPolMap> varPolMap;

    vector<uint64_t> qs;

    vector<uint64_t> cm_n;
    
    vector<PeCtx> peCtx;
    
    vector<PuCtx> puCtx;
    
    vector<CiCtx> ciCtx;

    vector<EvMap> evMap;

    Step step2prev;
    Step step3prev;
    Step step4;
    Step step42ns;
    Step step52ns;

    vector<Expression> exps_n;

    StarkInfo(const Config &config);
    void load (json j);
    void getPol(void * pAddress, uint64_t idPol, PolInfo &polInfo);
};

#endif