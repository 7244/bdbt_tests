#include <WITCH/WITCH.h>
#include <WITCH/PR/PR.h>
#include <WITCH/RAND/RAND.h>

#define BDBT_set_prefix bdbt
#define BDBT_set_type_node uint32_t
#define BDBT_set_BitPerNode __bpn
#define BDBT_set_declare_rest 1
#define BDBT_set_KeySize 0
#define BDBT_set_declare_Key 1
#define BDBT_set_Language 1
#define BDBT_set_BaseLibrary 0
#define BDBT_set_CPP_ConstructDestruct
#include <WITCH/BDBT/BDBT.h>
#undef __bpn

bdbt_t bdbt;
bdbt_NodeReference_t root;

constexpr bool BitOrderMatters = __BitOrderMatters;
#undef __BitOrderMatters
constexpr uint8_t TraverseFrom = __TraverseFrom;
#undef __TraverseFrom

using KeyType = uint32_t;
constexpr uint32_t TestSize = 1000000;

static_assert(TestSize <= ((uint64_t )1 << sizeof(KeyType) * 8));

bdbt_Key_t<sizeof(KeyType) * 8, BitOrderMatters> k;

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
    k.q(&bdbt, &key, &ki, &sub_root);
    if(ki != sizeof(KeyType) * 8){
      KeyMap[key].NeedToBeExist = true;
      k.a(&bdbt, &key, ki, sub_root, output);
    }
  }
}

void tree_out(){
  KeyType key;
  typename decltype(k)::Traverse_t tra;

  bool First = true;
  KeyType LastKey;
  if constexpr(TraverseFrom == 0){
    LastKey = 0;
  }
  else if(TraverseFrom == 1){
    LastKey = (KeyType)-1;
  }

  tra.i<TraverseFrom>(root);
  while(tra.t<TraverseFrom>(&bdbt, &key)){
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
    k.q(&bdbt, &key, &ki, &sub_root);
    if(KeyMap[i].NeedToBeExist != (ki == sizeof(KeyType) * 8)){
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
    k.r(&bdbt, &key, root);
    KeyMap[r].NeedToBeExist = false;
  }
}

int main(){
  KeyMap = (decltype(KeyMap))calloc(TestSize, sizeof(KeyMap[0]));

  bdbt_Open(&bdbt);

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

  return 0;
}
