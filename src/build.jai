#scope_file

#import "Basic";
#import "Compiler";
#import "Process";
#import "File";
#import "File_Utilities";
#import "System";
#import "String";

#run {
   set_build_options_dc(.{do_output=false, write_added_strings=false});
   
   w := compiler_create_workspace("jai_lib");
   options := get_build_options(w);
   
   options.use_visual_studio_message_format = true;
   options.output_type = .STATIC_LIBRARY;
   options.output_path = "../build";
   options.output_executable_name = "jai_lib";
   options.intermediate_path = "../build/tmp";
   options.dead_code_elimination = .ALL;
   
   set_build_options(options, w);
   
   add_build_file("kernel/kernel.jai", w);
}