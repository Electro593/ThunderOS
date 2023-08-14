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
   options.dead_code_elimination = .ALL;
   options.output_executable_name = "loader.so";
   options.output_path = "../build";
   options.intermediate_path = "../build/temp";
   options.entry_point_name = "start";
   
   import_path: [..]string;
   array_add(*import_path, ..options.import_path);
   array_add(*import_path, "modules");
   options.import_path = import_path;
   
   set_build_options(options, workspace);
   
   add_build_file("kernel/loader.jai", workspace);
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