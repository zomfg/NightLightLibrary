// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#else
#include <Windows.h>
#endif // VC_EXTRALEAN

// reference additional headers your program requires here
#include "nightlight_schema_apply.h"
#include "nightlight_schema_reflection.h"