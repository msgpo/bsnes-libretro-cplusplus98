#include "bus/bus.hpp"

class SuperFX : public Coprocessor, public MMIO {
public:
  #include "core/core.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"
  #include "disasm/disasm.hpp"

  static void Enter();
  void enter();
  void init();
  void enable();
  void power();
  void reset();
  void serialize(serializer&);

private:
  unsigned clockmode;
  unsigned instruction_counter;
};

extern SuperFX *superfx;
extern SuperFXBus *superfxbus;
