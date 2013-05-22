// #ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "vastmaxmind.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  VastMaxmind::Init(exports);
}

NODE_MODULE(index, InitAll)

// #endif