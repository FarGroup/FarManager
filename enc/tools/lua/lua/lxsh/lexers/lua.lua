--[[

 Lexer for Lua 5.1 source code powered by LPeg.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 30, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'
local P = lpeg.P
local R = lpeg.R
local S = lpeg.S
local D = R'09'
local I = R('AZ', 'az', '\127\255') + '_'
local B = -(I + D) -- word boundary

-- Create a lexer definition context.
local context = lxsh.lexers.new 'lua'

-- Pattern definitions start here.
context:define('whitespace', S'\r\n\f\t\v '^1)
context:define('constant', (P'true' + 'false' + 'nil') * B)

-- Interactive prompt.
context:define('prompt', function(input, index)
  if index == 1 then
    local copyright = '^Lua%s+%S+%s+Copyright[^\r\n]+'
    local _, last = input:find(copyright, index)
    if last then
      return last + 1
    end
  else
    local _, last = input:find('^[\r\n]>>?', index-1)
    if last then
      return last + 1
    end
  end
end)

-- Pattern for long strings and long comments.
local longstring = #(P'[[' + (P'[' * P'='^0 * '[')) * P(function(input, index)
  local level = input:match('^%[(=*)%[', index)
  if level then
    local _, last = input:find(']' .. level .. ']', index, true)
    if last then return last + 1 end
  end
end)

-- String literals.
local singlequoted = P"'" * ((1 - S"'\r\n\f\\") + (P'\\' * 1))^0 * "'"
local doublequoted = P'"' * ((1 - S'"\r\n\f\\') + (P'\\' * 1))^0 * '"'
context:define('string', singlequoted + doublequoted + longstring)

-- Comments.
local eol = P'\r\n' + '\n'
local line = (1 - S'\r\n\f')^0 * eol^-1
local soi = P(function(_, i) return i == 1 and i end)
local shebang = soi * '#!' * line
local singleline = P'--' * line
local multiline = P'--' * longstring
context:define('comment', multiline + singleline + shebang)

-- Numbers.
local sign = S'+-'^-1
local decimal = D^1
local hexadecimal = P'0' * S'xX' * R('09', 'AF', 'af') ^ 1
local float = D^1 * P'.' * D^0 + P'.' * D^1
local maybeexp = (float + decimal) * (S'eE' * sign * D^1)^-1
context:define('number', hexadecimal + maybeexp)

-- Operators (matched after comments because of conflict with minus).
context:define('operator', P'not' + '...' + 'and' + '..' + '~=' + '==' + '>=' + '<='
  + 'or' + S']{=>^[<;)*(%}+-:,/.#')

-- Keywords.
context:define('keyword', context:keywords [[
  break do else elseif end for function if in local repeat return then until while
]])

-- Identifiers - Sometimes it's very convenient to match for example "io.write"
-- as one token instead of three, however this is not really a lexer's job. As
-- a compromise we'll let the caller choose by passing a table of options with
-- the key "join_identifiers" and the value "true":
local ident = I * (I + D)^0
local expr = ('.' * ident)^0
context:define('identifier', lpeg.Cmt(ident * lpeg.Carg(1),
  function(input, index, options)
    if options and options.join_identifiers then
      return expr:match(input, index)
    else
      return index
    end
  end))

-- Define an `error' token kind that consumes one character and enables
-- the lexer to resume as a last resort for dealing with unknown input.
context:define('error', 1)

-- Compile the final LPeg pattern to match any single token and return the
-- table containing the various definitions that make up the Lua lexer.
return context:compile()

-- vim: ts=2 sw=2 et
