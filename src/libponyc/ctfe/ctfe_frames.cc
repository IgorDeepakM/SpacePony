#include "ctfe_frames.h"


using namespace std;

CtfeFrame::~CtfeFrame()
{

}


bool CtfeFrame::new_value(const string& name, const CtfeValue& value)
{
  const auto [it, success] = m_vars.insert({name, value});
  return success;
}


bool CtfeFrame::update_value(const string& name, const CtfeValue& value)
{
  auto it = m_vars.find(name);
  if(it != m_vars.end())
  {
    it->second = value;
    return true;
  }

  return false;
}


bool CtfeFrame::get_value(const string& name, CtfeValue& value) const
{
  auto it = m_vars.find(name);
  if(it != m_vars.end())
  {
    value = it->second;
    return true;
  }

  return false;
}







CtfeFrames::CtfeFrames():
  m_current_frame{0}
{
  m_frame.push_back(CtfeFrame{});
}


void CtfeFrames::push_frame()
{
  m_frame.push_back(CtfeFrame{});
  m_current_frame++;
}


void CtfeFrames::pop_frame()
{
  m_frame.pop_back();
  m_current_frame--;
}


bool CtfeFrames::find_in_frames(std::string name, CtfeValue& value) const
{
  size_t i = m_current_frame;

  do
  {
    if(get_value(name, value))
    {
      return true;
    }
  }while(i-- != 0);

  return false;
}


bool CtfeFrames::new_value(const string& name, const CtfeValue& value)
{
  return m_frame[m_current_frame].new_value(name, value);
}


bool CtfeFrames::update_value(const string& name, const CtfeValue& value)
{
  size_t i = m_current_frame;

  do
  {
    if(m_frame[i].update_value(name, value))
    {
      return true;
    }
  }while(i-- != 0);

  return false;
}


bool CtfeFrames::get_value(const string& name, CtfeValue& value) const
{
  size_t i = m_current_frame;

  do
  {
    if(m_frame[i].get_value(name, value))
    {
      return true;
    }
  }while(i-- != 0);

  return false;
}
