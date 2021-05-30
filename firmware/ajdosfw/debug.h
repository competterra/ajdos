#define PRINTLOG Serial
#ifdef DEBUG
  #define LOG(...)   { PRINTLOG.print( __VA_ARGS__ );   }
  #define LOGLN(...) { PRINTLOG.println( __VA_ARGS__ ); }
  #define LOGF(...)  { PRINTLOG.printf( __VA_ARGS__ );  }
#else
  #define LOG(...)   {}
  #define LOGLN(...) {}
  #define LOGF(...)  {}  
#endif
