#ifndef OBJGLUF_EXPORTS_H
#define OBJGLUF_EXPORTS_H

#ifdef _WIN32
#ifdef OBJGLF_DLL
#ifdef OBJGLUF_EXPORTS
#define OBJGLUF_API __declspec(dllexport)
#else
#define OBJGLUF_API __declspec(dllimport)
#endif
#else
#define OBJGLUF_API
#endif
#else
#define OBJGLUF_API
#endif

#endif //OBJGLUF_EXPORTS_H
