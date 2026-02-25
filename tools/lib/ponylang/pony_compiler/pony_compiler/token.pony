use @token_new[NullablePointer[_Token]](id: TokenId)
use @token_free[None](token: _Token)
use @token_dup[_Token](token: _Token)
use @token_dup_new_id[_Token](token: _Token, id: TokenId)
use @token_get_id[TokenId](token: _Token)
use @token_string[Pointer[U8] ref](token: _Token)
use @token_string_len[USize](token: _Token)
use @token_float[F64](token: _Token)
use @token_int[NullablePointer[_LexIntT]](token: _Token)
// return a string for ptinting the given token.
// The returned string must not be deleted and is valid for the lifetime of the
// token
use @token_source[NullablePointer[_Source]](token: _Token)
use @token_print[Pointer[U8] ref](token: _Token)
use @token_id_desc[Pointer[U8] ref](token: _Token)
use @token_line_number[USize](token: _Token)
use @token_line_position[USize](token: _Token)

struct _Token
  var id: TokenId = TokenIds.tk_eof()
  var frozen: Bool = false
  var source: NullablePointer[_Source] = source.none()
  var line: USize = 0
  var pos: USize = 0
  var printed: Pointer[U8] ref = printed.create()
  var opaque_value_1: U64 = 0
  var opaque_value_2: U64 = 0
    """
    Probably wrong derive from, just to get the right struct layout for reaching the frozen field
    ```c
    union
    {
      struct
      {
        const char* string;
        size_t str_length;
      };
      double real;
      lexint_t integer;
    }
    ```
    """

type TokenId is I32

primitive TokenIds
  """
  Numeric values based upon the order of elements in the
  `token_id` enum in libponyc/ast/token.h
  """
  enum I32
    tk_eof
    tk_lex_error
    tk_none

    // Literals
    tk_true
    tk_false
    tk_string
    tk_int
    tk_float
    tk_id

    // Symbols
    tk_lbrace
    tk_rbrace
    tk_lparen
    tk_rparen
    tk_lsquare
    tk_rsquare
    tk_backslash

    tk_comma
    tk_arrow
    tk_dblarrow
    tk_dot
    tk_tilde
    tk_chain
    tk_colon
    tk_semi
    tk_assign

    tk_plus
    tk_plus_tilde
    tk_minus
    tk_minus_tilde
    tk_multiply
    tk_multiply_tilde
    tk_divide
    tk_divide_tilde
    tk_rem
    tk_rem_tilde
    tk_mod
    tk_mod_tilde
    tk_at
    tk_at_lbrace

    tk_lshift
    tk_lshift_tilde
    tk_rshift
    tk_rshift_tilde

    tk_lt
    tk_lt_tilde
    tk_le
    tk_le_tilde
    tk_ge
    tk_ge_tilde
    tk_gt
    tk_gt_tilde

    tk_eq
    tk_eq_tilde
    tk_ne
    tk_ne_tilde

    tk_pipe
    tk_isecttype
    tk_ephemeral
    tk_aliased
    tk_subtype

    tk_question
    tk_unary_minus
    tk_unary_minus_tilde
    tk_ellipsis
    tk_constant
    tk_constant_object
    tk_constant_array

    // Newline symbols only used by lexer and parser
    tk_lparen_new
    tk_lsquare_new
    tk_minus_new
    tk_minus_tilde_new

    // Keywords
    tk_compile_intrinsic

    tk_use
    tk_type
    tk_interface
    tk_trait
    tk_primitive
    tk_struct
    tk_class
    tk_actor
    tk_object
    tk_lambda
    tk_barelambda
    tk_enum

    tk_as
    tk_is
    tk_isnt

    tk_var
    tk_let
    tk_embed
    tk_dontcare
    tk_new
    tk_fun
    tk_be

    tk_iso
    tk_trn
    tk_ref
    tk_val
    tk_box
    tk_tag
    tk_nhb

    tk_cap_read
    tk_cap_send
    tk_cap_share
    tk_cap_alias
    tk_cap_any

    tk_this
    tk_return
    tk_break
    tk_continue
    tk_consume
    tk_recover
    tk_comptime

    tk_if
    tk_ifdef
    tk_iftype
    tk_iftype_set
    tk_entityif
    tk_entityif_set
    tk_then
    tk_else
    tk_elseif
    tk_end
    tk_while
    tk_do
    tk_repeat
    tk_until
    tk_for
    tk_in
    tk_match
    tk_where
    tk_try
    tk_try_no_check
    tk_with
    tk_error
    tk_compile_error
    tk_asm

    tk_not
    tk_and
    tk_or
    tk_xor

    tk_digestof
    tk_address
    tk_offsetof
    tk_sizeof
    tk_location

    // Abstract tokens which don't directly appear in the source
    tk_program
    tk_package
    tk_module

    tk_members
    tk_fvar
    tk_flet
    tk_ffidecl
    tk_fficall
    tk_ffiref

    tk_ifdefand
    tk_ifdefor
    tk_ifdefnot
    tk_ifdefflag

    tk_provides
    tk_uniontype
    tk_tupletype
    tk_nominal
    tk_thistype
    tk_funtype
    tk_lambdatype
    tk_barelambdatype
    tk_dontcaretype
    tk_infertype
    tk_errortype

    tk_literal
    tk_literalbranch
    tk_operatorliteral

    tk_typeparams
    tk_typeparam
    tk_params
    tk_param
    tk_typeargs
    tk_valueformalparam
    tk_valueformalarg
    tk_positionalargs
    tk_namedargs
    tk_namedarg
    tk_updatearg
    tk_lambdacaptures
    tk_lambdacapture

    tk_seq
    tk_qualify
    tk_call
    tk_tuple
    tk_array
    tk_cases
    tk_case
    tk_match_capture
    tk_match_dontcare

    tk_reference
    tk_packageref
    tk_typeref
    tk_typeparamref
    tk_valueformalparamref
    tk_newref
    tk_newberef
    tk_beref
    tk_funref
    tk_fvarref
    tk_fletref
    tk_embedref
    tk_tupleelemref
    tk_varref
    tk_letref
    tk_paramref
    tk_dontcareref
    tk_newapp
    tk_beapp
    tk_funapp
    tk_bechain
    tk_funchain

    tk_entity_type_class
    tk_entity_type_struct
    tk_entity_type_primitive
    tk_entity_type_actor

    tk_annotation

    tk_disposing_block

    // Pseudo tokens that never actually exist
    tk_newline  // Used by parser macros
    tk_flatten   // Used by parser macros for tree building

    // Token types for testing
    tk_test_no_seq
    tk_test_seq_scope
    tk_test_try_no_check
    tk_test_aliased
    tk_test_updatearg
    tk_test_extra
  end

  fun string(token_id: TokenId): String val =>
    match token_id
    | TokenIds.tk_eof => "TK_EOF"
    | TokenIds.tk_lex_error => "TK_LEX_ERROR"
    | TokenIds.tk_none => "TK_NONE"
    | TokenIds.tk_true => "TK_TRUE"
    | TokenIds.tk_false => "TK_FALSE"
    | TokenIds.tk_string => "TK_STRING"
    | TokenIds.tk_int => "TK_INT"
    | TokenIds.tk_float => "TK_FLOAT"
    | TokenIds.tk_id => "TK_ID"
    | TokenIds.tk_lbrace => "TK_LBRACE"
    | TokenIds.tk_rbrace => "TK_RBRACE"
    | TokenIds.tk_lparen => "TK_LPAREN"
    | TokenIds.tk_rparen => "TK_RPAREN"
    | TokenIds.tk_lsquare => "TK_LSQUARE"
    | TokenIds.tk_rsquare => "TK_RSQUARE"
    | TokenIds.tk_backslash => "TK_BACKSLASH"
    | TokenIds.tk_comma => "TK_COMMA"
    | TokenIds.tk_arrow => "TK_ARROW"
    | TokenIds.tk_dblarrow => "TK_DBLARROW"
    | TokenIds.tk_dot => "TK_DOT"
    | TokenIds.tk_tilde => "TK_TILDE"
    | TokenIds.tk_chain => "TK_CHAIN"
    | TokenIds.tk_colon => "TK_COLON"
    | TokenIds.tk_semi => "TK_SEMI"
    | TokenIds.tk_assign => "TK_ASSIGN"
    | TokenIds.tk_plus => "TK_PLUS"
    | TokenIds.tk_plus_tilde => "TK_PLUS_TILDE"
    | TokenIds.tk_minus => "TK_MINUS"
    | TokenIds.tk_minus_tilde => "TK_MINUS_TILDE"
    | TokenIds.tk_multiply => "TK_MULTIPLY"
    | TokenIds.tk_multiply_tilde => "TK_MULTIPLY_TILDE"
    | TokenIds.tk_divide => "TK_DIVIDE"
    | TokenIds.tk_divide_tilde => "TK_DIVIDE_TILDE"
    | TokenIds.tk_rem => "TK_REM"
    | TokenIds.tk_rem_tilde => "TK_REM_TILDE"
    | TokenIds.tk_mod => "TK_MOD"
    | TokenIds.tk_mod_tilde => "TK_MOD_TILDE"
    | TokenIds.tk_at => "TK_AT"
    | TokenIds.tk_at_lbrace => "TK_AT_LBRACE"
    | TokenIds.tk_lshift => "TK_LSHIFT"
    | TokenIds.tk_lshift_tilde => "TK_LSHIFT_TILDE"
    | TokenIds.tk_rshift => "TK_RSHIFT"
    | TokenIds.tk_rshift_tilde => "TK_RSHIFT_TILDE"
    | TokenIds.tk_lt => "TK_LT"
    | TokenIds.tk_lt_tilde => "TK_LT_TILDE"
    | TokenIds.tk_le => "TK_LE"
    | TokenIds.tk_le_tilde => "TK_LE_TILDE"
    | TokenIds.tk_ge => "TK_GE"
    | TokenIds.tk_ge_tilde => "TK_GE_TILDE"
    | TokenIds.tk_gt => "TK_GT"
    | TokenIds.tk_gt_tilde => "TK_GT_TILDE"
    | TokenIds.tk_eq => "TK_EQ"
    | TokenIds.tk_eq_tilde => "TK_EQ_TILDE"
    | TokenIds.tk_ne => "TK_NE"
    | TokenIds.tk_ne_tilde => "TK_NE_TILDE"
    | TokenIds.tk_pipe => "TK_PIPE"
    | TokenIds.tk_isecttype => "TK_ISECTTYPE"
    | TokenIds.tk_ephemeral => "TK_EPHEMERAL"
    | TokenIds.tk_aliased => "TK_ALIASED"
    | TokenIds.tk_subtype => "TK_SUBTYPE"
    | TokenIds.tk_question => "TK_QUESTION"
    | TokenIds.tk_unary_minus => "TK_UNARY_MINUS"
    | TokenIds.tk_unary_minus_tilde => "TK_UNARY_MINUS_TILDE"
    | TokenIds.tk_ellipsis => "TK_ELLIPSIS"
    | TokenIds.tk_constant => "TK_CONSTANT"
    | TokenIds.tk_constant_object => "TK_CONSTANT_OBJECT"
    | TokenIds.tk_constant_array => "TK_CONSTANT_ARRAY"
    | TokenIds.tk_lparen_new => "TK_LPAREN_NEW"
    | TokenIds.tk_lsquare_new => "TK_LSQUARE_NEW"
    | TokenIds.tk_minus_new => "TK_MINUS_NEW"
    | TokenIds.tk_minus_tilde_new => "TK_MINUS_TILDE_NEW"
    | TokenIds.tk_compile_intrinsic => "TK_COMPILE_INTRINSIC"
    | TokenIds.tk_use => "TK_USE"
    | TokenIds.tk_type => "TK_TYPE"
    | TokenIds.tk_interface => "TK_INTERFACE"
    | TokenIds.tk_trait => "TK_TRAIT"
    | TokenIds.tk_primitive => "TK_PRIMITIVE"
    | TokenIds.tk_struct => "TK_STRUCT"
    | TokenIds.tk_class => "TK_CLASS"
    | TokenIds.tk_actor => "TK_ACTOR"
    | TokenIds.tk_object => "TK_OBJECT"
    | TokenIds.tk_lambda => "TK_LAMBDA"
    | TokenIds.tk_barelambda => "TK_BARELAMBDA"
    | TokenIds.tk_as => "TK_AS"
    | TokenIds.tk_is => "TK_IS"
    | TokenIds.tk_isnt => "TK_ISNT"
    | TokenIds.tk_var => "TK_VAR"
    | TokenIds.tk_let => "TK_LET"
    | TokenIds.tk_embed => "TK_EMBED"
    | TokenIds.tk_dontcare => "TK_DONTCARE"
    | TokenIds.tk_new => "TK_NEW"
    | TokenIds.tk_fun => "TK_FUN"
    | TokenIds.tk_be => "TK_BE"
    | TokenIds.tk_iso => "TK_ISO"
    | TokenIds.tk_trn => "TK_TRN"
    | TokenIds.tk_ref => "TK_REF"
    | TokenIds.tk_val => "TK_VAL"
    | TokenIds.tk_box => "TK_BOX"
    | TokenIds.tk_tag => "TK_TAG"
    | TokenIds.tk_nhb => "TK_NHB"
    | TokenIds.tk_cap_read => "TK_CAP_READ"
    | TokenIds.tk_cap_send => "TK_CAP_SEND"
    | TokenIds.tk_cap_share => "TK_CAP_SHARE"
    | TokenIds.tk_cap_alias => "TK_CAP_ALIAS"
    | TokenIds.tk_cap_any => "TK_CAP_ANY"
    | TokenIds.tk_this => "TK_THIS"
    | TokenIds.tk_return => "TK_RETURN"
    | TokenIds.tk_break => "TK_BREAK"
    | TokenIds.tk_continue => "TK_CONTINUE"
    | TokenIds.tk_consume => "TK_CONSUME"
    | TokenIds.tk_recover => "TK_RECOVER"
    | TokenIds.tk_comptime => "TK_COMPTIME"
    | TokenIds.tk_if => "TK_IF"
    | TokenIds.tk_ifdef => "TK_IFDEF"
    | TokenIds.tk_iftype => "TK_IFTYPE"
    | TokenIds.tk_iftype_set => "TK_IFTYPE_SET"
    | TokenIds.tk_entityif => "TK_ENTITYIF"
    | TokenIds.tk_entityif_set => "TK_ENTITYIF_SET"
    | TokenIds.tk_then => "TK_THEN"
    | TokenIds.tk_else => "TK_ELSE"
    | TokenIds.tk_elseif => "TK_ELSEIF"
    | TokenIds.tk_end => "TK_END"
    | TokenIds.tk_while => "TK_WHILE"
    | TokenIds.tk_do => "TK_DO"
    | TokenIds.tk_repeat => "TK_REPEAT"
    | TokenIds.tk_until => "TK_UNTIL"
    | TokenIds.tk_for => "TK_FOR"
    | TokenIds.tk_in => "TK_IN"
    | TokenIds.tk_match => "TK_MATCH"
    | TokenIds.tk_where => "TK_WHERE"
    | TokenIds.tk_try => "TK_TRY"
    | TokenIds.tk_try_no_check => "TK_TRY_NO_CHECK"
    | TokenIds.tk_with => "TK_WITH"
    | TokenIds.tk_error => "TK_ERROR"
    | TokenIds.tk_compile_error => "TK_COMPILE_ERROR"
    | TokenIds.tk_not => "TK_NOT"
    | TokenIds.tk_and => "TK_AND"
    | TokenIds.tk_or => "TK_OR"
    | TokenIds.tk_xor => "TK_XOR"
    | TokenIds.tk_digestof => "TK_DIGESTOF"
    | TokenIds.tk_address => "TK_ADDRESS"
    | TokenIds.tk_offsetof => "TK_OFFSETOF"
    | TokenIds.tk_sizeof => "TK_SIZEOF"
    | TokenIds.tk_location => "TK_LOCATION"
    | TokenIds.tk_program => "TK_PROGRAM"
    | TokenIds.tk_package => "TK_PACKAGE"
    | TokenIds.tk_module => "TK_MODULE"
    | TokenIds.tk_members => "TK_MEMBERS"
    | TokenIds.tk_fvar => "TK_FVAR"
    | TokenIds.tk_flet => "TK_FLET"
    | TokenIds.tk_ffidecl => "TK_FFIDECL"
    | TokenIds.tk_fficall => "TK_FFICALL"
    | TokenIds.tk_ifdefand => "TK_IFDEFAND"
    | TokenIds.tk_ifdefor => "TK_IFDEFOR"
    | TokenIds.tk_ifdefnot => "TK_IFDEFNOT"
    | TokenIds.tk_ifdefflag => "TK_IFDEFFLAG"
    | TokenIds.tk_provides => "TK_PROVIDES"
    | TokenIds.tk_uniontype => "TK_UNIONTYPE"
    | TokenIds.tk_tupletype => "TK_TUPLETYPE"
    | TokenIds.tk_nominal => "TK_NOMINAL"
    | TokenIds.tk_thistype => "TK_THISTYPE"
    | TokenIds.tk_funtype => "TK_FUNTYPE"
    | TokenIds.tk_lambdatype => "TK_LAMBDATYPE"
    | TokenIds.tk_barelambdatype => "TK_BARELAMBDATYPE"
    | TokenIds.tk_dontcaretype => "TK_DONTCARETYPE"
    | TokenIds.tk_infertype => "TK_INFERTYPE"
    | TokenIds.tk_errortype => "TK_ERRORTYPE"
    | TokenIds.tk_literal => "TK_LITERAL"
    | TokenIds.tk_literalbranch => "TK_LITERALBRANCH"
    | TokenIds.tk_operatorliteral => "TK_OPERATORLITERAL"
    | TokenIds.tk_typeparams => "TK_TYPEPARAMS"
    | TokenIds.tk_typeparam => "TK_TYPEPARAM"
    | TokenIds.tk_params => "TK_PARAMS"
    | TokenIds.tk_param => "TK_PARAM"
    | TokenIds.tk_typeargs => "TK_TYPEARGS"
    | TokenIds.tk_valueformalparam => "TK_VALUEFORMALPARAM"
    | TokenIds.tk_valueformalarg => "TK_VALUEFORMALARG"
    | TokenIds.tk_positionalargs => "TK_POSITIONALARGS"
    | TokenIds.tk_namedargs => "TK_NAMEDARGS"
    | TokenIds.tk_namedarg => "TK_NAMEDARG"
    | TokenIds.tk_updatearg => "TK_UPDATEARG"
    | TokenIds.tk_lambdacaptures => "TK_LAMBDACAPTURES"
    | TokenIds.tk_lambdacapture => "TK_LAMBDACAPTURE"
    | TokenIds.tk_seq => "TK_SEQ"
    | TokenIds.tk_qualify => "TK_QUALIFY"
    | TokenIds.tk_call => "TK_CALL"
    | TokenIds.tk_tuple => "TK_TUPLE"
    | TokenIds.tk_array => "TK_ARRAY"
    | TokenIds.tk_cases => "TK_CASES"
    | TokenIds.tk_case => "TK_CASE"
    | TokenIds.tk_match_capture => "TK_MATCH_CAPTURE"
    | TokenIds.tk_match_dontcare => "TK_MATCH_DONTCARE"
    | TokenIds.tk_reference => "TK_REFERENCE"
    | TokenIds.tk_packageref => "TK_PACKAGEREF"
    | TokenIds.tk_typeref => "TK_TYPEREF"
    | TokenIds.tk_typeparamref => "TK_TYPEPARAMREF"
    | TokenIds.tk_valueformalparamref => "TK_VALUEFORMALPARAMREF"
    | TokenIds.tk_newref => "TK_NEWREF"
    | TokenIds.tk_newberef => "TK_NEWBEREF"
    | TokenIds.tk_beref => "TK_BEREF"
    | TokenIds.tk_funref => "TK_FUNREF"
    | TokenIds.tk_fvarref => "TK_FVARREF"
    | TokenIds.tk_fletref => "TK_FLETREF"
    | TokenIds.tk_embedref => "TK_EMBEDREF"
    | TokenIds.tk_tupleelemref => "TK_TUPLEELEMREF"
    | TokenIds.tk_varref => "TK_VARREF"
    | TokenIds.tk_letref => "TK_LETREF"
    | TokenIds.tk_paramref => "TK_PARAMREF"
    | TokenIds.tk_dontcareref => "TK_DONTCAREREF"
    | TokenIds.tk_newapp => "TK_NEWAPP"
    | TokenIds.tk_beapp => "TK_BEAPP"
    | TokenIds.tk_funapp => "TK_FUNAPP"
    | TokenIds.tk_bechain => "TK_BECHAIN"
    | TokenIds.tk_funchain => "TK_FUNCHAIN"
    | TokenIds.tk_entity_type_class => "TK_ENTITY_TYPE_CLASS"
    | TokenIds.tk_entity_type_struct => "TK_ENTITY_TYPE_STRUCT"
    | TokenIds.tk_entity_type_primitive => "TK_ENTITY_TYPE_PRIMITIVE"
    | TokenIds.tk_entity_type_actor => "TK_ENTITY_TYPE_ACTOR"
    | TokenIds.tk_annotation => "TK_ANNOTATION"
    | TokenIds.tk_disposing_block => "TK_DISPOSING_BLOCK"
    | TokenIds.tk_newline => "TK_NEWLINE"  // Used by parser macros
    | TokenIds.tk_flatten => "TK_FLATTEN"  // Used by parser macros for tree building
    | TokenIds.tk_test_no_seq => "TK_TEST_NO_SEQ"
    | TokenIds.tk_test_seq_scope => "TK_TEST_SEQ_SCOPE"
    | TokenIds.tk_test_try_no_check => "TK_TEST_TRY_NO_CHECK"
    | TokenIds.tk_test_aliased => "TK_TEST_ALIASED"
    | TokenIds.tk_test_updatearg => "TK_TEST_UPDATEARG"
    | TokenIds.tk_test_extra => "TK_TEST_EXTRA"
    else
      "TK_UNKNOWN"
    end

  fun is_entity(token_id: I32): Bool =>
    """
    Something like a class, actor, primitive, trait etc.

    Lambdas are not included. Maybe they should.
    """
    (token_id == this.tk_class())
      or (token_id == this.tk_actor())
      or (token_id == this.tk_primitive())
      or (token_id == this.tk_struct())
      or (token_id == this.tk_interface())
      or (token_id == this.tk_trait())
      or (token_id == this.tk_object())
