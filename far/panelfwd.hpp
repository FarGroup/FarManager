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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class Panel;
class FileList;
class TreeList;
class QuickView;
class InfoList;

using panel_ptr = std::shared_ptr<Panel>;
using file_panel_ptr = std::shared_ptr<FileList>;
using tree_panel_ptr = std::shared_ptr<TreeList>;
using qview_panel_ptr = std::shared_ptr<QuickView>;
using info_panel_ptr = std::shared_ptr<InfoList>;

// Do not change the order, type is stored in config by value
enum class panel_type
{
	FILE_PANEL  = 0,
	TREE_PANEL  = 1,
	QVIEW_PANEL = 2,
	INFO_PANEL  = 3,
};

#endif // PANELFWD_HPP_DD0AF087_5711_436F_AC96_26B471446E97
