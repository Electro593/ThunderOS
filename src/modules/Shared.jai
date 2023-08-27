//
// This module contains some shared utilities for the
// kernel and loader.
//

PAGE_SIZE :: 0x1000;

align_up :: inline (value: $I, $alignment: int) -> I #modify {
   info: *Type_Info_Integer = xx I;
   return Bit_Operations.popcount(alignment) == 1
      && info.type == .INTEGER
      && !info.signed;
} #expand {
   return (value + alignment - 1) & ~(alignment - 1);
}

align_down :: inline (value: $I, $alignment: int) -> I #modify {
   info: *Type_Info_Integer = xx I;
   return Bit_Operations.popcount(alignment) == 1
      && info.type == .INTEGER
      && !info.signed;
} #expand {
   return value & ~(alignment - 1);
}

#program_export
memset :: (data: *u8, value: u8, count: s32) #c_call #no_context {
   for 0..count-1 data[it] = value;
}

#program_export
memcpy :: (destination: *u8, source: *u8, count: s32) #c_call #no_context {
   for 0..count-1 destination[it] = source[it];
}

#program_export
memcmp :: (a: *u8, b: *u8, count: s32) -> s32 #c_call #no_context {
   i := 0;
   while i < count && a[i] == b[i] i += 1;
   return
      xx ifx i == count then 0
      else ifx a[i] < b[i] then -1
      else 1;
}

// We're not linking to the standard library, so we have to replace
// the intrinsics. We're exporting because it prevents name mangling.
// #program_export

#program_export sigemptyset               :: () #no_call { }
#program_export write                     :: () #no_call { }
#program_export pthread_mutex_unlock      :: () #no_call { }
#program_export pthread_mutex_lock        :: () #no_call { }
#program_export pthread_mutexattr_init    :: () #no_call { }
#program_export pthread_mutexattr_settype :: () #no_call { }
#program_export pthread_mutex_init        :: () #no_call { }
#program_export _exit                     :: () #no_call { }
#program_export __gmon_start__            :: () #no_call { }
#program_export sigaction                 :: () #no_call { }
#program_export backtrace                 :: () #no_call { }
#program_export backtrace_symbols         :: () #no_call { }


#scope_file

Bit_Operations :: #import "Bit_Operations";