#ifndef WINDOWSFWD_HPP_5F5E7997_B435_44BD_83DF_300F2E8BB155
#define WINDOWSFWD_HPP_5F5E7997_B435_44BD_83DF_300F2E8BB155
#pragma once

/*
windowsfwd.hpp

*/
/*
Copyright © 2014 Far Group
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

class window;
class desktop;
class FilePanels;
class FileViewer;
class FileEditor;
class Dialog;
class VMenu;
class VMenu2;
class Help;
class FolderTree;
class Grabber;
class HMenu;
class FastFind;

using window_ptr = std::shared_ptr<window>;
using desktop_ptr = std::shared_ptr<desktop>;
using filepanels_ptr = std::shared_ptr<FilePanels>;
using fileviewer_ptr = std::shared_ptr<FileViewer>;
using fileeditor_ptr = std::shared_ptr<FileEditor>;
using dialog_ptr = std::shared_ptr<Dialog>;
using vmenu_ptr = std::shared_ptr<VMenu>;
using vmenu2_ptr = std::shared_ptr<VMenu2>;
using help_ptr = std::shared_ptr<Help>;
using foldertree_ptr = std::shared_ptr<FolderTree>;
using grabber_ptr = std::shared_ptr<Grabber>;
using hmenu_ptr = std::shared_ptr<HMenu>;
using fastfind_ptr = std::shared_ptr<FastFind>;

#endif // WINDOWSFWD_HPP_5F5E7997_B435_44BD_83DF_300F2E8BB155
