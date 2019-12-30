#pragma once

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG

#if defined DEBUG_NEW
#define new DEBUG_NEW
#endif