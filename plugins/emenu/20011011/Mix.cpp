#ifndef __mix_cpp
#define __mix_cpp

inline const char *GetMsg(int MsgId)
{
  return g_Info.GetMsg(g_Info.ModuleNumber, MsgId);
}

void InitDialogItems(struct InitDialogItem *Init,
		     struct FarDialogItem *Item, int ItemsNumber)
{
  for (int ni = 0; ni < ItemsNumber; ni++)
    {
      Item[ni].Type = Init[ni].Type;
      Item[ni].X1 = Init[ni].X1;
      Item[ni].Y1 = Init[ni].Y1;
      Item[ni].X2 = Init[ni].X2;
      Item[ni].Y2 = Init[ni].Y2;
      Item[ni].Focus = Init[ni].Focus;
      Item[ni].Selected = Init[ni].Selected;
      Item[ni].Flags = Init[ni].Flags;
      Item[ni].DefaultButton = Init[ni].DefaultButton;

      strcpy(Item[ni].Data, ((unsigned int) Init[ni].Data < 2000) ?
	     GetMsg((unsigned int) Init[ni].Data) : Init[ni].Data);
    }
}

#endif // __mix_cpp
