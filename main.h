#if !defined(MAIN_H)

#if RAKSHAZA_INTERNAL
struct FileStructure {
  Uint32 content_size;
  void* content;
};

FileStructure read_file(const char* file_name);
void* free_file(void* bitmap_memory);
#endif

#endif
