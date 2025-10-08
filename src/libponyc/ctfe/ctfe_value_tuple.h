#pragma once


class CtfeValue;

class CtfeValueTuple
{
  // We could have just used a std::vector but that would caused an unbreakable
  // circular dependency.
  CtfeValue* m_vars;
  size_t m_size;

public:
  CtfeValueTuple();
  CtfeValueTuple(size_t size);
  CtfeValueTuple(const CtfeValueTuple& val);
  CtfeValueTuple(CtfeValueTuple&& val);
  ~CtfeValueTuple();
  CtfeValueTuple& operator=(const CtfeValueTuple& val);
  bool update_value(size_t pos, const CtfeValue& value);
  bool get_value(size_t pos, CtfeValue& value) const;
};
