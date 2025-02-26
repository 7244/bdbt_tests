#include <WITCH/WITCH.h>
#include <WITCH/PR/PR.h>
#include <WITCH/RAND/RAND.h>

#ifndef set_UseZeroAsInvalid
  #define set_UseZeroAsInvalid 0
#endif

#define BDBT_set_prefix bdbt
#define BDBT_set_type_node uint32_t
#define BDBT_set_BitPerNode __bpn
#define BDBT_set_lcpp
#define BDBT_set_UseZeroAsInvalid set_UseZeroAsInvalid
#if __KeySize
  //#define BDBT_set_MaxKeySize 8 * 8
  #define BDBT_set_KeySize RealKeySize
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
constexpr uint32_t TestSize = 100000;

#if !__KeySize
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
    bdbt_NodeReference_t output = i + set_UseZeroAsInvalid;
    KeyType key = i;
    bdbt_KeySize_t ki;
    auto sub_root = root;
    bdbt_QueryNoPointer(&bdbt, BitOrderMatters, PassKeySize_ &key, &ki, &sub_root);
    if(ki != RealKeySize){
      KeyMap[key].NeedToBeExist = true;
      bdbt_Add(&bdbt, BitOrderMatters, PassKeySize_ &key, ki, sub_root, output);
    }
  }
}

void tree_out(){
  KeyType key = 0;
  bdbt_Traverse_t tra;

  bool First = true;
  KeyType LastKey; // value of LastKey is not used if TraverseFrom is not below checks
  if constexpr(TraverseFrom == bdbt_BitOrderLow){
    LastKey = 0;
  }
  else if(TraverseFrom == bdbt_BitOrderHigh){
    LastKey = (KeyType)-1;
  }

  #if !__KeySize
    bdbt_Traverse_InternalDataPerKeyNode_t idpkn[RealKeySize / __bpn];
  #endif

  bdbt_TraverseInit(
    &tra,
    TraverseFrom,
    #if !__KeySize
      idpkn,
    #endif
    root
  );

  while(
    bdbt_Traverse(
      &bdbt,
      &tra,
      #if !__KeySize
        idpkn,
      #endif
      BitOrderMatters,
      TraverseFrom,
      #if !__KeySize
        RealKeySize,
      #endif
      &key
    )
  ){
    if(key >= TestSize){
      __abort();
    }

    if constexpr(TraverseFrom == bdbt_BitOrderLow){
      if(key <= LastKey){
        if(First == false){
          __abort();
        }
        First = false;
      }
    }
    else if(TraverseFrom == bdbt_BitOrderHigh){
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
    bdbt_KeySize_t ki;
    auto sub_root = root;
    bdbt_QueryNoPointer(
      &bdbt,
      BitOrderMatters,
      #if !__KeySize
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

    #if !__KeySize
      bdbt_Remove_InternalDataPerKeyNode_t idpkn[RealKeySize / __bpn];
    #endif

    auto sub_root = root;

    KeyType key = r;
    bdbt_Remove(
      &bdbt,
      #if !__KeySize
        idpkn,
      #endif
      BitOrderMatters,
      #if !__KeySize
        RealKeySize,
      #endif
      &key,
      &sub_root
    );
    KeyMap[r].NeedToBeExist = false;
  }
}

int main(){
  KeyMap = (decltype(KeyMap))calloc(TestSize, sizeof(KeyMap[0]));

  root = bdbt.NewNode();

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
