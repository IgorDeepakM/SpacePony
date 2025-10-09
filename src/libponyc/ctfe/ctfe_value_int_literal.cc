#include "ctfe_value_int_literal.h"

#include "ctfe_value.h"
#include "../expr/literal.h"


CtfeValueIntLiteral::CtfeValueIntLiteral()
{
  zero();
}
  
CtfeValueIntLiteral::CtfeValueIntLiteral(const CtfeValueIntLiteral& b)
{
  set(b);
}

CtfeValueIntLiteral::CtfeValueIntLiteral(const lexint_t& b)
{
  m_val = b;
}

void CtfeValueIntLiteral::set(const CtfeValueIntLiteral& b)
{
  m_val = b.m_val;
}

void CtfeValueIntLiteral::zero()
{
  m_val = lexint_zero();
}

int CtfeValueIntLiteral::cmp(const CtfeValueIntLiteral& b) const
{
  return lexint_cmp(&m_val, &b.m_val);
}

int CtfeValueIntLiteral::cmp64(uint64_t b) const
{
  return lexint_cmp64(&m_val, b);
}

CtfeValueIntLiteral CtfeValueIntLiteral::shl(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_shl(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::shr(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_shr(&m_val, b));
}

uint64_t CtfeValueIntLiteral::testbit(uint8_t b) const
{
  return lexint_testbit(&m_val, b);
}

CtfeValueIntLiteral CtfeValueIntLiteral::setbit(uint8_t b) const
{
  return CtfeValueIntLiteral(lexint_setbit(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::add(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_add(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::add64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_add64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::sub(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_sub(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::sub64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_add64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::mul(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_mul(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::mul64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_mul64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::div(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_div(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::div64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_div64(&m_val, b));
}

void CtfeValueIntLiteral::from_char(int c)
{
  m_val = lexint_char(&m_val, c);
}

bool CtfeValueIntLiteral::accum(uint64_t digit, uint64_t base)
{
  return lexint_accum(&m_val, digit, base);
}

double CtfeValueIntLiteral::to_double() const
{
  return lexint_double(&m_val);
}

CtfeValueIntLiteral CtfeValueIntLiteral::op_and(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_and(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::and64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_and64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::op_or(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_or(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::or64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_or64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::op_xor(const CtfeValueIntLiteral& b) const
{
  return CtfeValueIntLiteral(lexint_xor(&m_val, &b.m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::xor64(uint64_t b) const
{
  return CtfeValueIntLiteral(lexint_xor64(&m_val, b));
}

CtfeValueIntLiteral CtfeValueIntLiteral::op_not() const
{
  return CtfeValueIntLiteral(lexint_not(&m_val));
}

CtfeValueIntLiteral CtfeValueIntLiteral::negate() const
{
  return CtfeValueIntLiteral(lexint_negate(&m_val));
}


CtfeValueBool CtfeValueIntLiteral::eq(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp == 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::ne(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp != 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::lt(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp < 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::le(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp <= 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::gt(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp > 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::ge(const CtfeValueIntLiteral& b) const
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp >= 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


ast_t* CtfeValueIntLiteral::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(TK_INT);
  ast_set_int(new_node, &m_val);
  make_literal_type(new_node);
  return new_node;
}


bool CtfeValueIntLiteral::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
{
  CtfeValueIntLiteral rec_val = args[0].get_int_literal();

  if(args.size() == 2)
  {
    CtfeValueIntLiteral first_arg = args[1].get_int_literal();

    if(method_name == "add")
    {
      CtfeValueIntLiteral r = rec_val.add(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "sub")
    {
      CtfeValueIntLiteral r = rec_val.sub(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "mul")
    {
      CtfeValueIntLiteral r = rec_val.mul(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "div")
    {
      CtfeValueIntLiteral r = rec_val.div(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_and")
    {
      CtfeValueIntLiteral r = rec_val.op_and(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_or")
    {
      CtfeValueIntLiteral r = rec_val.op_or(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_xor")
    {
      CtfeValueIntLiteral r = rec_val.op_xor(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "shl")
    {
      CtfeValueIntLiteral r = rec_val.shl(first_arg.m_val.low);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "shr")
    {
      CtfeValueIntLiteral r = rec_val.shr(first_arg.m_val.low);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "eq")
    {
      CtfeValueBool r = rec_val.eq(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ne")
    {
      CtfeValueBool r = rec_val.ne(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "lt")
    {
      CtfeValueBool r = rec_val.lt(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "le")
    {
      CtfeValueBool r = rec_val.le(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "gt")
    {
      CtfeValueBool r = rec_val.gt(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ge")
    {
      CtfeValueBool r = rec_val.ge(first_arg);
      result = CtfeValue(r);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "op_not")
    {
      CtfeValueIntLiteral r = rec_val.op_not();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "neg")
    {
      CtfeValueIntLiteral r = rec_val.negate();
      result = CtfeValue(r);
      return true;
    }
  }

  return false;
}