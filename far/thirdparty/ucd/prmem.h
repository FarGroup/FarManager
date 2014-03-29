#pragma once

#define PR_Malloc malloc
#define PR_Calloc calloc
#define PR_Realloc realloc
#define PR_Free free
#define PR_FREEIF(ptr) PR_Free(ptr)
