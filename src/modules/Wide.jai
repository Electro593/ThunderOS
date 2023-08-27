//
// This module adds some utilities for converting values into
// wide strings at compile-time.
//

wide :: ($str: string) -> *u16 #expand {
   return #insert -> string {
      builder: Basic.String_Builder;
      Basic.init_string_builder(*builder);
      Basic.append(*builder, "u16.[");
      
      for 0..str.count-1 {
         Basic.print_to_builder(*builder, "%,", str.data[it]);
      }
      
      Basic.append(*builder, "0].data;");
      return Basic.builder_to_string(*builder);
   }
}

wide :: (val: $U, $radix := 10) -> *u16 #modify {
   info: *Type_Info_Integer = xx U;
   return info.type == .INTEGER && !info.signed && radix >= 2 && radix <= 64;
} #c_call {
   encoding_map :: "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";
   
   #if radix == 2 || radix == 16 {
      prefix_size :: 2;
   } else #if radix == 8 {
      prefix_size :: 1;
   } else {
      prefix_size :: 0;
   }
   
   value_size: int : xx #run (Math.ceil(Math.log(256) / Math.log(cast(float64) radix) * xx size_of(U)));
   
   buf :: #insert -> string {
      builder: Basic.String_Builder;
      Basic.init_string_builder(*builder);
      Basic.append(*builder, "u16.[");
      for 1..prefix_size Basic.append(*builder, "0,");
      for 1..value\_size Basic.append(*builder, "0,");
      #if radix != 2 && radix != 8 && radix != 10 && radix != 16 {
         suffix := Basic.tprint(" (base %)", radix);
         for 0..suffix.count-1 Basic.print_to_builder(*builder, "%,", cast(u16) suffix[it]);
      }
      Basic.append(*builder, "0,];");
      return Basic.builder_to_string(*builder);
   }
   
   print_char :: (char: u16) #expand {
      `index -= 1;
      `buf.data[`index] = char;
   }
   
   index := prefix_size + value_size;
   
   if val == 0 {
      print_char(encoding_map[0]);
   } else {
      while val > 0 {
         print_char(encoding_map[val % radix]);
         val /= radix;
      }
   }
   
   #if radix == 2 {
      print_char(#char "b");
      print_char(#char "0");
   } else #if radix == 8 {
      print_char(#char "0");
   } else #if radix == 16 {
      print_char(#char "x");
      print_char(#char "0");
   }
   
   return buf.data + index;
}

wide :: (val: $U) -> *u16 #modify {
   info: *Type_Info_Integer = xx U;
   return info.type == .INTEGER && info.signed;
} #c_call {
   value_size: int : xx #run (Math.ceil(2.40823996531 * xx size_of(U)));
   
   buf :: #insert -> string {
      builder: Basic.String_Builder;
      Basic.init_string_builder(*builder);
      Basic.append(*builder, "u16.[0,");
      for 1..value_size Basic.append(*builder, "0,");
      Basic.append(*builder, "0,];");
      return Basic.builder_to_string(*builder);
   }
   
   print_char :: (char: u16) #expand {
      `index -= 1;
      `buf.data[`index] = char;
   }
   
   index := 1 + value_size;
   
   is_negative := val < 0;
   
   if val == 0 {
      print_char(#char "0");
   } else {
      while val != 0 {
         print_char(cast(u16) (val % 10) + #char "0");
         val /= 10;
      }
   }
   
   if is_negative print_char(#char "-");
   
   return buf.data + index;
}

wide :: (val: $E) -> *u16 #modify {
   info: *Type_Info_Enum = xx E;
   return info.type == .ENUM && !(info.enum_type_flags & .FLAGS);
} #c_call {
   #insert -> string {
      info := type_info(E);
      
      builder: Basic.String_Builder;
      Basic.init_string_builder(*builder);
      Basic.append(*builder, "if val == {\n");
      
      for name: info.names {
         for 0..it_index-1 if info.values[it_index] == info.values[it] continue name;
         Basic.print_to_builder(*builder, "case .%; return wide(\"%\");\n", name, name);
      }
      
      internal_info := type_info(E).internal_type;
      bits := internal_info.runtime_size * 8;
      if internal_info.signed {
         Basic.print_to_builder(*builder, "case; return wide(cast(s%) val);\n", bits);
      } else {
         Basic.print_to_builder(*builder, "case; return wide(cast(u%) val, 16);\n", bits);
      }
      Basic.append(*builder, "}\n");
      return Basic.builder_to_string(*builder);
   }
}

#scope_file

Basic :: #import "Basic";
Math :: #import "Math";