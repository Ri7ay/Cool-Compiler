(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

 class List {
   isNil() : Bool { true };

   head()  : String { { abort(); ""; } };

   tail()  : List { { abort(); self; } };

   add(i : String) : List {
      (new Node).init(i, self)
   };
};


class Node inherits List {

  car : String;

  cdr : List;

  isNil() : Bool { false };

  head()  : String { car };

  tail()  : List { cdr };

  init(i : String, rest : List) : List {
    {
	   car <- i;
	   cdr <- rest;
	   self;
    }
  };

};

class Main inherits IO {
   stack: List <- new List;
   convert: A2I <- new A2I;

   print_list(l : List) : Object {
      if l.isNil() 
         then ""
         else {
            out_string(l.head());
            out_string("\n");
            print_list(l.tail());
         }
      fi
   };

   plus(l : List) : List {
         let num_lhs : Int, num_rhs : Int, res: String in 
         {
            num_lhs <- convert.a2i(l.head());
            l <- l.tail();
            num_rhs <- convert.a2i(l.head());
            l <- l.tail();
            l <- l.add(convert.i2a(num_lhs + num_rhs));
         }
   };

   swap(l : List) : List {
      let first : String, second : String in 
      {
         first <- l.head();
         l <- l.tail();
         second <- l.head();
         l <- l.tail();
         l <- l.add(first).add(second);
      } 
   };

   exec(l: List) : List {
      let cmd : String in 
       {
         if l.isNil()
            then l
            else  {
               cmd <- l.head();
               l <- l.tail();
               if cmd = "s"
                  then l <- swap(l)
                  else {
                     if cmd = "+"
                        then l <- plus(l)
                        else l <- l.add(cmd)
                     fi;
                  }
               fi;
            }
         fi;
       }
   };

   main() : Object {
      {
         let str: String, end: Bool <- false in
         while (not end) loop {
            str <- in_string();
            if str = "x"
               then end <- true
               else {
                  if str = "d"
                     then print_list(stack)
                     else {
                        if str = "e"
                           then stack <- exec(stack)
                           else stack <- stack.add(str)
                        fi;
                     }
                  fi;
               }
            fi;
         } 
         pool;
      }
   };

};
