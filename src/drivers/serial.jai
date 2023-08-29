Serial_Port :: #type,distinct u16;

initialize_serial_port :: (serial_port: Serial_Port, divisor: u16) -> bool {
   test_byte: u8: 0xAE;
   
   // We're using this for offsets, but not actually
   // dereferencing it.
   port: *Serial_Port_Registers = xx serial_port;
   
   // Disable interrupts
   out8(xx *port.interrupt_enable, 0);
   
   // Setup the baud rate
   out8(xx *port.line_control, xx Line_Control.DIVISOR_LATCH_ACCESS);
   out8(xx *port.divisor_low, xx (divisor & 0xFF));
   out8(xx *port.divisor_high, xx (divisor >> 8));
   
   // Configure the line and FIFO
   out8(xx *port.line_control, xx Line_Control.DATA_IS_8_BITS | .STOP_IS_1_BIT | .NO_PARITY);
   out8(xx *port.fifo_control, xx FIFO_Control.ENABLE | .CLEAR_RECEIVE | .CLEAR_TRANSMIT | .INTERRUPT_AT_14_BYTES);
   
   // Do a loopback test
   out8(xx *port.modem_control, xx Modem_Control.LOOP_BACK | .REQUEST_TO_SEND | .OUT_1 | .OUT_2);
   out8(xx *port.data, test_byte);
   if in8(xx *port.data) != test_byte return false;
   
   // Configure the modem
   out8(xx *port.modem_control, xx Modem_Control.DATA_TERMINAL_READY | .REQUEST_TO_SEND | .OUT_1 | .OUT_2);
   return true;
}

initialize_serial_port :: (baud_rate: int) -> port: Serial_Port, successful: bool {
   if baud_rate <= 0 return 0, false;
   if baud_rate > max_baud_rate baud_rate = max_baud_rate;
   
   divisor: u16 = xx (max_baud_rate / baud_rate);
   
   for ports {
      if initialize_serial_port(it, divisor) {
         return it, true;
      }
   }
   
   return 0, false;
}

read_char :: (port: Serial_Port) -> u8 {
   registers: *Serial_Port_Registers = xx port;
   while !(in8(xx *registers.line_status) & xx Line_Status.DATA_READY) { }
   return in8(xx port);
}

write_char :: (port: Serial_Port, char: u8) {
   registers: *Serial_Port_Registers = xx port;
   while !(in8(xx *registers.line_status) & xx Line_Status.TRANSMISSION_BUFFER_EMPTY) { }
   out8(xx port, char);
}

write_string :: (port: Serial_Port, str: string) {
   for 0..str.count-1 {
      write_char(port, str.data[it]);
   }
}

serial_logger :: (message: string, data: *void, info: Log_Info) {
   write_string(xx data, message);
}

#scope_file

// Most common port addresses
ports :: Serial_Port.[0x3F8, 0x2F8, 0x3E8, 0x2E8, 0x5F8, 0x4F8, 0x5E8, 0x4E8];

max_baud_rate :: 115200;

// This is purely for reference, since the actual data
// is accessed by IO.
Serial_Port_Registers :: struct {
   data: u8;
   
   interrupt_enable: Interrupt_Enable;
   
   fifo_control: FIFO_Control; // Write-Only
   
   line_control: Line_Control;
   modem_control: Modem_Control;
   
   line_status: Line_Status;
   modem_status: Modem_Status;
   
   scratch: u8;
   
   #place data;
   divisor_low: u8 = ---;
   
   #place interrupt_enable;
   divisor_high: u8 = ---;
   
   #place fifo_control;
   interrupt_identification: Interrupt_Identification = ---; // Read-Only
}

Interrupt_Enable :: enum_flags u8 {
   DATA_AVAILABLE    :: 1 << 0;
   TRANSMITTER_EMPTY :: 1 << 1;
   BREAK_OR_ERROR    :: 1 << 2;
   STATUS_CHANGE     :: 1 << 3;
   SLEEP_MODE        :: 1 << 4; // 16750
   LOW_POWER_MODE    :: 1 << 5; // 16750
}

FIFO_Control :: enum_flags u8 {
   ENABLE          :: 1 << 0;
   CLEAR_RECEIVE   :: 1 << 1;
   CLEAR_TRANSMIT  :: 1 << 2;
   DMA_MODE_SELECT :: 1 << 3;
   ENABLE_64_BIT   :: 1 << 5; // 16750
   
   INTERRUPT_TRIGGER_LEVEL_MASK :: 3 << 6;
   INTERRUPT_AT_1_BYTE          :: 0 << 6;
   INTERRUPT_AT_4_BYTES         :: 1 << 6;
   INTERRUPT_AT_8_BYTES         :: 2 << 6;
   INTERRUPT_AT_14_BYTES        :: 3 << 6;
}

Interrupt_Identification :: enum_flags u8 {
   NO_INTERRUPT_PENDING :: 1 << 0;
   
   INTERRUPT_TYPE_MASK    :: 7 << 1;
   STATUS_CHANGED         :: 0 << 1;
   TRANSMITTER_IS_EMPTY   :: 1 << 1;
   DATA_IS_AVAILABLE      :: 2 << 1;
   BREAK_OR_ERROR_OCCURED :: 3 << 1;
   CHARACTER_TIMEOUT      :: 6 << 1; // 16550
   
   FIFO_64_ENABLED :: 1 << 5; // 16750
   
   FIFO_STATUS_MASK :: 3 << 6;
   FIFO_NOT_FOUND   :: 0 << 6;
   FIFO_UNUSABLE    :: 1 << 6;
   FIFO_ENABLED     :: 3 << 6;
}

Line_Control :: enum_flags u8 {
   DATA_IS_5_BITS :: 0 << 0;
   DATA_IS_6_BITS :: 1 << 0;
   DATA_IS_7_BITS :: 2 << 0;
   DATA_IS_8_BITS :: 3 << 0;
   DATA_SIZE_MASK :: 3 << 0;
   
   STOP_IS_1_BIT  :: 0 << 2;
   STOP_IS_2_BITS :: 1 << 2;
   
   NO_PARITY   :: 0 << 3;
   ODD_PARITY  :: 1 << 3;
   EVEN_PARITY :: 3 << 3;
   HIGH_PARITY :: 5 << 3;
   LOW_PARITY  :: 7 << 3;
   PARITY_MASK :: 7 << 3;
   
   BREAK_ENABLE :: 1 << 6;
   
   DIVISOR_LATCH_ACCESS :: 1 << 7;
}

Modem_Control :: enum_flags u8 {
   DATA_TERMINAL_READY :: 1 << 0;
   REQUEST_TO_SEND     :: 1 << 1;
   OUT_1               :: 1 << 2;
   OUT_2               :: 1 << 3;
   LOOP_BACK           :: 1 << 4;
   AUTO_FLOW           :: 1 << 5; // 16750
}

Line_Status :: enum_flags u8 {
   DATA_READY                :: 1 << 0;
   OVERRUN_ERROR             :: 1 << 1;
   PARITY_ERROR              :: 1 << 2;
   FRAMING_ERROR             :: 1 << 3;
   BREAK_ERROR               :: 1 << 4;
   TRANSMISSION_BUFFER_EMPTY :: 1 << 5;
   TRANSMITTER_EMPTY         :: 1 << 6;
   IMPENDING_ERROR           :: 1 << 7;
}

Modem_Status :: enum_flags u8 {
   DELTA_CLEAR_TO_SEND             :: 1 << 0;
   DELTA_DATA_SET_READY            :: 1 << 1;
   TRAILING_EDGE_OF_RING_INDICATOR :: 1 << 2;
   DELTA_DATA_CARRIER_DETECT       :: 1 << 3;
   CLEAR_TO_SEND                   :: 1 << 4;
   DATA_SET_READY                  :: 1 << 5;
   RING_INDICATOR                  :: 1 << 6;
   DATA_CARRIER_DETECT             :: 1 << 7;
}