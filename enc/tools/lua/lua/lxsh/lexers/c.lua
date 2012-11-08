--[[

 Lexer for C source code powered by LPeg.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 30, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'
local P = lpeg.P
local R = lpeg.R
local S = lpeg.S

-- Create a lexer definition context.
local context = lxsh.lexers.new 'c'

-- The following LPeg patterns are used as building blocks.
local U, L = R'AZ', R'az' -- uppercase, lowercase
local O, D = R'07', R'09' -- octal, decimal
local X = D + R'AF' + R'af' -- hexadecimal
local W = U + L -- case insensitive letter
local A = W + D + '_' -- identifier
local B = -A -- word boundary
local endline = S'\r\n\f' -- end of line character
local newline = '\r\n' + endline -- newline sequence
local escape = '\\' * ( newline -- escape sequence
                      + S'\\"\'?abfnrtv'
                      + (#O * O^-3)
                      + ('x' * #X * X^-2))

context:define('keyword', context:keywords [[
  auto break case char const continue default do double else enum extern float
  for goto if int long register return short signed sizeof static struct switch
  typedef union unsigned void volatile while
]])

-- Pattern definitions start here.
context:define('whitespace' , S'\r\n\f\t\v '^1)
context:define('identifier', (W + '_') * A^0)
context:define('preprocessor', '#' * (1 - S'\r\n\f\\' + '\\' * (newline + 1))^0 * newline^-1)

-- Character and string literals.
context:define('character', "'" * ((1 - S"\\\r\n\f'") + escape) * "'")
context:define('string', '"' * ((1 - S'\\\r\n\f"') + escape)^0 * '"')

-- Comments.
local slc = '//' * (1 - endline)^0 * newline^-1
local mlc = '/*' * (1 - P'*/')^0 * '*/'
context:define('comment', slc + mlc)

-- Numbers (matched before operators because .1 is a number).
local int = (('0' * ((S'xX' * X^1) + O^1)) + D^1) * S'lL'^-2
local flt = ((D^1 * '.' * D^0
            + D^0 * '.' * D^1
            + D^1 * 'e' * D^1) * S'fF'^-1)
            + D^1 * S'fF'
context:define('number', flt + int)

-- Operators (matched after comments because of conflict with slash/division).
context:define('operator', P'>>=' + '<<=' + '--' + '>>' + '>=' + '/=' + '==' + '<='
    + '+=' + '<<' + '*=' + '++' + '&&' + '|=' + '||' + '!=' + '&=' + '-='
    + '^=' + '%=' + '->' + S',)*%+&(-~/^]{}|.[>!?:=<;')

-- Define an `error' token kind that consumes one character and enables
-- the lexer to resume as a last resort for dealing with unknown input.
context:define('error', 1)

return context:compile()

-- vim: ts=2 sw=2 et
