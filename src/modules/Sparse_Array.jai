Sparse_Array :: struct(Element_Type := u8) {
   data: *u8;
   entry_size: u64;
   count: u64;
}

for_expansion :: (using array: Sparse_Array, body: Code, flags: For_Flags) #expand {
   for `it_index: 0..count-1 {
      `it := array[xx `it_index];
      #insert body;
   }
}

operator[] :: (using array: Sparse_Array, index: int) -> *array.Element_Type #c_call #no_context {
   return ifx index < xx count then xx (data + xx index * entry_size) else null;
}