--[[

 Infrastructure to make it easier to define syntax highlighters.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 29, 2011
 URL: http://peterodding.com/code/lua/lxsh/

 The syntax highlighters in the LXSH module decorate the token streams produced
 by the lexers with the following additional tokens:

  - TODO, FIXME and XXX markers in comments
  - e-mail addresses and hyper links in strings and comments
  - escape sequences in character and string literals

 Coroutines are used to simplify the implementation of the decorated token
 stream and while it works I'm not happy with the code. Note also that the
 token stream is flat which means the following Lua source code:

   -- TODO Nested tokens?

 Produces the following HTML source code (reformatted for readability):

   <span class="comment">--</span>
   <span class="marker">TODO</span>
   <span class="comment">Nested tokens?</span>

 Instead of what you may have expected:

   <span class="comment">--
   <span class="marker">TODO</span>
   Nested tokens?</span>

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'

-- LPeg patterns to decorate the token stream (richer highlighting). {{{1

-- LPeg patterns to scan for comment markers.
local comment_marker = lpeg.P'TODO' + 'FIXME' + 'XXX'
local comment_scanner = lpeg.Cc'marker' * lpeg.C(comment_marker)
                      + lpeg.Carg(1) * lpeg.C((1 - comment_marker)^1)

-- LPeg patterns to match e-mail addresses.
local alnum = lpeg.R('AZ', 'az', '09')
local domainpart = alnum^1 * (lpeg.S'_-' * alnum^1)^0
local domain = domainpart * ('.' * domainpart)^1
local email = alnum^1 * (lpeg.S'_-.+' * alnum^1)^0 * '@' * domain

-- LPeg patterns to match URLs.
local protocol = ((lpeg.P'https' + 'http' + 'ftp' + 'irc') * '://') + 'mailto:'
local remainder = ((1-lpeg.S'\r\n\f\t\v ,."}])') + (lpeg.S',."}])' * (1-lpeg.S'\r\n\f\t\v ')))^0
local url = protocol * remainder

-- LPeg pattern to scan for e-mail addresses and URLs.
local other = (1 - (email + url))^1
local url_scanner = lpeg.C(email) / function(email) return 'email', email, email end
                  + lpeg.C(url) / function(url) return 'url', url, url end
                  + lpeg.Carg(1) * lpeg.C(other)

-- Constructor for syntax highlighting modes. {{{1

-- Construct a new syntax highlighter from the given parameters.
function lxsh.highlighters.new(context)

  -- Implementation of decorated token stream (depends on lexer as upvalue). {{{2

  -- LPeg pattern to scan for escape sequences in character and string literals.
  local escape_scanner = context.escape_sequence and
       (lpeg.Cc'escape' * lpeg.C(context.escape_sequence)
      + lpeg.Carg(1) * lpeg.C((1 - context.escape_sequence)^1))

  -- Turn an LPeg pattern into an iterator that produces (kind, text) pairs.
  -- TODO Find a better name for this.
  local function iterator(kind, text, pattern)
    local index = 1
    while index <= #text do
      local subkind, subtext, url = pattern:match(text, index, kind)
      if subkind and subtext then
        coroutine.yield(subkind, subtext, url)
        index = index + #subtext
      end
    end
  end

  -- Transform a function that produces values using yield() into an iterator.
  -- TODO Find a better name for this.
  local function producer(fun, a1, a2, a3)
    -- Lua refuses to pass ... as an upvalue but we
    -- only need three arguments so we fake it :-)
     return coroutine.wrap(function() fun(a1, a2, a3) end)
  end

  -- Decorate the token stream produced by a lexer so that comment markers,
  -- URLs, e-mail addresses and escape sequences are recognized as well.
  local function decorator(subject)
    local docs = context.docs
    local has_escapes = context.has_escapes
    for kind, text in context.lexer.gmatch(subject, { join_identifiers = true }) do
      -- Check to see if this token has documentation.
      local url = docs and docs[text]
      if url then
        coroutine.yield('library', text, url)
      elseif kind == 'comment' or kind == 'constant' or kind == 'string' then
        -- Identify e-mail addresses and URLs.
        for kind, text, url in producer(iterator, kind, text, url_scanner) do
          if kind == 'comment' then
            -- Identify comment markers.
            iterator(kind, text, comment_scanner)
          elseif has_escapes and has_escapes(kind, text) then
            -- Identify escape sequences.
            iterator(kind, text, escape_scanner)
          else
            coroutine.yield(kind, text, url)
          end
        end
      else
        coroutine.yield(kind, text)
      end
    end
  end

  -- Highlighter function (depends on context and decorator as upvalues). {{{2

  local default_aliases = {
    email = 'url',
    number = 'constant',
    string = 'constant',
    character = 'string',
  }

  return function(subject, options)
    local output = {}
    local options = type(options) == 'table' and options or {}
    local aliases = setmetatable(context.aliases or {}, { __index = default_aliases })
    if not options.colors then options.colors = lxsh.colors.earendel end
    subject = subject:gsub('\r\n', '\n')
    for kind, text, url in producer(decorator, subject) do
      -- Automatically prefix "mailto:" to plain e-mail addresses.
      if (url or ''):find '@' and not (url:find '://' or url:find '^mailto:') then
        url = 'mailto:' .. url
      end
      if not options.colors[kind] then
        -- Resolve aliases into concrete styles.
        while true do
          local alias = aliases[kind]
          if not alias then break end
          kind = alias
        end
      end
      output[#output + 1] = options.formatter.token(kind, text, url, options)
    end
    output = options.formatter.wrap(context, output, options)
    if options.demo then
      output = options.formatter.demo(output, options)
    end
    return output
  end

end

-- }}}1

return lxsh.highlighters
