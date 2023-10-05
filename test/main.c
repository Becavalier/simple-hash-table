#include <stdio.h>
#include "../hash-table.h"


static String* makeString(const char* str) {
  String* ptr = (String*) malloc(sizeof(String));
  size_t strLen = strlen(str);
  ptr->key = str;
  ptr->length = strLen;
  ptr->hash = hashString(str, strLen);
  return ptr;
}

int main(int argc, char** agrv) {
  // Init a key.
  String* strObj = makeString("Hello, world!");

  // Init a hash-table.
  Table table;
  initTable(&table);

  // Save KV into the hash-table.
  tableSet(&table, strObj, 100);

  // Retrieve value from the hash-table.
  int value;
  bool result = tableGet(&table, strObj, &value);
  if (result) printf("%d\n", value);
  return 0;
}
