#ifndef BASE_MEMORY_LINKED_PTR_H_
#define BASE_MEMORY_LINKED_PTR_H_

class linked_ptr_internal {
 public:
  // Create a new circle that includes only this instance.
  void join_new() {
    next_ = this;
  }

  // Join an existing circle.
  void join(linked_ptr_internal const* ptr) {
    next_ = ptr->next_;
    ptr->next_ = this;
  }

  // Leave whatever circle we're part of.  Returns true iff we were the
  // last member of the circle.  Once this is done, you can join() another.
  bool depart() {
    if (next_ == this) return true;
    linked_ptr_internal const* p = next_;
    while (p->next_ != this) p = p->next_;
    p->next_ = next_;
    return false;
  }

 private:
  mutable linked_ptr_internal const* next_;
};

namespace google {

template <typename T>
class linked_ptr {
 public:
  typedef T element_type;

  // Take over ownership of a raw pointer.  This should happen as soon as
  // possible after the object is created.
  explicit linked_ptr(T* ptr = NULL) { capture(ptr); }
  ~linked_ptr() { depart(); }

  // Copy an existing linked_ptr<>, adding ourselves to the list of references.
  template <typename U> linked_ptr(linked_ptr<U> const& ptr) { copy(&ptr); }

  linked_ptr(linked_ptr const& ptr) {
    copy(&ptr);
  }

  // Assignment releases the old value and acquires the new.
  template <typename U> linked_ptr& operator=(linked_ptr<U> const& ptr) {
    depart();
    copy(&ptr);
    return *this;
  }

  linked_ptr& operator=(linked_ptr const& ptr) {
    if (&ptr != this) {
      depart();
      copy(&ptr);
    }
    return *this;
  }

  // Smart pointer members.
  void reset(T* ptr = NULL) {
    depart();
    capture(ptr);
  }
  T* get() const { return value_; }
  T* operator->() const { return value_; }
  T& operator*() const { return *value_; }
  // Release ownership of the pointed object and returns it.
  // Sole ownership by this linked_ptr object is required.
  T* release() {
    bool last = link_.depart();
    T* v = value_;
    value_ = NULL;
    return v;
  }

  bool operator==(const T* p) const { return value_ == p; }
  bool operator!=(const T* p) const { return value_ != p; }
  template <typename U>
  bool operator==(linked_ptr<U> const& ptr) const {
    return value_ == ptr.get();
  }
  template <typename U>
  bool operator!=(linked_ptr<U> const& ptr) const {
    return value_ != ptr.get();
  }

 private:
  template <typename U>
  friend class linked_ptr;

  T* value_;
  linked_ptr_internal link_;

  void depart() {
    if (link_.depart()) delete value_;
  }

  void capture(T* ptr) {
    value_ = ptr;
    link_.join_new();
  }

  template <typename U> void copy(linked_ptr<U> const* ptr) {
    value_ = dynamic_cast<T*>(ptr->get());
    if (value_)
      link_.join(&ptr->link_);
    else
      link_.join_new();
  }
};

template<typename T> inline
bool operator==(T* ptr, const linked_ptr<T>& x) {
  return ptr == x.get();
}

template<typename T> inline
bool operator!=(T* ptr, const linked_ptr<T>& x) {
  return ptr != x.get();
}

// A function to convert T* into linked_ptr<T>
// Doing e.g. make_linked_ptr(new FooBarBaz<type>(arg)) is a shorter notation
// for linked_ptr<FooBarBaz<type> >(new FooBarBaz<type>(arg))
template <typename T>
linked_ptr<T> make_linked_ptr(T* ptr) {
  return linked_ptr<T>(ptr);
}

}

#endif //BASE_MEMORY_LINKED_PTR_H_

