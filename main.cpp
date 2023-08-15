#include <iostream>

#include "trs_string.h"
using namespace std;

int main(int argc, char** argv)
{
   if (argc < 2)
   {
      cout << "Usage: terse-string.exe <file> -<command> <...>\n";
      return 1;
   }

   const char* filename = argv[1];   
   trs::string test(filename);
   cout << "Loaded Terse file with: " << test.pageCount() << " scrolls and " << test.size() << " bytes.\n";

   if (argc >= 3)
   {
      const char* possible = argv[2];
      if (possible[0] == '-')
      {
         std::string command(&possible[1]);
         if (command == "expand")
         {
            cout << "Expanding " << filename;
            test.expand();
            test.save(filename);
         }
         else if (command == "contract")
         {
            cout << "Contracting " << filename;
            test.contract();
            test.save(filename);
         }
         else if (command == "output")
         {
            if (argc >= 4)
            {
               filename = argv[3];
            }
            test.save(filename);
            cout << "Saved " << filename << endl;            
         }
         else
         {
            cout << "Unknown command: " << command;
         }
      }
   }

   cout << test.getNavTree();

   return 0;
}