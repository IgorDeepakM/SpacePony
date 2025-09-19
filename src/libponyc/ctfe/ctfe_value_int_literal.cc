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
  lexint_zero(&m_val);
}

int CtfeValueIntLiteral::cmp(const CtfeValueIntLiteral& b) const
{
  return lexint_cmp(&m_val, &b.m_val);
}

int CtfeValueIntLiteral::cmp64(uint64_t b)
{
  return lexint_cmp64(&m_val, b);
}

CtfeValueIntLiteral CtfeValueIntLiteral::shl(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_shl(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::shr(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_shr(&dst.m_val, &m_val, b);
  return dst;
}

uint64_t CtfeValueIntLiteral::testbit(uint8_t b)
{
  return lexint_testbit(&m_val, b);
}

CtfeValueIntLiteral CtfeValueIntLiteral::setbit(uint8_t b)
{
  CtfeValueIntLiteral dst;
  lexint_setbit(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::add(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_add(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::add64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_add64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::sub(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_sub(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::sub64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_add64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::mul(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_mul(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::mul64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_mul64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::div(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_div(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::div64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_div64(&dst.m_val, &m_val, b);
  return dst;
}

void CtfeValueIntLiteral::from_char(int c)
{
  lexint_char(&m_val, c);
}

bool CtfeValueIntLiteral::accum(uint64_t digit, uint64_t base)
{
  return lexint_accum(&m_val, digit, base);
}

double CtfeValueIntLiteral::to_double()
{
  return lexint_double(&m_val);
}

CtfeValueIntLiteral CtfeValueIntLiteral::and(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_and(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::and64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_and64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::or(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_or(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::or64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_or64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::xor(CtfeValueIntLiteral& b)
{
  CtfeValueIntLiteral dst;
  lexint_xor(&dst.m_val, &m_val, &b.m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::xor64(uint64_t b)
{
  CtfeValueIntLiteral dst;
  lexint_xor64(&dst.m_val, &m_val, b);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::not()
{
  CtfeValueIntLiteral dst;
  lexint_not(&dst.m_val, &m_val);
  return dst;
}

CtfeValueIntLiteral CtfeValueIntLiteral::negate()
{
  CtfeValueIntLiteral dst;
  lexint_negate(&dst.m_val, &m_val);
  return dst;
}


CtfeValueBool CtfeValueIntLiteral::eq(CtfeValueIntLiteral& b)
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp == 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::ne(CtfeValueIntLiteral& b)
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp != 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::lt(CtfeValueIntLiteral& b)
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp < 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::le(CtfeValueIntLiteral& b)
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp <= 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::gt(CtfeValueIntLiteral& b)
{
  int cmp = lexint_cmp(&m_val, &b.m_val);
  if(cmp > 0)
  {
    return CtfeValueBool(true);
  }

  return CtfeValueBool(false);
}


CtfeValueBool CtfeValueIntLiteral::ge(CtfeValueIntLiteral& b)
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
  std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
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
      CtfeValueIntLiteral r = rec_val.and(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_or")
    {
      CtfeValueIntLiteral r = rec_val.or(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_xor")
    {
      CtfeValueIntLiteral r = rec_val.xor(first_arg);
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
      CtfeValueIntLiteral r = rec_val.not();
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