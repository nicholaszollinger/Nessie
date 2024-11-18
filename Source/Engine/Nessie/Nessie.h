#pragma once
#if NESSIE_EXPORTS
    #define NESSIE_API __declspec(dllexport)
#else
    #define NESSIE_API __declspec(dllimport)
#endif