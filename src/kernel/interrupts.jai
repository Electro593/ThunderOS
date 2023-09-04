divide_error_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Divide Error (#DE)\n\n");
}

debug_exception :: (_: u32) {
   write_string(port, "Exception hit (trap/fault): Debug (#DB)\n\n");
}

non_maskable_interrupt :: (_: u32) {
   write_string(port, "Non-Maskable Interrupt hit\n\n");
}

breakpoint_exception :: (_: u32) {
   write_string(port, "Exception hit (trap): Breakpoint (#BP)\n\n");
}

overflow_exception :: (_: u32) {
   write_string(port, "Exception hit (trap): Overflow (#OF)\n\n");
}

bounds_range_exceeded_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Bound Range Exceeded (#BR)\n\n");
}

invalid_opcode_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Invalid Opcode (#UD)\n\n");
}

device_not_available_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Device Not Available (#NM)\n\n");
}

double_fault_exception :: (error_code: u32) {
   write_string(port, "Exception hit (abort): Double Fault (#DF)\n\tError Code: %\n\n");
}

invalid_tss_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Invalid TSS (#TS)\n\tError Code: %\n\n");
}

segment_not_present_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Segment Not Present (#NP)\n\tError Code: %\n\n");
}

stack_fault_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Stack Fault (#SS)\n\tError Code: %\n\n");
}

general_protection_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): General Protection (#GP)\n\tError Code: %\n\n");
}

page_fault_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Page Fault (#PF)\n\tError Code: %\n\n");
}

x87_floating_point_error_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): x87 Floating Point Error (#MF)\n\n");
}

alignment_check_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Alignment Check (#AC)\n\tError Code: %\n\n");
}

machine_check_exception :: (_: u32) {
   write_string(port, "Exception hit (abort): Machine Check (#MC)\n\n");
}

simd_floating_point_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): SIMD Floating-Point (#XM)\n\n");
}

virtualization_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Virtualization (#VE)\n\n");
}

control_protection_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Control Protection (#CP)\n\tError Code: %\n\n");
}

hypervisor_injection_exception :: (_: u32) {
   write_string(port, "Exception hit (fault): Hypervisor Injection (#HV)\n\n");
}

vmm_communication_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): VMM Communication (#VC)\n\tError Code: %\n\n");
}

security_exception :: (error_code: u32) {
   write_string(port, "Exception hit (fault): Security (#SX)\n\tError Code: %\n\n");
}

#scope_file

// TODO Remove
port: Serial_Port = xx 0x3F8;