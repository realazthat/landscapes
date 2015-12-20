#ifndef DEBUG_MACRO_H
#define DEBUG_MACRO_H 1



#ifndef NDEBUG


#ifndef DEBUG
#define DEBUG if(1)
#endif

#ifndef SCAFFOLDING
#define SCAFFOLDING if(1)
#endif

    
#else

#ifndef DEBUG
#define DEBUG if(0)
#endif

#ifndef SCAFFOLDING
#define SCAFFOLDING if(0)
#endif

#endif


#endif
