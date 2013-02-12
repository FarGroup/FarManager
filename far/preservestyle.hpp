#pragma once

bool PreserveStyleReplaceString(const wchar_t *Source, size_t StrSize, const string& Str, string& ReplaceStr, int& CurPos, int Position, int Case, int WholeWords, const wchar_t *WordDiv, int Reverse, int& SearchLength);
