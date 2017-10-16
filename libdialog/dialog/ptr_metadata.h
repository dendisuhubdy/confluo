#ifndef DIALOG_DIALOG_PTR_METADATA_H_
#define DIALOG_DIALOG_PTR_METADATA_H_

namespace dialog {
namespace memory {

enum alloc_type : uint8_t {
  D_DEFAULT = 0,
  D_MMAP = 1
};

enum state_type : uint8_t {
  D_IN_MEMORY = 0,
  D_ARCHIVED = 1
};

typedef struct ptr_metadata {
  size_t size_ : 32;
  uint8_t thread_id_ : 8;
  state_type state_ : 2;
  alloc_type alloc_type_ : 1;

  /**
   * Get metadata associated with a pointer
   * @param ptr pointer
   * @return metadata
   */
  static ptr_metadata* get_metadata(void* ptr) {
    return static_cast<ptr_metadata*>(ptr) - 1;
  }

} ptr_metadata;

}
}

#endif /* DIALOG_DIALOG_PTR_METADATA_H_ */
