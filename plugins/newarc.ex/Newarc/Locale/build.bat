echo "Building Lanuage Files"
echo %1

"..\..\Tools\lng.generator.exe" newarc.feed
move "newarc.Messages.h" "../Include/newarc.Messages.h"
move "newarc.Russian.lng" "../../Bin/%1/newarc.Russian.lng"
move "newarc.English.lng" "../../Bin/%1/newarc.English.lng"