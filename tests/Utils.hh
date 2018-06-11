#ifndef TESTS_UTILS_HH_
#define TESTS_UTILS_HH_

template <typename T>
T& dangling_ref() noexcept
{
  return *(T*)((void*)(1));
}

#endif /* !TESTS_UTILS_HH_ */
