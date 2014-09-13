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
class Search;

typedef std::shared_ptr<window> window_ptr;
typedef std::shared_ptr<desktop> desktop_ptr;
typedef std::shared_ptr<FilePanels> filepanels_ptr;
typedef std::shared_ptr<FileViewer> fileviewer_ptr;
typedef std::shared_ptr<FileEditor> fileeditor_ptr;
typedef std::shared_ptr<Dialog> dialog_ptr;
typedef std::shared_ptr<VMenu> vmenu_ptr;
typedef std::shared_ptr<VMenu2> vmenu2_ptr;
typedef std::shared_ptr<Help> help_ptr;
typedef std::shared_ptr<FolderTree> foldertree_ptr;
typedef std::shared_ptr<Grabber> grabber_ptr;
typedef std::shared_ptr<HMenu> hmenu_ptr;
typedef std::shared_ptr<Search> search_ptr;
