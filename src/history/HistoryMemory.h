#ifndef HISTORYMEMORY_CPP
#define HISTORYMEMORY_CPP

#include "../DisasterClient.h"
#include "../DisasterHistory.h"

#include <list>

#define DEFAULT_LIMIT 10

struct HistoryEntry {
	Datagram data;
	size_t len;
};

class HistoryMemory : public DisasterHistory
{
	// std::list<HistoryEntry> buffer;
  std::list<String> buffer;
  unsigned int limit = DEFAULT_LIMIT;

public:
  HistoryMemory(int l = DEFAULT_LIMIT)
      : limit(l){};

  void record(struct Datagram datagram, size_t len);
  void replay(DisasterClient *client);
};

#endif
