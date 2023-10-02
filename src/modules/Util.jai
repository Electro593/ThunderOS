assert :: (condition: Code, message := "", args: ..Any, loc := #caller_location) #expand {
   if #insert condition {
      if !context.allocator.proc {
         context.assertion_failed(loc, message);
      }
   }
}