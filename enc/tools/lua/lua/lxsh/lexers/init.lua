--[[

 Infrastructure to make it easier to define lexers using LPeg.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 30, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'

-- Primitive LPeg patterns.
local D = lpeg.R'09'
local I = lpeg.R('AZ', 'az', '\127\255') + '_'
local SOS = lpeg.P(function(s, i) return i == 1 end) -- start of string
local EOS = -lpeg.P(1) -- end of string
local B = -(I + D) -- word boundary

-- Transform a string with keywords into an LPeg pattern that matches a keyword
-- followed by a word boundary. This automatically takes care of sorting from
-- longest to smallest.
local function compile_keywords(context, keywords)
  local list = {}
  for word in keywords:gmatch '%S+' do
    list[#list + 1] = word
  end
  table.sort(list, function(a, b)
    return #a > #b
  end)
  local pattern
  for _, word in ipairs(list) do
    local p = lpeg.P(word)
    pattern = pattern and (pattern + p) or p
  end
  local B = context.word_boundary or B
  local BB = lpeg.B(B, 1) + SOS -- starting boundary
  local AB = B + EOS -- ending boundary
  return BB * pattern * AB
end

-- Closure to define token type given name and LPeg pattern.
local function define_token(context, name, patt)
  local existing = context.patterns[name]
  context.patterns[#context.patterns + 1] = name
  patt = lpeg.P(patt)
  context.patterns[name] = existing and (existing + patt) or patt
end

-- Closure to compile all patterns into one pattern that captures (kind, text) pair.
local function compile_lexer(context)
  local function id(n)
    return lpeg.Cc(n) * context.patterns[n] * lpeg.Cp()
  end
  local any = id(context.patterns[1])
  for i = 2, #context.patterns do
    any = any + id(context.patterns[i])
  end
  context.any = any
  return context.lexer
end

-- Constructor for lexers defined using LPeg.
function lxsh.lexers.new(language)

  -- Table of LPeg patterns to match all kinds of tokens.
  local patterns = {}

  -- Public interface of lexer being constructed.
  local lexer = {
    language = language,
    patterns = patterns
  }

  -- Context in which lexer is being defined (private).
  local context = {
    lexer = lexer,
    patterns = patterns,
    keywords = compile_keywords,
    define = define_token,
    compile = compile_lexer,
  }

  -- The basic function for lexical analysis, it takes a subject string and
  -- optional index and returns a token type and the last index of the match.
  function lexer.find(subject, init, options)
    local kind, after = context.any:match(subject, init, options)
    if kind and after then return kind, after - 1 end
  end

  -- Convenience function that returns token type and matched text.
  function lexer.match(subject, init, options)
    local kind, after = context.any:match(subject, init, options)
    if kind and after then
      return kind, subject:sub(init, after - 1)
    end
  end

  -- Return an iterator that produces (kind, text) on each iteration.
  function lexer.gmatch(subject, options)
    local index, lnum, cnum = 1, 1, 1
    return function()
      local kind, after = context.any:match(subject, index, options)
      if kind and after then
        local text = subject:sub(index, after - 1)
        local oldlnum, oldcnum = lnum, cnum
        index = after
        lnum, cnum = lxsh.sync(text, lnum, cnum)
        return kind, text, oldlnum, oldcnum
      end
    end
  end

  -- Return the two closures used to construct the lexer.
  return context

end

return lxsh.lexers
