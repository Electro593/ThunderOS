#import "Basic";
#import "Compiler";
#import "File";
#import "Process";
#import "String";

// TODO: Auto-install nasm, qemu-full, gdb

build_kernel :: (delta: *Delta) {
   workspace := compiler_create_workspace("Kernel");
   
   options := get_build_options(workspace);
   options.output_type = .DYNAMIC_LIBRARY;
   options.output_executable_name = "kernel";
   options.output_path = "../build";
   options.intermediate_path = "../build/temp";
   options.entry_point_name = "_start";
   options.use_custom_link_command = true;
   
   // set_optimization(*options, .VERY_OPTIMIZED);
   set_optimization(*options, .VERY_DEBUG);
   
   import_path: [..]string;
   array_add(*import_path, ..options.import_path);
   array_add(*import_path, "modules");
   options.import_path = import_path;
   
   set_build_options(options, workspace);
   
   run_command("nasm", "-g", "-f", "elf64", "src/kernel/x64.s", "-o", "build/kernel.o", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   
   compiler_begin_intercept(workspace);
   add_build_file("kernel/kernel.jai", workspace);
   while true {
      msg := compiler_wait_for_message();
      if msg.kind == {
         case .PHASE;
            message: *Message_Phase = xx msg;
            if message.phase == {
               case .READY_FOR_CUSTOM_LINK_COMMAND;
                  arguments: [..]string;
                  array_add(*arguments, "lld-linux", "-flavor", "Gnu", "-nostdlib", "-Bsymbolic", "-shared", "-o", "build/kernel.so", "build/kernel.o");
                  array_add(*arguments, "-L", "/home/ariaseiler/Programming/ThunderOS_Jai", "-L", "/lib", "-L" ,"/lib64", "-L", "/usr/lib", "-L", "/usr/lib64", "-L", "/usr/lib/libfakeroot", "-L", "/usr/modules/");
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
   
   result, output := run_command("objdump", "-l", "-S", "-d", "-Mintel", "--source-comment", "build/kernel.so", working_directory="..", capture_and_return_output=true);
   write_entire_file("../build/kernel.s", output);
   
   result, output = run_command("objdump", "-x", "-t", "-T", "-r", "-R", "build/kernel.so", working_directory="..", capture_and_return_output=true);
   write_entire_file("../build/kernel.dump", output);
   
   delta.kernel_binaries = true;
}

build_loader :: (delta: *Delta) {
   workspace := compiler_create_workspace("Loader");
   
   options := get_build_options(workspace);
   options.output_type = .DYNAMIC_LIBRARY;
   options.output_executable_name = "loader";
   options.output_path = "../build";
   options.intermediate_path = "../build/temp";
   options.entry_point_name = "_start";
   options.use_custom_link_command = true;
   
   set_optimization(*options, .OPTIMIZED_VERY_SMALL);
   // set_optimization(*options, .VERY_DEBUG);
   
   import_path: [..]string;
   array_add(*import_path, ..options.import_path);
   array_add(*import_path, "modules");
   options.import_path = import_path;
   
   set_build_options(options, workspace);
   
   run_command("nasm", "-g", "-f", "elf64", "src/loader/x64.s", "-o", "build/loader.o", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   
   compiler_begin_intercept(workspace);
   add_build_file("loader/loader.jai", workspace);
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
   
   #if CPU == .X64 {
      efi_arch := "efi-app-x86_64";
   } else {
      #assert false, "Unsupported CPU";
   }
   
   run_command("objcopy", "-R", ".note*", "-R", ".comment", "-R", ".gnu*", "-R", ".hash", "-R", ".rela*", "-R", ".plt", "-R", ".eh_frame", "-R", ".dynsm", "-R", ".dynstr", "-R", ".debug*", "--target", efi_arch, "--subsystem=10", "build/loader.so", "build/loader.efi", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   run_command("objcopy", "-R", ".note*", "-R", ".comment", "-R", ".gnu*", "-R", ".hash", "-R", ".rela*", "-R", ".plt", "-R", ".eh_frame", "-R", ".dynsm", "-R", ".dynstr",                  "--target", efi_arch, "--subsystem=10", "build/loader.so", "build/loader.so", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   
   result, output := run_command("objdump", "-l", "-S", "-d", "-Mintel", "--source-comment", "build/loader.so", working_directory="..", capture_and_return_output=true);
   write_entire_file("../build/loader.s", output);
   
   result, output = run_command("objdump", "-x", "-t", "-r", "build/loader.so", working_directory="..", capture_and_return_output=true);
   write_entire_file("../build/loader.dump", output);
   
   delta.loader_binaries = true;
}

build_image :: (delta: *Delta) {
   // run_command("bash", "scripts/build.sh", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   result, output := run_command("mount", working_directory="..", capture_and_return_output=true);
   if !contains(output, "/mnt") {
      run_command("sudo", "qemu-nbd", "-c", "/dev/nbd0", "emulator/disk.vhd", working_directory="..", capture_and_return_output=true, print_captured_output=true);
      run_command("sudo", "mount", "-t", "auto", "-o", "rw", "/dev/nbd0p1", "/mnt", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   }
   
   if delta.kernel_binaries {
      run_command("sudo", "cp", "build/kernel.so", "/mnt/kernel.so", working_directory="..", capture_and_return_output=true, print_captured_output=true);
      delta.kernel_binaries = false;
   }
   if delta.loader_binaries {
      run_command("sudo", "cp", "build/loader.efi", "/mnt/EFI/BOOT/BOOTX64.efi", working_directory="..", capture_and_return_output=true, print_captured_output=true);
      delta.loader_binaries = false;
   }
   
   run_command("sudo", "umount", "/dev/nbd0p1", working_directory="..", capture_and_return_output=true, print_captured_output=true);
   run_command("sudo", "qemu-nbd", "-d", "/dev/nbd0", working_directory="..", capture_and_return_output=true, print_captured_output=true);
}

#if OS == .LINUX {
   run_debug_linux :: () {
      #import "POSIX";
         
      pid := fork();
      
      if pid == 0 {
         run_command("konsole", "-e", "\"gdb\"", working_directory="..");
      }
      
      run_command("qemu-system-x86_64",
         "-cpu", "qemu64",
         "-bios", "./emulator/OVMF.fd",
         "-drive", "if=ide,file=./emulator/disk.vhd",
         "-device", "qemu-xhci,id=xhci",
         "-serial", "stdio",
         "-m", "1G",
         "-s",
         "-S",
         working_directory="..", capture_and_return_output=true, print_captured_output=true
      );
   }
}

#run {
   set_build_options_dc(.{do_output = false, write_added_strings = false});
   
   options := get_build_options();
   
   make_directory_if_it_does_not_exist("../build");
   
   delta: Delta;
   delta_string, success := read_entire_file("../build/delta");
   if success {
      delta = (cast(*Delta) delta_string.data).*;
   }
   
   if options.compile_time_command_line.count {
      if options.compile_time_command_line[0] == {
         case "build";
            build_kernel(*delta);
            build_loader(*delta);
         case "run";
            if delta.loader_binaries || delta.kernel_binaries {
               build_image(*delta);
            }
            #if OS == .LINUX {
               run_debug_linux();
            }
      }
   }
   
   write_entire_file("../build/delta", *delta, size_of(Delta));
}

Delta :: struct {
   kernel_binaries: bool;
   loader_binaries: bool;
}