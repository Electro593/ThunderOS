print_bufferless :: (format: string, args: ..Any, loc := #caller_location) {
   string_to_print: string = format;
   string_to_print.count = 0;
   start_index := 0;
   
   log_info := Log_Info.{location = loc};
   
   arg_index := 0;
   
   for 0..format.count-1 {
      if format[it] != #char "%" continue;
      
      // Escaped %%
      if it > 0 && start_index == it continue;
      
      // Print the standard string
      string_to_print.count = it - start_index;
      context.logger(string_to_print, context.logger_data, log_info);
      start_index = it + 1;
      string_to_print.data = format.data + start_index;
      
      // Print the special value
      if args[arg_index].type.type == {
         case .INTEGER;
            context.logger("<integer>", context.logger_data, log_info);
         
         case .FLOAT;
            context.logger("<float>", context.logger_data, log_info);
         
         case .BOOL;
            context.logger("<bool>", context.logger_data, log_info);
         
         case .STRING;
            context.logger("<string>", context.logger_data, log_info);
         
         case .POINTER;
            value := (cast(*u64) args[arg_index].value_pointer).*;
            if value == 0 {
               context.logger("<null>", context.logger_data, log_info);
            } else {
               encoding_map :: "0123456789ABCDEF";
               buffer := "0000000000000000";
               index := buffer.count;
               while value > 0 {
                  index -= 1;
                  buffer[index] = encoding_map[value % 16];
                  value /= 16;
               }
               buffer.data += index;
               buffer.count -= index;
               context.logger("0x", context.logger_data, log_info);
               context.logger(buffer, context.logger_data, log_info);
            }
         
         case .PROCEDURE;
            context.logger("<procedure>", context.logger_data, log_info);
         
         case .VOID;
            context.logger("<void>", context.logger_data, log_info);
         
         case .STRUCT;
            context.logger("<struct>", context.logger_data, log_info);
         
         case .ARRAY;
            context.logger("<array>", context.logger_data, log_info);
         
         case .OVERLOAD_SET;
            context.logger("<overload_set>", context.logger_data, log_info);
         
         case .ANY;
            context.logger("<any>", context.logger_data, log_info);
         
         case .ENUM;
            context.logger("<enum>", context.logger_data, log_info);
         
         case .POLYMORPHIC_VARIABLE;
            context.logger("<polymorphic_variable>", context.logger_data, log_info);
         
         case .TYPE;
            context.logger("<type>", context.logger_data, log_info);
         
         case .CODE;
            context.logger("<code>", context.logger_data, log_info);
         
         case .VARIANT;
            context.logger("<variant>", context.logger_data, log_info);
         
         case;
            context.logger("<unknown>", context.logger_data, log_info);
      }
      arg_index += 1;
   }
   
   string_to_print.count = format.count - start_index;
   context.logger(string_to_print, context.logger_data, log_info);
}