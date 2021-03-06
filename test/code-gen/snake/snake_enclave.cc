#include "interface_selectors.h"
#include "asylo/platform/primitives/extent.h"
#include "asylo/platform/primitives/primitive_status.h"
#include "asylo/platform/primitives/trusted_primitives.h"
#include "asylo/util/status_macro.h"

extern void show_score(screen_t* screen);
extern void setup_level(screen_t* screen, snake_t* snake, int level);
extern void move(snake_t* snake, char[] keys, char key);
extern int collide_object(snake_t* snake, screen_t* screen, char object);
extern int collision(snake_t* snake, screen_t* screen);
extern int eat_gold(snake_t* snake, screen_t* screen);

int printf(char* ) { 
   int returnVal;
   asylo::primitives::printf(&*, &returnVal);
   return returnVal;
}

int printf(char* __format) { 
   int returnVal;
   asylo::primitives::printf(&*__format, &returnVal);
   return returnVal;
}

int puts(char* __s) { 
   int returnVal;
   asylo::primitives::puts(&*__s, &returnVal);
   return returnVal;
}

int rand() { 
   int returnVal;
   asylo::primitives::rand(&returnVal);
   return returnVal;
}

void srand(unsigned int __seed) { 
   asylo::primitives::srand(__seed);
   return returnVal;
}

long time(long* __timer) { 
   long returnVal;
   asylo::primitives::time(&*__timer, &returnVal);
   return returnVal;
}

void draw_line(int col, int row) { 
   asylo::primitives::draw_line(col, row);
   return returnVal;
}

namespace asylo {

namespace primitives {

extern "C" PrimitiveStatus asylo_enclave_init() { 
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(keat_goldEnclaveSelector, EntryHandler{secure_eat_gold}));
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(kcollisionEnclaveSelector, EntryHandler{secure_collision}));
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(kcollide_objectEnclaveSelector, EntryHandler{secure_collide_object}));
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(kshow_scoreEnclaveSelector, EntryHandler{secure_show_score}));
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(kmoveEnclaveSelector, EntryHandler{secure_move}));
    ASYLO_RETURN_IF_ERROR(TrustedPrimitives::RegisterEntryHandler(ksetup_levelEnclaveSelector, EntryHandler{secure_setup_level}));
   return PrimitiveStatus::OkStatus();
}

extern "C" PrimitiveStatus asylo_enclave_fini() { 
   return PrimitiveStatus::OkStatus();
}

namespace  {

PrimitiveStatus Abort(void* context, TrustedParameterStack* params) { 
   TrustedPrimitives::BestEffortAbort("Aborting enclave");
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_show_score(void* context, TrustedParameterStack* params) { 
   screen_t screen_param = params->Pop<screen_t>();
   show_score(&screen_param);
   *params->PushAlloc<screen_t>() = screen_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_setup_level(void* context, TrustedParameterStack* params) { 
   int level_param = params->Pop<int>();
   snake_t snake_param = params->Pop<snake_t>();
   screen_t screen_param = params->Pop<screen_t>();
   setup_level(&screen_param, &snake_param, level_param);
   *params->PushAlloc<screen_t>() = screen_param;
   *params->PushAlloc<snake_t>() = snake_param;
   *params->PushAlloc<int>() = level_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_move(void* context, TrustedParameterStack* params) { 
   char key_param = params->Pop<char>();
   char* keys_param = reinterpret_cast<char*>(params->Pop()->data());
   snake_t snake_param = params->Pop<snake_t>();
   move(&snake_param, keys_param, key_param);
   *params->PushAlloc<snake_t>() = snake_param;
   auto keys_param_ext = params->PushAlloc(sizeof(char) *strlen(keys_param));
memcpy(keys_param_ext.As<char>(), keys_param, strlen(keys_param));
   *params->PushAlloc<char>() = key_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_collide_object(void* context, TrustedParameterStack* params) { 
   char object_param = params->Pop<char>();
   screen_t screen_param = params->Pop<screen_t>();
   snake_t snake_param = params->Pop<snake_t>();
   int returnVal = collide_object(&snake_param, &screen_param, object_param);
   *params->PushAlloc<int>() = returnVal;
   *params->PushAlloc<snake_t>() = snake_param;
   *params->PushAlloc<screen_t>() = screen_param;
   *params->PushAlloc<char>() = object_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_collision(void* context, TrustedParameterStack* params) { 
   screen_t screen_param = params->Pop<screen_t>();
   snake_t snake_param = params->Pop<snake_t>();
   int returnVal = collision(&snake_param, &screen_param);
   *params->PushAlloc<int>() = returnVal;
   *params->PushAlloc<snake_t>() = snake_param;
   *params->PushAlloc<screen_t>() = screen_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus secure_eat_gold(void* context, TrustedParameterStack* params) { 
   screen_t screen_param = params->Pop<screen_t>();
   snake_t snake_param = params->Pop<snake_t>();
   int returnVal = eat_gold(&snake_param, &screen_param);
   *params->PushAlloc<int>() = returnVal;
   *params->PushAlloc<snake_t>() = snake_param;
   *params->PushAlloc<screen_t>() = screen_param;
   PrimitiveStatus::OkStatus();
}

PrimitiveStatus printf(char* , int* returnVal) { 
   TrustedParameterStack params;
   *params.PushAlloc<char>() = *;
   *params.PushAlloc<int>() = *returnVal;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(kprintfOCallHandler, &params));
    = params.Pop<char>();
   returnVal = params.Pop<int>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus printf(char* __format, int* returnVal) { 
   TrustedParameterStack params;
   *params.PushAlloc<char>() = *__format;
   *params.PushAlloc<int>() = *returnVal;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(kprintfOCallHandler, &params));
   __format = params.Pop<char>();
   returnVal = params.Pop<int>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus puts(char* __s, int* returnVal) { 
   TrustedParameterStack params;
   *params.PushAlloc<char>() = *__s;
   *params.PushAlloc<int>() = *returnVal;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(kputsOCallHandler, &params));
   __s = params.Pop<char>();
   returnVal = params.Pop<int>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus rand(int* returnVal) { 
   TrustedParameterStack params;
   *params.PushAlloc<int>() = *returnVal;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(krandOCallHandler, &params));
   returnVal = params.Pop<int>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus srand(unsigned int __seed) { 
   TrustedParameterStack params;
   *params.PushAlloc<unsigned int>() = __seed;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(ksrandOCallHandler, &params));
   __seed = params.Pop<unsigned int>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus time(long* __timer, long* returnVal) { 
   TrustedParameterStack params;
   *params.PushAlloc<long>() = *__timer;
   *params.PushAlloc<long>() = *returnVal;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(ktimeOCallHandler, &params));
   __timer = params.Pop<long>();
   returnVal = params.Pop<long>();
   return PrimitiveStatus::OkStatus();
}

PrimitiveStatus draw_line(int col, int row) { 
   TrustedParameterStack params;
   *params.PushAlloc<int>() = col;
   *params.PushAlloc<int>() = row;
   ASYLO_RETURN_IF_ERROR(TrustedPrimitives::UntrustedCall(kdraw_lineOCallHandler, &params));
   col = params.Pop<int>();
   row = params.Pop<int>();
   return PrimitiveStatus::OkStatus();
}

} // namespace 
} // namespace primitives
} // namespace asylo
