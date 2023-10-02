//
// An array where each element is packed tightly between bytes. When retrieving an element,
// the first Bits_Per_Entry bits are filled from the array, and the rest are zeroed.
//

Packed_Array :: struct(Entry_Type: Type, ENTRY_COUNT: int, BITS_PER_ENTRY: int) {
   BYTE_COUNT :: (ENTRY_COUNT * BITS_PER_ENTRY + 7) / 8;
   
   bytes: [BYTE_COUNT] u8;
}

operator [] :: inline (using arr: Packed_Array($T, $N, $B), index: u64) -> T {
   return arr[xx index];
}

operator [] :: (using arr: Packed_Array($T, $N, $B), index: int) -> T {
   #assert 0 < B && B <= 8 * size_of(T);
   #assert 0 < N;
   assert(0 <= index && index < N);
   
   BYTE_ENTRY_MASK :: #ifx B <= 8 then (1 << B) - 1; else 0xFF;
   BYTES_PER_ENTRY :: (B + 7) / 8;
   
   bit_index  := index * B;
   byte_index := bit_index / 8;
   low_shift  := bit_index % 8;
   
   bit_index_end := bit_index + B;
   byte_index_end := bit_index_end / 8;
   
   result: T;
   out: *u8 = xx result;
   
   #if B == 8 {
      out[0] = bytes[byte_index];
   } else #if B % 8 == 0 {
      memcpy(out, bytes.data + byte_index, BYTES_PER_ENTRY);
   } else #if B == 1 || B == 2 || B == 4 {
      out[0] = (bytes[byte_index] >> low_shift) & BYTE_ENTRY_MASK;
   } else if byte_index == byte_index_end {
      out[0] = (bytes[byte_index] >> low_shift) & BYTE_ENTRY_MASK;
   } else {
      high_shift := 8 - low_shift;
      end_mask := (1 << (bit_index_end % 8)) - 1;
      buffer: u16 = xx (bytes[byte_index] >> low_shift);
      
      // TODO: bitmove, larger sizes
      for byte_index+1..byte_index_end-1 {
         buffer |= (cast(u16) bytes[it]) << high_shift;
         out[it_index] = xx buffer;
         buffer >>= 8;
      }
      
      buffer |= (cast(u16) bytes[byte_index_end]) << high_shift;
      out[(B - 1) / 8] = xx (buffer & end_mask);
   }
   
   return result;
}

operator []= :: inline (using arr: Packed_Array($T, $N, $B), index: u64, value: T) {
   arr[xx index] = T;
}

operator []= :: (using arr: Packed_Array($T, $N, $B), index: int, value: T) {
   #assert 0 < B && B <= 8 * size_of(T);
   #assert 0 < N;
   assert(0 <= index && index < N);
   
   BYTE_ENTRY_MASK :: #ifx B <= 8 then (1 << B) - 1; else 0xFF;
   BYTES_PER_ENTRY :: (B + 7) / 8;
   
   bit_index   := index * B;
   byte_index  := bit_index / 8;
   start_shift := bit_index % 8;
   start_mask  := ~(BYTE_ENTRY_MASK << start_shift);
   
   bit_index_end  := bit_index + B;
   byte_index_end := bit_index_end / 8;
   end_shift      := bit_index_end % 8;
   end_mask       := ~((1 << end_shift) - 1);
   
   buffer: u16 = xx (bytes[byte_index] & start_mask);
   value_bytes: *u8 = xx *value;
   
   #if B == 8 {
      bytes[byte_index] = value_bytes[0];
   } else #if B % 8 == 0 {
      memcpy(bytes.data + byte_index, value_bytes, BYTES_PER_ENTRY);
   } else #if B == 1 || B == 2 || B == 4 {
      bytes[byte_index] = xx (buffer | (value_bytes[0] << start_shift));
   } else if byte_index == byte_index_end {
      bytes[byte_index] = buffer | (value_bytes[0] << start_shift);
   } else {
      end_byte_mask := ~(BYTE_ENTRY_MASK << end_shift);
      
      // TODO: bitmove, larger sizes
      for byte_index..byte_index_end-1 {
         buffer |= (cast(u16) value_bytes[it_index]) << start_shift;
         bytes[it] = xx buffer;
         buffer >>= 8;
      }
      
      buffer &= end_mask;
      buffer |= (cast(u16) bytes[byte_index_end]) & end_mask;
      bytes[byte_index_end] = xx buffer;
   }
}

#scope_file

#import "Util";