struct Cartridge : property<Cartridge> {
  struct Mode { 
    enum e {
      Normal,
      BsxSlotted,
      Bsx,
      SufamiTurbo,
      SuperGameBoy,
    } i; };

  struct Region { 
    enum e {
      NTSC,
      PAL,
    } i; };

  struct Slot {
    enum e {
      Base,
      Bsx,
      SufamiTurbo,
      SufamiTurboA,
      SufamiTurboB,
      GameBoy,
    } i; };

  MappedRAM rom;
  MappedRAM ram;

  readonly<bool> loaded;
  readonly<unsigned> crc32;
  readonly<string> sha256;

  Mode mode;
  Region region;
  readonly<unsigned> ram_size;

  readonly<bool> has_bsx_slot;
  readonly<bool> has_nss_dip;
  readonly<bool> has_superfx;
  readonly<bool> has_sa1;
  readonly<bool> has_necdsp;
  readonly<bool> has_hitachidsp;
  readonly<bool> has_srtc;
  readonly<bool> has_sdd1;
  readonly<bool> has_spc7110;
  readonly<bool> has_spc7110rtc;
  readonly<bool> has_obc1;
  readonly<bool> has_st0018;
  readonly<bool> has_msu1;
  readonly<bool> has_link;

  struct NonVolatileRAM {
    const string id;
    uint8_t *data;
    unsigned size;
    Slot::e slot;
    NonVolatileRAM() : id(""), data(0), size(0), slot(Slot::Base) {}
    NonVolatileRAM(const string id, uint8_t *data, unsigned size, Slot::e slot = Slot::Base)
    : id(id), data(data), size(size), slot(slot) {}
  };
  linear_vector<NonVolatileRAM> nvram;

  struct Mapping {
    function<uint8 (unsigned)> read;
    function<void (unsigned, uint8)> write;
    Bus::MapMode mode;
    unsigned banklo;
    unsigned bankhi;
    unsigned addrlo;
    unsigned addrhi;
    unsigned offset;
    unsigned size;

    Mapping();
    Mapping(const function<uint8 (unsigned)>&, const function<void (unsigned, uint8)>&);
    Mapping(Memory&);
  };
  linear_vector<Mapping> mapping;

  struct Information {
    struct NSS {
      lstring setting;
      lstring option[16];
    } nss;
  } information;

  void load(Mode::e, const char*);
  void unload();

  void serialize(serializer&);
  Cartridge();
  ~Cartridge();

private:
  void parse_markup(const char*);
  unsigned parse_markup_integer(cstring&);
  void parse_markup_map(Mapping&, BML::Node&);

  void parse_markup_rom(BML::Node&);
  void parse_markup_ram(BML::Node&);
  void parse_markup_nss(BML::Node&);
  void parse_markup_icd2(BML::Node&);
  void parse_markup_superfx(BML::Node&);
  void parse_markup_sa1(BML::Node&);
  void parse_markup_necdsp(BML::Node&);
  void parse_markup_hitachidsp(BML::Node&);
  void parse_markup_bsx(BML::Node&);
  void parse_markup_sufamiturbo(BML::Node&);
  void parse_markup_srtc(BML::Node&);
  void parse_markup_sdd1(BML::Node&);
  void parse_markup_spc7110(BML::Node&);
  void parse_markup_obc1(BML::Node&);
  void parse_markup_setarisc(BML::Node&);
  void parse_markup_msu1(BML::Node&);
  void parse_markup_link(BML::Node&);
};

extern Cartridge cartridge;
