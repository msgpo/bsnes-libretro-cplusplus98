#include <snes/snes.hpp>

namespace SNES {

Interface *interface = 0;

void Interface::videoRefresh(const uint32_t *data, bool hires, bool interlace, bool overscan) {
}

void Interface::audioSample(int16_t l_sample, int16_t r_sample) {
}

int16_t Interface::inputPoll(bool port, Input::Device::e device, unsigned index, unsigned id) {
  return 0;
}

void Interface::initialize(Interface *derived_interface) {
  interface = derived_interface;
  system.init();
}

void Interface::connect(bool port, Input::Device::e device) {
  input.connect(port, device);
}

bool Interface::cartridgeLoaded() {
  return cartridge.loaded();
}

void Interface::loadCartridge(const CartridgeData &base) {
  cartridge.rom.copy(base.data, base.size);
  cartridge.load(Cartridge::Mode::Normal, base.markup);
  system.power();
}

void Interface::loadSatellaviewSlottedCartridge(const CartridgeData &base, const CartridgeData &slot) {
  cartridge.rom.copy(base.data, base.size);
  if(slot.data) bsxflash.memory.copy(slot.data, slot.size);
  cartridge.load(Cartridge::Mode::BsxSlotted, base.markup);
  system.power();
}

void Interface::loadSatellaviewCartridge(const CartridgeData &base, const CartridgeData &slot) {
  cartridge.rom.copy(base.data, base.size);
  if(slot.data) bsxflash.memory.copy(slot.data, slot.size);
  cartridge.load(Cartridge::Mode::Bsx, base.markup);
  system.power();
}

void Interface::loadSufamiTurboCartridge(const CartridgeData &base, const CartridgeData &slotA, const CartridgeData &slotB) {
  cartridge.rom.copy(base.data, base.size);
  if(slotA.data) sufamiturbo.slotA.rom.copy(slotA.data, slotA.size);
  if(slotB.data) sufamiturbo.slotB.rom.copy(slotB.data, slotB.size);
  cartridge.load(Cartridge::Mode::SufamiTurbo, base.markup);
  system.power();
}

void Interface::loadSuperGameBoyCartridge(const CartridgeData &base, const CartridgeData &slot) {
  cartridge.rom.copy(base.data, base.size);
  GameBoy::cartridge.load(slot.markup, slot.data, slot.size);
  cartridge.load(Cartridge::Mode::SuperGameBoy, base.markup);
  system.power();
}

void Interface::unloadCartridge() {
  cartridge.unload();
}

Cartridge::Information& Interface::information() {
  return cartridge.information;
}

linear_vector<Cartridge::NonVolatileRAM>& Interface::memory() {
  return cartridge.nvram;
}

void Interface::power() {
  system.power();
}

void Interface::reset() {
  system.reset();
}

void Interface::run() {
  system.run();
}

serializer Interface::serialize() {
  system.runtosave();
  return system.serialize();
}

bool Interface::unserialize(serializer &s) {
  return system.unserialize(s);
}

void Interface::setCheats(const lstring &list) {
  if(cartridge.mode.i == Cartridge::Mode::SuperGameBoy) {
    return icd2.setCheats(list);
  }

  cheat.reset();
  foreach(code, list) {
    lstring codelist;
    codelist.split("+", code);
    foreach(part, codelist) {
      unsigned addr, data;
      if(Cheat::decode(part, addr, data)) {
        CheatCode code = {addr, data};
        cheat.append(code);
      }
    }
  }
  cheat.synchronize();
}

void Interface::message(const string &text) {
  print(text, "\n");
}

}
