#include <unordered_map>
#include <vector>

#include <jni.h>
#include "uast.h"

struct NodeRef {
  void *copy;
  jobject ref;
};

class MemTracker {
private:
  UastIterator *currentIter_ = nullptr;
  bool inFilter_ = false;

  // Nodes allocated and referenced in JVM during Iteration
  std::unordered_map<UastIterator*, std::vector<NodeRef>> iterRefNodes_;
  // Nodes allocated by the jni_utils during iteration (like ChildAt)
  std::unordered_map<UastIterator*, std::vector<void*>> iterUtilsNodes_;
  // Nodes allocated by the jni_utils during filter()
  std::vector<void*> filterUtilsNodes_;
  NodeRef iterNewNode_;

public:
  UastIterator *CurrentIterator();
  void SetCurrentIterator(UastIterator *iter, bool isNew);
  bool CurrentIteratorSet();
  void ClearCurrentIterator();
  void EnterFilter();
  void ExitFilter();
  void AddIterRefNode(void *node, jobject ref);
  void AddUtilsNode(void *node);
  void DisposeMem(JNIEnv *env);
  void debugIterMem();
};

