#ifndef HISTORYMEMORY_CPP
#define HISTORYMEMORY_CPP

#include "../DisasterClient.h"
#include "../DisasterHistory.h"

#include <list>

#define DEFAULT_LIMIT 10

class HistoryMemory : public DisasterHistory
{
  std::list<String> buffer;
  unsigned int limit = DEFAULT_LIMIT;

public:
  HistoryMemory(int l = DEFAULT_LIMIT)
      : limit(l){};

  void record(String message);
  void replay(DisasterClient *client);
};

#endif
