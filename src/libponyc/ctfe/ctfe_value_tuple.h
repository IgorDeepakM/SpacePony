#pragma once

#include <cstddef>


class CtfeValue;

class CtfeValueTuple
{
  // We could have just used a std::vector but that would caused an unbreakable
  // circular dependency.
  CtfeValue* m_vars;
  size_t m_size;

  friend void swap(CtfeValueTuple& a, CtfeValueTuple& b);

public:
  CtfeValueTuple();
  CtfeValueTuple(size_t size);
  CtfeValueTuple(const CtfeValueTuple& val);
  CtfeValueTuple(CtfeValueTuple&& val) noexcept;
  ~CtfeValueTuple();
  CtfeValueTuple& operator=(CtfeValueTuple val);
  bool update_value(size_t pos, const CtfeValue& value);
  bool get_value(size_t pos, CtfeValue& value) const;
};
