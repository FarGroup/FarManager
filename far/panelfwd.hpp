#ifndef PANELFWD_HPP_DD0AF087_5711_436F_AC96_26B471446E97
#define PANELFWD_HPP_DD0AF087_5711_436F_AC96_26B471446E97
#pragma once

/*
panelfwd.hpp

*/
/*
Copyright © 2016 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

class Panel;
class FileList;
class TreeList;
class InfoList;
class QuickView;

typedef std::shared_ptr<Panel> panel_ptr;
typedef std::shared_ptr<FileList> file_panel_ptr;
typedef std::shared_ptr<TreeList> tree_panel_ptr;
typedef std::shared_ptr<InfoList> info_panel_ptr;
typedef std::shared_ptr<QuickView> qview_panel_ptr;

typedef const panel_ptr& panel_ptr_ref;

#endif // PANELFWD_HPP_DD0AF087_5711_436F_AC96_26B471446E97
