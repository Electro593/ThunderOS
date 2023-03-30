//
// An array where each element is packed tightly between bytes. When retrieving an element,
// the first Bits_Per_Entry bits are filled from the array, and the rest are zeroed.
//

Packed_Array :: struct(Entry_Count: int, Entry_Type: Type, Bits_Per_Entry: int) {
   bytes : [(Entry_Count * Bits_Per_Entry + 7) / 8] u8;
}

operator [] :: inline (using arr: Packed_Array($N, $T, $B), index: u64) -> T {
   return arr[xx index];
}

operator [] :: (using arr: Packed_Array($N, $T, $B), index: int) -> T {
   assert(index < N * B);
   
   res: T;
   out : *u8 = xx *res;
   
   value_mask :: ((1 << B) - 1);
   bit_index := index * B;
   byte_index := bit_index >> 3;
   low_shift := bit_index & 7;
   
   #if B & 7 == 0 {
      //TODO: memset
      for 0..(B >> 3)-1 out[it] = bytes[byte_index + it];
   } else if byte_index == byte_index_end {
      out[0] = (bytes[byte_index] >> low_shift) & value_mask;
   } else {
      high_shift := 8 - low_shift;
      bit_index_end := bit_index + B;
      byte_index_end := bit_index_end >> 3;
      buffer : u16 = bytes[byte_index] >> low_shift;
      
      for byte_index+1..byte_index_end-1 {
         buffer |= cast(u16) bytes[it] << high_shift;
         out[it_index] = buffer;
         buffer >>= 8;
      }
      
      buffer |= cast(u16) bytes[byte_index_end] << high_shift;
      out[B >> 3] = buffer & (1 << (bit_index_end & 7)) - 1;
   }
   
   return res;
}

operator []= :: inline (using arr: Packed_Array($N, $T, $B), index: u64, value: T) {
   arr[xx index] = value;
}

operator [] :: (using arr: Packed_Array($N, $T, $B), index: int, value: T) {
   assert(index < N * B);
   
   data : *u8 = xx *value;
   
   value_mask :: ((1 << B) - 1);
   bit_index := index * B;
   byte_index := bit_index >> 3;
   low_shift := bit_index & 7;
   
   #if B & 7 == 0 {
      //TODO: memset
      for 0..(B >> 3)-1 bytes[byte_index + it] = out[it];
   } else if byte_index == byte_index_end {
      masked_data := (data[0] & value_mask) << low_shift;
      masked_byte := bytes[byte_index] & ~(value_mask << low_shift);
      bytes[byte_index] = masked_byte | masked_data;
      out[0] = (bytes[byte_index] >> low_shift) & ((1 << B) - 1);
   } else {
      high_shift := 8 - low_shift;
      bit_index_end := bit_index + B;
      byte_index_end := bit_index_end >> 3;
      end_shift := bit_index_end & 7;
      buffer : u16 = bytes[byte_index] & ((1 << low_shift) - 1);
      
      for byte_index+1..byte_index_end-1 {
         buffer |= cast(u16) data[it_index] << low_shift;
         bytes[it] = buffer;
         buffer >>= 8;
      }
      
      buffer |= (data[B >> 3] & value_mask) << low_shift;
      buffer |= bytes[byte_index_end] & ((1 << (low_shift + end_shift)) - 1);
      bytes[byte_index_end] = buffer;
   }
}