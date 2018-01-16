#include "memtracker.h"
#include <cstdio>

UastIterator* MemTracker::CurrentIterator() { return currentIter_; }
bool MemTracker::CurrentIteratorSet() { return currentIter_ != nullptr; }
void MemTracker::ClearCurrentIterator() { currentIter_ = nullptr; }
void MemTracker::EnterFilter() { inFilter_ = true; }
void MemTracker::ExitFilter() { inFilter_ = false; }

void MemTracker::SetCurrentIterator(UastIterator *iter, bool isNew) {
  currentIter_ = iter;

  if (isNew) {
    // Save the iterNode referenced on new iterator constructor
    AddIterRefNode(iterNewNode_.copy, iterNewNode_.ref);
    iterNewNode_.copy = nullptr;
    iterNewNode_.ref = nullptr;
  }
}

void MemTracker::AddIterRefNode(void *node, jobject ref) {
  if (!CurrentIteratorSet()) {
    // Iterator construction
    iterNewNode_.copy = node;
    iterNewNode_.ref = ref;
  } else {
    iterRefNodes_[currentIter_].push_back({node, ref});
  }
}

void MemTracker::AddUtilsNode(void *node) {
  if (inFilter_) {
    filterUtilsNodes_.push_back(node);
  } else {
    iterUtilsNodes_[currentIter_].push_back(node);
  }
}

void MemTracker::debugIterMem() {
  for (auto const& x : iterRefNodes_) {
    auto key = x.first;
    auto vect = x.second;
    for (auto &n : vect) {
      printf("DEBUG key: %p copy: %p ref: %p\n", key, n.copy, n.ref);
    }
  }
}

void MemTracker::DisposeMem(JNIEnv *env) {
  if (inFilter_) {
    for (auto &i : filterUtilsNodes_) {
      free(i);
      i = nullptr;
    }
  } else {
    for (auto &n : iterRefNodes_[currentIter_]) {
      env->DeleteGlobalRef(n.ref);
      free(n.copy);
      n.copy = nullptr;
    }

    for (auto &i : iterUtilsNodes_[currentIter_]) {
      free(i);
      i = nullptr;
    }
    iterRefNodes_.erase(currentIter_);
    ClearCurrentIterator();
  }
}

