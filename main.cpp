#include <iostream>

#include "trs_string.h"
using namespace std;

int main(int argc, char** argv)
{
   if (argc < 2)
   {
      cout << "Usage: terse-string.exe <file> <output>\n";
      return 1;
   }

   const char* filename = argv[1];   
   trs::string test(filename);
   cout << "Loaded Terse file with: " << test.pageCount() << " scrolls.\n";
   cout << test.getNavTree();

   if (argc >= 3)
   {
      filename = argv[2];
      test.save(filename);
      cout << "Saved " << filename << endl;
   }
}