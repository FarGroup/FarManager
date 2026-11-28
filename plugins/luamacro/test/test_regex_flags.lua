-- test_regex_flags.lua

local function run_regexp_tests()
	-- Each entry: { pattern, flags, subject, expected_start, expected_end }
	-- (expected_start == nil means expected no match)
	local cases = {
		-- 1. Baseline compatibility
		{ [[a*?ca]], nil, "abca", 3, 4 },
		{ [[^\[([\w.]+)\]:\s*\[(.*)\]$]], nil, "[init.svc.imsdatadaemon]: [running]", 1, 35 },
    { [[ \$ \{ (\w+) \} | \$ (\w+) ]], "x", "echo ${HOME}", 6, 12 },

    -- 2. Scoped (?i) and (?-i) toggling
    { [[^(?i)abc$]], nil, "ABC", 1, 3 },
    { [[^(?i)abc(?-i)def$]], nil, "ABCdef", 1, 6 },
    { [[^(?i)abc(?-i)def$]], nil, "ABCDEF", nil },
    { [[^abc(?-i)def$]], "i", "ABCdef", 1, 6 },
    { [[^abc(?-i)def$]], "i", "ABCDEF", nil },
    { [[^(?i)a(?-i)b(?i)c$]], nil, "AbC", 1, 3 },
    { [[^(?i)a(?-i)b(?i)c$]], nil, "ABC", nil },

    -- 3. Bracket scope isolation: (?i) inside (...) does not leak past ')'
    { [[^a((?i)b(c)d)e$]], nil, "aBCDe", 1, 5 },
    { [[^a((?i)b(c)d)e$]], nil, "ABCDE", nil },
    { [[^a((?-i)b(c)d)e$]], "i", "AbcdE", 1, 5 },
    { [[^a((?-i)b(c)d)e$]], "i", "ABCDE", nil },

    -- 4. Alternation with inline flags
    { [[^(?i)foo|bar$]], nil, "FOO", 1, 3 },
    { [[^(?i)foo|bar$]], nil, "BAR", 1, 3 },
    { [[^(?i)foo|bar$]], nil, "baz", nil },
    { [[^(?:(?i)foo|(?-i)bar)$]], nil, "FOO", 1, 3 },
    { [[^(?:(?i)foo|(?-i)bar)$]], nil, "bar", 1, 3 },
    { [[^(?:(?i)foo|(?-i)bar)$]], nil, "BAR", nil },

    -- 5. Non-capturing flag groups (?i:...), nested flags, quantified flag groups
    { [[^a(?i:bc)d$]], nil, "aBCd", 1, 4 },
    { [[^a(?i:bc)d$]], nil, "ABcd", nil },
    { [[^a(?-i:bc)d$]], "i", "AbcD", 1, 4 },
    { [[^a(?-i:bc)d$]], "i", "ABCD", nil },
    { [[^a(?i:b(?-i:c)d)e$]], nil, "aBcDe", 1, 5 },
    { [[^a(?i:b(?-i:c)d)e$]], nil, "aBCDe", nil },
    { [[^(?i:a)+$]], nil, "aAa", 1, 3 },

    -- 6. Character classes [...], ranges, negated classes, single-char folding
    { [[^(?i)[a-c]+(?-i)[d-f]+$]], nil, "AbCdef", 1, 6 },
    { [[^(?i)[a-c]+(?-i)[d-f]+$]], nil, "AbCdeF", nil },
    { [[^(?i)[x]+(?-i)[y]+$]], nil, "xXyy", 1, 4 },
    { [[^(?i)[x]+(?-i)[y]+$]], nil, "xXyY", nil },
    { [[^(?i)[^x]+(?-i)[^y]+$]], nil, "abcz", 1, 4 },
    { [[^(?i)[^x]+(?-i)[^y]+$]], nil, "abXy", nil },
    { [[^(?i)[\x41]+(?-i)[\x42]+$]], nil, "aABB", 1, 4 },
    { [[^(?i)[\x41]+(?-i)[\x42]+$]], nil, "aAbb", nil },

    -- 7. Lazy quantifiers (MinRange + MinSkip) and TrimTail
    { [[(?i)a+?(?-i)b]], nil, "AAb", 1, 3 },
    { [[(?i)a+?(?-i)b]], nil, "AAB", nil },
    { [[(?i)[^a]+?(?-i)b]], nil, "xyb", 1, 3 },
    { [[(?i)[^a]+?(?-i)b]], nil, "xAb", nil },
    { [[(?i)a+(?-i)b+]], nil, "zzzAAbbzzz", 4, 7 },
    { [[(?i)a+(?-i)b+]], nil, "zzzAABBzzz", nil },

    -- 8. Quoting \Q...\E and hex escapes \xHH
    { [[^(?i)\QAbC\E(?-i)\QDeF\E$]], nil, "aBcDeF", 1, 6 },
    { [[^(?i)\QAbC\E(?-i)\QDeF\E$]], nil, "aBcdef", nil },
    { [[^(?i)\x41(?-i)\x42$]], nil, "aB", 1, 2 },
    { [[^(?i)\x41(?-i)\x42$]], nil, "ab", nil },

    -- 9. Backreferences \1 and named backreferences \p{name} (single and quantified)
    { [[^([a-z]+)-(?i)\1$]], nil, "abc-ABC", 1, 7 },
    { [[^([a-z]+)-(?-i)\1$]], "i", "ABC-abc", nil },
    { [[^(?{tag}[a-z]+)-(?i)\p{tag}$]], nil, "div-DIV", 1, 7 },
    { [[^(?{tag}[a-z]+)-(?-i)\p{tag}$]], "i", "DIV-div", nil },
    { [[^([a-z]+)-(?i)\1+$]], nil, "ab-ABab", 1, 7 },
    { [[^([a-z]+)-(?-i)\1+$]], "i", "AB-ab", nil },

    -- 10. Inline (?s), (?m), (?x) and combined flags (?is), (?i-s)
    { [[^(?s).+$]], nil, "a\nb", 1, 3 },
    { [[^(?-s).+$]], "s", "a\nb", nil },
    { [[^(?s:a.b).c$]], nil, "a\nbXc", 1, 5 },
    { [[^(?s:a.b).c$]], nil, "a\nb\nc", nil },
    { [[^(?is)a.b$]], nil, "A\nB", 1, 3 },
    { [[^(?i-s)a.b$]], "s", "A\nB", nil },
    { [[^(?i-s)a.b$]], "s", "AXB", 1, 3 },
    { [[(?m)^bar$]], nil, "foo\nbar\nbaz", 5, 7 },
    { [[(?-m)^bar$]], "m", "foo\nbar\nbaz", nil },
    { [[^(?x) a b (?-x: c d ) e $]], nil, "ab c d e", 1, 8 },
    { [[^(?x) a b (?-x: c d ) e $]], nil, "abcde", nil },

    -- 11. Lookahead / lookbehind with inline flags
    { [[^(?=(?i)abc)a..$]], nil, "aBC", 1, 3 },
    { [[(?<=(?i)ab)c]], nil, "ABc", 3, 3 },
    { [[(?<=(?-i)ab)c]], "i", "ABc", nil },

    -- 12. Quantified groups with inline flags inside
    { [[^((?i)ab)+(?-i)cd$]], nil, "ABabcd", 1, 6 },
    { [[^((?i)ab)+(?-i)cd$]], nil, "ABabCD", nil },
    { [[^(?i:ab){2,3}$]], nil, "aBAb", 1, 4 },

    -- 13. Non-ASCII / Unicode case folding (9 codepoints)
    { [[^(?i)привет(?-i)мир$]], nil, "ПРИВЕТмир", 1, 9 },
    { [[^(?i)привет(?-i)мир$]], nil, "ПРИВЕТМИР", nil },

    -- 14. Empty flag groups
    { [[^(?i)$]], nil, "", 1, 0 },
    { [[^(?i:)$]], nil, "", 1, 0 },

    -- 15. Atomic groups (?>subexp)
    { [[^(?>a+)ab$]], nil, "aaab", nil },
    { [[^(?>a+)b$]], nil, "aaab", 1, 4 },
    { [[^(?>ab|a)b$]], nil, "ab", nil },
    { [[^(?>ab|ac)d$]], nil, "acd", 1, 3 },
    { [[^(?:(?>a+)ab|a+b)$]], nil, "aaab", 1, 4 },
    { [[^(?>a(?>b|bc)d)$]], nil, "abcd", nil },
    { [[^(?>a(?>b|bc)d)$]], nil, "abd", 1, 3 },
    { [[^(?>a+)+ab$]], nil, "aaab", nil },
    { [[^(?>a+)+?ab$]], nil, "aaab", nil },
    { [[^(?>(?i)ab)cd$]], nil, "ABcd", 1, 4 },
    { [[^(?>(?i)ab)cd$]], nil, "ABCD", nil },
    { [[(?<=(?>ab|cd))e]], nil, "cde", 3, 3 },
  }

  for i, c in ipairs(cases) do
    local rx = assert(regex.new(c[1], c[2]), "compile failed at #" .. i)
    local s, e = rx:find(c[3])
    assert(s == c[4] and e == c[5],
      ("case #%d (%s) failed: expected (%s,%s), got (%s,%s)"):format(
        i, c[1], tostring(c[4]), tostring(c[5]), tostring(s), tostring(e)))
  end

  -- 16. Capture indexing and named captures via tfind / match / gsub / gmatch
  do
    local rx = assert(regex.new([[(?i)(a)(?i:b)(?{third}c)]]))
    assert(rx:bracketscount() == 3)
    local c1, c2 = rx:match("ABC")
    assert(c1 == "A" and c2 == "C")

    local s, e, t = rx:tfind("ABC")
    assert(s == 1 and e == 3 and t[1] == "A" and t[2] == "C" and t.third == "C")

    local rx_atomic = assert(regex.new([[^(?>([ab]+))c$]]))
    assert(rx_atomic:bracketscount() == 2)
    assert(rx_atomic:match("abc") == "ab")

    local rx_atomic_rb = assert(regex.new([[^(?:(?>([ab]+))c|abbd)$]]))
    local rs, re, rt = rx_atomic_rb:tfind("abbd")
    assert(rs == 1 and re == 4 and rt[1] == false)

    local out, n = regex.gsub("foo-FOO bar-bAr", [[([a-z]+)-(?i)\1]], "%1")
    assert(out == "foo bar" and n == 2)

    local items = {}
    for w in regex.gmatch("Abc DEF gHi", [[(?i)abc|(?-i)DEF|(?i)ghi]]) do
      items[#items + 1] = w
    end
    assert(#items == 3 and items[1] == "Abc" and items[2] == "DEF" and items[3] == "gHi")
  end

  -- 17. Malformed flag constructs & invalid quantifier placement
  local invalid = {
    "(?)",
    "(?-)",
    "(?--i)",
    "(?i-)",
    "(?z)",
    "(?i:abc",
    "(?>abc",
    "(?i)+",
    "(?>+)",
    "a(?i)+",
    "a{1,1}+",
    "a{1,1}?",
  }
  for _, pat in ipairs(invalid) do
    assert(not pcall(regex.new, pat), "expected compile error for: " .. pat)
  end

  -- far.Message(("All %d RegExp test cases passed!"):format(#cases), "RegExp Tests")
end

return run_regexp_tests
