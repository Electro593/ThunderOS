/*

format: [ text | statement ]+
statement: '%' [ clause [ ',' clause ]+ ] [ ';' ]

Clauses:
  % - Insert a '%'
  0..N - Insert the argument from the given index
  [1] c - Print integers as characters
  [1,2,4,8] b [2..64] - Print integers in base-N

*/

Print_Format :: struct {
   escaped := false;
   
   argument_index := -1;
   
   print_as_characters_8 := false;
   
   radix_8: u8 = 10;
   radix_16: u8 = 10;
   radix_32: u8 = 10;
   radix_64: u8 = 10;
}

parse_format :: (format_string: string, i: *int) -> Print_Format {
   format: Print_Format;
   
   peek :: () -> u8 #expand {
      if `i.* >= `format_string.count then return #char ";";
      return `format_string.data[`i.*];
   }
   
   next :: () -> u8 #expand {
      if `i.* >= `format_string.count then return #char ";";
      defer `i.* += 1;
      return `format_string.data[`i.*];
   }
   
   next_number :: () -> int #expand {
      number := 0;
      while peek() >= #char "0" && peek() <= #char "9" {
         number *= 10;
         number += next() - #char "0";
      }
      return number;
   }
   
   if next() != #char "%" return format;
   
   while i.* < format_string.count {
      has_first := peek() >= #char "0" && peek() <= #char "9";
      first := next_number();
      
      if next() == {
         case #char "%";
            format.escaped = true;
         
         case #char "c";
            if !has_first then first = 1;
            if first & 1 then format.print_as_characters_8 = true;
            
         case #char "b";
            radix := next_number();
            if radix >= 2 && radix <= 64 {
               if !has_first then first = 15;
               if first & 1 then format.radix_8  = xx radix;
               if first & 2 then format.radix_16 = xx radix;
               if first & 4 then format.radix_32 = xx radix;
               if first & 8 then format.radix_64 = xx radix;
            }
         
         case;
            i.* -= 1;
            #through;
         case #char ";";
            defer return format;
            #through;
         case #char ",";
            if has_first then format.argument_index = first;
      }
   }
   
   return format;
}

print_bufferless :: (format_string: string, args: ..Any, loc := #caller_location) {
   string_to_print: string = format_string;
   string_to_print.count = 0;
   start_index := 0;
   
   log_info := Log_Info.{location = loc};
   
   arg_index := 0;
   
   for 0..format_string.count-1 {
      if format_string[it] != #char "%" continue;
      
      // Print the current text
      string_to_print.count = it - start_index;
      context.logger(string_to_print, context.logger_data, log_info);
      
      format := parse_format(format_string, *it);
      
      start_index = it;
      string_to_print.data = format_string.data + start_index;
      
      if format.argument_index >= 0 {
         arg_index = format.argument_index;
      }
      
      if format.escaped {
         context.logger("%", context.logger_data, log_info);
      } else {
         print_bufferless(args[arg_index], format, loc);
         arg_index += 1;
      }
   }
   
   string_to_print.count = format_string.count - start_index;
   context.logger(string_to_print, context.logger_data, log_info);
}

print_bufferless :: (arg: Any, format := Print_Format.{}, loc := #caller_location) {
   print_bufferless(arg.type, arg.value_pointer, format, loc);
}

#scope_file

print_bufferless :: (info: *Type_Info, value_pointer: *void, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   if info.type == {
      case .INTEGER;
         print_bufferless_integer(xx info, value_pointer, format, loc);
      
      case .FLOAT; // TODO
         context.logger("<float>", context.logger_data, log_info);
      
      case .BOOL;
         value := (cast(*bool) value_pointer).*;
         context.logger(ifx value then "true" else "false", context.logger_data, log_info);
      
      case .STRING;
         value := (cast(*string) value_pointer).*;
         context.logger(value, context.logger_data, log_info);
      
      case .POINTER;
         print_bufferless_pointer(xx info, value_pointer, format, loc);
      
      case .PROCEDURE; // TODO
         context.logger("<procedure>", context.logger_data, log_info);
      
      case .VOID;
         context.logger("void", context.logger_data, log_info);
      
      case .STRUCT;
         print_bufferless_struct(xx info, value_pointer, format, loc);
      
      case .ARRAY;
         print_bufferless_array(xx info, value_pointer, format, loc);
      
      case .OVERLOAD_SET; // TODO
         context.logger("<overload_set>", context.logger_data, log_info);
      
      case .ANY; // TODO
         context.logger("<any>", context.logger_data, log_info);
      
      case .ENUM;
         print_bufferless_enum(xx info, value_pointer, format, loc);
      
      case .POLYMORPHIC_VARIABLE; // TODO
         context.logger("<polymorphic_variable>", context.logger_data, log_info);
      
      case .TYPE; // TODO
         context.logger("<type>", context.logger_data, log_info);
      
      case .CODE; // TODO
         context.logger("<code>", context.logger_data, log_info);
      
      case .VARIANT;
         print_bufferless((cast(*Type_Info_Variant) info).variant_of, value_pointer, format, loc);
      
      case;
         context.logger("<unknown>", context.logger_data, log_info);
   }
}

print_bufferless_integer :: (info: *Type_Info_Integer, value_pointer: *int, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   signed, unsigned := get_integer_value(info, value_pointer);
   
   buffer := "00000000000000000000000000000000000000000000000000000000000000000000000000";
   index := buffer.count;
   
   if format.print_as_characters_8 && info.runtime_size == 1 {
      index -= 1;
      buffer[index] = xx unsigned;
   } else if info.signed {
      // Take the absolute value of the remainders
      encoding_map :: "9876543210123456789";
      
      negative := signed < 0;
      
      while index == buffer.count || signed != 0 {
         index -= 1;
         buffer[index] = xx encoding_map[signed % 10 + 9];
         signed /= 10;
      }
      
      if negative {
         index -= 1;
         buffer[index] = #char "-";
      }
   } else {
      encoding_map :: "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";
      
      radix := 10;
      if info.runtime_size == {
         case 1; radix = format.radix_8;
         case 2; radix = format.radix_16;
         case 4; radix = format.radix_32;
         case 8; radix = format.radix_64;
      }
      if radix < 2 || radix > 64 then radix = 10;
      
      if radix != 2 && radix != 8 && radix != 10 && radix != 16 {
         index -= 9;
         buffer[index + 8] = #char ")";
         buffer[index + 7] = encoding_map[radix % 10];
         if radix >= 10 {
            index -= 1;
            buffer[index + 7] = encoding_map[radix / 10];
         }
         buffer[index + 6] = #char " ";
         buffer[index + 5] = #char "e";
         buffer[index + 4] = #char "s";
         buffer[index + 3] = #char "a";
         buffer[index + 2] = #char "b";
         buffer[index + 1] = #char "(";
         buffer[index + 0] = #char " ";
      }
      
      while index == buffer.count || unsigned != 0 {
         index -= 1;   
         buffer[index] = encoding_map[unsigned % xx radix];
         unsigned /= xx radix;
      }
      
      if radix == {
         case 2;
            index -= 2;
            buffer[index + 1] = #char "b";
            buffer[index + 0] = #char "0";
         case 8;
            index -= 1;
            buffer[index] = #char "0";
         case 16;
            index -= 2;
            buffer[index + 1] = #char "x";
            buffer[index + 0] = #char "0";
      }
   }
   
   buffer.data += index;
   buffer.count -= index;
   context.logger(buffer, context.logger_data, log_info);
}

print_bufferless_pointer :: (info: *Type_Info_Pointer, value_pointer: *u64, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   value := value_pointer.*;
   
   encoding_map :: "0123456789ABCDEF";
   
   buffer := "0000000000000000";
   index := buffer.count;
   
   if value == 0 {
      context.logger("<null>", context.logger_data, log_info);
   } else {
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
}

print_bufferless_struct :: (info: *Type_Info_Struct, value_pointer: *u8, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   context.logger("{ ", context.logger_data, log_info);
   
   for info.members {
      if it_index > 0
         context.logger(", ", context.logger_data, log_info);
      context.logger(it.name, context.logger_data, log_info);
      context.logger(ifx it.flags & .CONSTANT then " : " else " = ", context.logger_data, log_info);
      
      print_bufferless(it.type, value_pointer + it.offset_in_bytes, format, loc);
   }
   
   context.logger(" }", context.logger_data, log_info);
}

print_bufferless_array :: (info: *Type_Info_Array, value_pointer: *[]u8, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   context.logger("[ ", context.logger_data, log_info);
   
   for 0..value_pointer.count-1 {
      if it > 0 then context.logger(", ", context.logger_data, log_info);
      print_bufferless(info.element_type, value_pointer.data + it * info.element_type.runtime_size, format, loc);
   }
   
   context.logger(" ]", context.logger_data, log_info);
}

print_bufferless_enum :: (info: *Type_Info_Enum, value_pointer: *int, format := Print_Format.{}, loc := #caller_location) {
   log_info := Log_Info.{location = loc};
   
   first := true;
   value := get_integer_value(info.internal_type, value_pointer);
   
   if info.enum_type_flags & .FLAGS {
      for info.values {
         //TODO masks (e.g. print .ABC_MASK & value)
         if (value & it) == it {
            if !first context.logger(" | ", context.logger_data, log_info);
            context.logger(".", context.logger_data, log_info);
            context.logger(info.names[it_index], context.logger_data, log_info);
            value &= ~it;
            first = false;
         }
      }
   } else {
      for info.values {
         if value == it {
            context.logger(".", context.logger_data, log_info);
            context.logger(info.names[it_index], context.logger_data, log_info);
            first = false;
            break;
         }
      }
      
      if first {
         print_bufferless(info.internal_type, value_pointer, format, loc);
      }
   }
}

get_integer_value :: (info: *Type_Info_Integer, value_pointer: *int) -> signed: s64, unsigned: u64 {
   if info.runtime_size == {
      case 1; return xx (cast(*s8)  value_pointer).*, xx (cast(*u8)  value_pointer).*;
      case 2; return xx (cast(*s16) value_pointer).*, xx (cast(*u16) value_pointer).*;
      case 4; return xx (cast(*s32) value_pointer).*, xx (cast(*u32) value_pointer).*;
      case 8; return xx (cast(*s64) value_pointer).*, xx (cast(*u64) value_pointer).*;
      case; return 0, 0;
   }
}