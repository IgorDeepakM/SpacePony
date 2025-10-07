#pragma once

#include <map>
#include <string>

#include "ctfe_value.h"

class CtfeValueStruct
{
  std::map<std::string, CtfeValue> m_vars;

public:
  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value);
  bool get_value(const std::string& name, CtfeValue& value) const;
};
