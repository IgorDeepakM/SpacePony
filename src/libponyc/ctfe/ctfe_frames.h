#pragma once

#include <vector>
#include <string>
#include <map>

#include "../ast/ast.h"

#include "ctfe_value.h"


class CtfeFrame
{
  std::map<std::string, CtfeValue> m_vars;

public:
  ~CtfeFrame();
  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value, CtfeValue& old_value);
  bool get_value(const std::string& name, CtfeValue& value) const;
};

class CtfeFrames
{
  std::vector<CtfeFrame> m_frame;
  size_t m_current_frame;

  bool find_in_frames(std::string name, CtfeValue& value) const;

public:
  CtfeFrames();
  
  void push_frame();
  void pop_frame();

  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value, CtfeValue& old_value);
  bool get_value(const std::string& name, CtfeValue& value) const;
};