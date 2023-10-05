#ifndef	_SIMPLE_HASH_TABLE_H
#define	_SIMPLE_HASH_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define TABLE_MAX_LOAD 0.75
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)
#define ALLOCATE(type, count) \
    (type*) reallocate(NULL, 0, sizeof(type) * (count))
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }
  void* result = realloc(pointer, newSize);  // Copying would be performed if needed.
  return result;
}

typedef struct {
  const char* key;
  size_t length;
  uint32_t hash;
} String;

typedef struct {
  String* key;
  int value;  // Only integer is available.
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
};

void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

// Find an existing or empty entry by a given key.
Entry* findEntry(Entry* entries, int capacity, String* key) {
  uint32_t index = key->hash % capacity;  // Find the target entry.
  Entry* tombstone = NULL;
  for (;;) {
    Entry* entry = &entries[index];
    if (entry->key == NULL) {
      if (entry->value != INT_MAX) {  // Keeping searching until we find an empty slot.
        return tombstone != NULL ? tombstone : entry;  // Empty entry.
      } else {
        if (tombstone == NULL) tombstone = entry;  // We found a tombstone.
      }
    } else if (entry->key == key) {
      return entry;  // We found the key. 
    }
    index = (index + 1) % capacity;  // Linear probing (circular).
  }
}

void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = 0;
  }
  // Since the bucket number has changed, the existing elements should be re-mapped in the new table.
  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {  
    Entry* entry = &table->entries[i];

    // Ignore empty bucket.
    if (entry->key == NULL) continue;  

    // Find a slot in the new table.
    Entry* dest = findEntry(entries, capacity, entry->key);  

    // Update slot with the original data.
    dest->key = entry->key;  
    dest->value = entry->value;
    table->count++;  // Recalculate the table count.
  }
  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;  // Point to the new allocated memory.
  table->capacity = capacity;  // Update the new capacity.
}

bool tableSet(Table* table, String* key, int value) {
  if (table->count + 1 > table->capacity + TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);  // Double capacity.
    adjustCapacity(table, capacity);
  }
  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
 
  // Consider tombstones to be full buckets, so the table size won't reduced until it undergoes an "adjustCapacity", -
  // then the tombstones would be excluded and the table would be shrunk in certain cases.
  if (isNewKey && entry->value != INT_MAX) table->count++;  
  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableGet(Table* table, String* key, int* value) {
  if (table->count == 0) return false;
  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;
  *value = entry->value;
  return true;
}

bool tableDelete(Table* table, String* key) {
  if (table->count == 0) return false;
  
  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;  // No target found needs to be deleted.

  // Place a tombstone in the entry.
  entry->key = NULL;
  entry->value = INT_MAX;
  return true;
}

#endif
