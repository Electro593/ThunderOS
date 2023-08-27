#program_export
_start :: () #c_call #no_context {
   ctx: Context;
   push_context ctx {
      kernel_entry();
   }
}

kernel_entry :: () {
   while true { }
}


#scope_file

#import "Shared";
#import "UEFI";