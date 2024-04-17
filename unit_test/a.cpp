#include <WITCH/WITCH.h>
#include <WITCH/PR/PR.h>
#include <WITCH/RAND/RAND.h>

#define BDBT_set_prefix bdbt
#define BDBT_set_type_node uint32_t
#define BDBT_set_BitPerNode __bpn
#define BDBT_set_declare_rest 1
#define BDBT_set_declare_Key 1
#define BDBT_set_lcpp
#ifdef __KeySize
  #define BDBT_set_KeySize __KeySize
#endif
#define BDBT_set_CPP_ConstructDestruct
#include <BDBT/BDBT.h>

bdbt_t bdbt;
bdbt_NodeReference_t root;

constexpr bool BitOrderMatters = __BitOrderMatters;
#undef __BitOrderMatters
constexpr uint8_t TraverseFrom = __TraverseFrom;
#undef __TraverseFrom

using KeyType = uint64_t;
constexpr uint32_t TestSize = 500000;

bdbt_Key_t<
  #ifndef __KeySize
    RealKeySize,
  #endif
  BitOrderMatters
>k;

#ifdef __KeySize
  #define PassKeySize_ RealKeySize,
#else
  #define PassKeySize_
#endif

struct{
  uint8_t NeedToBeExist : 1, QueryFound : 1, TraverseFound : 1;
}*KeyMap;

void tree_in(){
  for(uint32_t i = 0; i < TestSize; i++){
    if(KeyMap[i].NeedToBeExist == true){
      continue;
    }
    bdbt_NodeReference_t output = i;
    KeyType key = i;
    typename decltype(k)::KeySize_t ki;
    auto sub_root = root;
    k.q(&bdbt, PassKeySize_ &key, &ki, &sub_root);
    if(ki != RealKeySize){
      KeyMap[key].NeedToBeExist = true;
      k.a(&bdbt, PassKeySize_ &key, ki, sub_root, output);
    }
  }
}

void tree_out(){
  KeyType key = 0;
  typename decltype(k)::Traverse_t tra;

  bool First = true;
  KeyType LastKey; // value of LastKey is not used if TraverseFrom is not below checks
  if constexpr(TraverseFrom == 0){
    LastKey = 0;
  }
  else if(TraverseFrom == 1){
    LastKey = (KeyType)-1;
  }

  #ifdef __KeySize
    decltype(tra)::ta_t ta[tra.GetTraverseArraySize(RealKeySize)];
    tra.i<TraverseFrom>(ta, root);
  #else
    tra.i<TraverseFrom>(root);
  #endif

  while(
    tra.t<TraverseFrom>(
      &bdbt,
      #ifdef __KeySize
        ta,
        RealKeySize,
      #endif
      &key
    )
  ){
    if(key >= TestSize){
      __abort();
    }

    if constexpr(TraverseFrom == 0){
      if(key <= LastKey){
        if(First == false){
          __abort();
        }
        First = false;
      }
    }
    else if(TraverseFrom == 1){
      if(key >= LastKey){
        if(First == false){
          __abort();
        }
        First = false;
      }
    }
    if(KeyMap[key].NeedToBeExist != true){
      __abort();
    }
    KeyMap[key].TraverseFound = true;
    LastKey = key;
  }
}

void QueryAll(){
  for(uint32_t i = 0; i < TestSize; i++){
    if(KeyMap[i].NeedToBeExist == false){
      continue;
    }
    KeyType key = i;
    typename decltype(k)::KeySize_t ki;
    auto sub_root = root;
    k.q(
      &bdbt,
      #ifdef __KeySize
        RealKeySize,
      #endif
      &key,
      &ki,
      &sub_root
    );
    if(KeyMap[i].NeedToBeExist != (ki == RealKeySize)){
      __abort();
    }
  }
}

void ResetInfo(){
  for(uint32_t i = 0; i < TestSize; i++){
    KeyMap[i].QueryFound = false;
    KeyMap[i].TraverseFound = false;
  }
}

void CheckAll(){
  tree_out();
  QueryAll();

  for(uint32_t i = 0; i < TestSize; i++){
    if(KeyMap[i].NeedToBeExist == true && KeyMap[i].TraverseFound == false){
      __abort();
    }
  }

  ResetInfo();
}

void DeleteRandom(){
  for(uint32_t i = 0; i < TestSize / 10; i++){
    uint32_t r = RAND_hard_32() % TestSize;
    if(KeyMap[r].NeedToBeExist != true){
      continue;
    }
    KeyType key = r;
    k.r(
      &bdbt,
      #ifdef __KeySize
        RealKeySize,
      #endif
      &key,
      root
    );
    KeyMap[r].NeedToBeExist = false;
  }
}

int main(){
  KeyMap = (decltype(KeyMap))calloc(TestSize, sizeof(KeyMap[0]));

  root = bdbt_NewNode(&bdbt);

  tree_in();

  CheckAll();
  auto Current0 = bdbt.NodeList.Current;
  DeleteRandom();
  CheckAll();

  tree_in();
  CheckAll();
  auto Current1 = bdbt.NodeList.Current;
  if(Current0 != Current1){
    __abort();
  }

  free(KeyMap);

  return 0;
}
