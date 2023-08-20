#import "Basic";
#import "Compiler";
#import "Process";

#if OS == .LINUX {
   #import "POSIX";
}

complete := 0;

build_kernel :: () {
   workspace := compiler_create_workspace("Kernel");
   
   options := get_build_options(workspace);
   options.output_type = .DYNAMIC_LIBRARY;
   options.dead_code_elimination = .ALL;
   options.output_executable_name = "kernel";
   options.output_path = "../build";
   options.intermediate_path = "../build/temp";
   options.entry_point_name = "kernel_entry";
   
   set_optimization(*options, .VERY_DEBUG);
   
   import_path: [..]string;
   array_add(*import_path, ..options.import_path);
   array_add(*import_path, "modules");
   options.import_path = import_path;
   
   set_build_options(options, workspace);
   
   add_build_file("kernel/kernel.jai", workspace);
}

build_loader :: () {
   workspace := compiler_create_workspace("Loader");
   
   options := get_build_options(workspace);
   options.output_type = .DYNAMIC_LIBRARY;
   options.dead_code_elimination = .ALL;
   options.output_executable_name = "loader";
   options.output_path = "../build";
   options.intermediate_path = "../build/temp";
   options.entry_point_name = "_start";
   options.use_custom_link_command = true;
   
   set_optimization(*options, .OPTIMIZED_VERY_SMALL);
   
   options.llvm_options.command_line = .["", "--relocation-model=pic"];
   
   import_path: [..]string;
   array_add(*import_path, ..options.import_path);
   array_add(*import_path, "modules");
   options.import_path = import_path;
   
   set_build_options(options, workspace);
   
   run_command("nasm", "-g", "-f", "elf64", "src/kernel/_start.s", "-o", "build/loader.o", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   
   compiler_begin_intercept(workspace);
   add_build_file("kernel/loader.jai", workspace);
   while true {
      msg := compiler_wait_for_message();
      if msg.kind == {
         case .PHASE;
            message: *Message_Phase = xx msg;
            if message.phase == .READY_FOR_CUSTOM_LINK_COMMAND {
               arguments: [..]string;
               array_add(*arguments, "lld-linux", "-flavor", "Gnu", "-nostdlib", "-Bsymbolic", "-shared", "-Tscripts/elf_x86_64_efi.lds", "-o", "build/loader.so", "build/loader.o");
               array_add(*arguments, ..message.compiler_generated_object_files);
               print("%\n", arguments);
               run_command(..arguments, working_directory="..", capture_and_return_output=true, print_captured_output=true);
               compiler_custom_link_command_is_complete(workspace);
            }
            
         case .COMPLETE;
            break;
      }
   }
   compiler_end_intercept(workspace);
}

build_image :: () {
   run_command("bash", "./scripts/build.sh", working_directory="..", capture_and_return_output=true, print_captured_output=true);
}

run_debug_linux :: () {
   pid := fork();
   
   if pid == 0 {
      run_command("konsole", "-e", "\"gdb\"", working_directory="..");
   }
   
   run_command("qemu-system-x86_64", "-cpu", "qemu64", "-bios", "./emulator/OVMF.fd", "-drive", "if=ide,file=./emulator/disk.vhd", "-device", "qemu-xhci,id=xhci", "-m", "1G", "-s", "-S", working_directory="..");
}

#run {
   set_build_options_dc(.{do_output = false, write_added_strings = false});
   
   options := get_build_options();
   
   if options.compile_time_command_line.count {
      if options.compile_time_command_line[0] == {
         case "build";
            build_kernel();
            build_loader();
         case "run";
            #if OS == .LINUX {
               build_image();
               run_debug_linux();
            }
      }
   }
}