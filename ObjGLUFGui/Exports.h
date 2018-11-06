#ifndef OBJGLUF_EXPORTS_H
#define OBJGLUF_EXPORTS_H

#ifdef WIN32
#ifdef OBJGLF_DLL
#ifdef OBJGLUF_EXPORTS
#define OBJGLUF_GUI_API __declspec(dllexport)
#else
#define OBJGLUF_GUI_API __declspec(dllimport)
#endif
#else
#define OBJGLUF_GUI_API
#endif
#else
#define OBJGLUF_GUI_API
#endif

#endif //OBJGLUF_EXPORTS_H
