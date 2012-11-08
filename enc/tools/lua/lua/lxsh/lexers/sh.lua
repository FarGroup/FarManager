--[[

 Lexer for shell script code powered by LPeg.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: October 4, 2011
 URL: http://peterodding.com/code/lua/lxsh/

 TODO Whitespace? :-P

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'
local P = lpeg.P
local R = lpeg.R
local S = lpeg.S

-- The following LPeg patterns are used as building blocks.
local U, L = R'AZ', R'az' -- uppercase, lowercase
local O, D = R'07', R'09' -- octal, decimal
local X = D + R'AF' + R'af' -- hexadecimal
local W = U + L -- case insensitive letter
local A = W + D + S'_' -- identifier
local B = -(A + S'/.-') -- word boundary

-- Create a lexer definition context.
local context = lxsh.lexers.new 'sh'

context.word_boundary = B

-- Comments.
local eol = P'\r\n' + '\n'
local line = (1 - S'\r\n\f')^0 * eol^-1
context:define('comment', '#' * line)

-- Numbers.
context:define('number', lpeg.B(B) * R'09'^1 * B)

-- String literals.
local singlequoted = P"'" * ((1 - S"'") + (P'\\' * 1))^0 * "'"
local doublequoted = P'"' * ((1 - S'"') + (P'\\' * 1))^0 * '"'
context:define('string', singlequoted + doublequoted)

-- Environment variables.
context:define('variable', P'$' * A^1 + A^1 * #P'=')

-- Operators.
context:define('operator',
    '$' * S'({'
  + S'!=<>;()[]{}|`&\\'
  + '.' * #P' \t'
  + (lpeg.B(B) * '-' * (L * L^-1 + P(1)) * B))

-- Keywords.
context:define('keyword', context:keywords [[
  alias bg bind break builtin caller case cd command compgen complete continue
  declare dirs disown do done echo elif else enable esac eval exec exit export
  false fc fg fi for getopts hash help history if in jobs kill let local logout
  popd printf pushd pwd read readonly return select set shift shopt source
  suspend test then time times trap true type typeset ulimit umask unalias
  unset until which while
]])

-- Common external programs.
context:define('command', context:keywords [[
  cat cmp cp curl cut date find grep gunzip gvim gzip kill lua make mkdir mv
  php pkill python rm rmdir rsync ruby scp sed sleep ssh sudo tar unlink wget zip
]])

-- Git sub commands.
context:define('command', P'git' * S' \t'^1 * context:keywords [[
  add am annotate apply applymbox applypatch archimport archive bisect blame
  branch cat-file check-attr check-ref-format checkout checkout-index cherry
  cherry-pick clean clone clone-pack commit commit-tree config convert-objects
  count-objects cvsexportcommit cvsimport describe diff diff-files diff-index
  diff-stages diff-tree fetch fetch-pack for-each-ref format-patch fsck gc
  get-tar-commit-id grep hash-object imap-send index-pack init instaweb
  local-fetch log lost-found ls-files ls-remote ls-tree mailinfo mailsplit
  merge merge-base merge-file merge-index merge-tree mergetool mktag mktree mv
  name-rev pack-objects pack-redundant pack-refs parse-remote patch-id
  peek-remote prune prune-packed pull push quiltimport read-tree rebase reflog
  relink remote repack request-pull rerere reset rev-list revert rm send-email
  shortlog show show-branch show-index show-ref stage stash status svn
  svnimport symbolic-ref tag tar-tree unpack-file unpack-objects update-index
  update-ref update-server-info var verify-pack verify-tag whatchanged
  write-tree
]])

-- Mercurial sub commands.
context:define('command', P'hg' * S' \t'^1 * context:keywords [[
  add addremove annotate archive backout bisect blame bookmarks branch branches
  bundle cat checkout ci clone co commit copy cp debugconfig diff export forget
  grep heads help history id identify import in incoming init locate log
  manifest merge move mv out outgoing parents patch paths pull push recover
  remove rename resolve revert rm rollback root serve showconfig st status sum
  summary tag tags tip unbundle up update verify version
]])

-- Define an `error' token kind that consumes one character and enables
-- the lexer to resume as a last resort for dealing with unknown input.
context:define('error', 1)

-- Compile the final LPeg pattern to match any single token and return the
-- table containing the various definitions that make up the Lua lexer.
return context:compile()

-- vim: ts=2 sw=2 et
