--[[

 Documentation links for the BibTeX syntax highlighter of the LXSH module.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 29, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local url = 'http://amath.colorado.edu/documentation/LaTeX/reference/faq/bibstyles.html#faq'

return setmetatable({
  ['@article'] = url,
  ['@book'] = url,
  ['@booklet'] = url,
  ['@collection'] = url,
  ['@conference'] = url,
  ['@inbook'] = url,
  ['@incollection'] = url,
  ['@inproceedings'] = url,
  ['@manual'] = url,
  ['@mastersthesis'] = url,
  ['@misc'] = url,
  ['@patent'] = url,
  ['@phdthesis'] = url,
  ['@proceedings'] = url,
  ['@techreport'] = url,
  ['@unpublished'] = url,
  ['abstract'] = url,
  ['address'] = url,
  ['affiliation'] = url,
  ['annote'] = url,
  ['author'] = url,
  ['booktitle'] = url,
  ['chapter'] = url,
  ['contents'] = url,
  ['copyright'] = url,
  ['crossref'] = url,
  ['edition'] = url,
  ['editor'] = url,
  ['howpublished'] = url,
  ['institution'] = url,
  ['isbn'] = url,
  ['issn'] = url,
  ['journal'] = url,
  ['key'] = url,
  ['keywords'] = url,
  ['language'] = url,
  ['lccn'] = url,
  ['location'] = url,
  ['month'] = url,
  ['mrnumber'] = url,
  ['note'] = url,
  ['number'] = url,
  ['organization'] = url,
  ['pages'] = url,
  ['publisher'] = url,
  ['school'] = url,
  ['series'] = url,
  ['title'] = url,
  ['type'] = url,
  ['url'] = url,
  ['volume'] = url,
  ['year'] = url,
}, {
  __index = function(self, key)
    if type(key) == 'string' then
      return rawget(self, key:lower())
    end
  end
})
