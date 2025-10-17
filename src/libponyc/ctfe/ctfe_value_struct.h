#pragma once

#include <map>
#include <string>

#include "ctfe_value.h"
#include "ctfe_value_type_ref.h"

class CtfeValueStruct
{
  std::map<std::string, CtfeValue> m_vars;
  CtfeValueTypeRef m_type_ref;

public:
  CtfeValueStruct(const CtfeValueTypeRef& typeref);
  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value);
  bool get_value(const std::string& name, CtfeValue& value) const;

  const std::string get_struct_type_name() const { return m_type_ref.get_type_name(); }
};
