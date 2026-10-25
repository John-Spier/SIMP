#include <srl.hpp>

#define STARTHELD(x) (x.IsHeld(SRL::Input::Digital::Button::START) || x.WasPressed(SRL::Input::Digital::Button::START))

constexpr int32_t DISPLAY_FILES = 10;
constexpr int32_t START_LINES = 4;

enum FileType : uint32_t
{
    TYPE_BASE = 0xFFFFFF00,
    TYPE_SSF = TYPE_BASE + 0x18,
    TYPE_MOD = TYPE_BASE + 0x1B,
    TYPE_VGM = TYPE_BASE + 0x1C,
    TYPE_MDX = TYPE_BASE + 0x1D,
    TYPE_VGM_DRIVER = TYPE_BASE + 0x1A,
    TYPE_DIR = TYPE_BASE + 0xFB,
    TYPE_VFS = TYPE_BASE + 0xFE,
    TYPE_VFS2 = TYPE_BASE + 0xED,
    TYPE_LASTDIR = TYPE_BASE + 0xF4,
    TYPE_DIR_UP = TYPE_BASE + 0xEE,
    TYPE_AIFF = TYPE_BASE + 0x1E,
    TYPE_ADPCM = TYPE_BASE + 0x1F,
    TYPE_VGM_DRIVER_MANUAL = TYPE_BASE + 0x20,
    TYPE_SEQ = TYPE_BASE + 0x21,
    TYPE_TON = TYPE_BASE + 0x22,
    TYPE_SSF2 = TYPE_BASE + 0x23, //seq/ton/etc
    TYPE_DSPPROG = TYPE_BASE + 0x24,
    TYPE_DSPRAM = TYPE_BASE + 0x25,
    TYPE_CINEPAK = TYPE_BASE + 0x26,
    TYPE_MAP = TYPE_BASE + 0x29, //for ssf2
    TYPE_TSK = TYPE_BASE + 0x2A, //for ssf2
    TYPE_CDDA = TYPE_BASE + 0x07,
    TYPE_CDTRACKS = TYPE_BASE + 0xFA
};

enum class MemoryLocation : uint8_t
{
    HwRAM = 0,
    LwRAM = 1,
    CartRAM = 2
};

enum class VgmDriverLoadMode : uint8_t
{
    Automatic = 0,
    Manual = 1
};

enum class ColorPreset : uint8_t
{
    Custom = 0,
    Black = 1,
    Red = 2,
    Green = 3,
    Blue = 4,
    Yellow = 5,
    Magenta = 6
};

enum OSDItem : uint8_t
{
    OSD_CDDA = 0, //use .. from root
    OSD_SETCOLOR = 1,
    OSD_RED = 2,
    OSD_GREEN = 3,
    OSD_BLUE = 4,
    OSD_REPEAT = 5,
    OSD_REPEATFRAMES = 6,
    OSD_TURBO = 7, //use buttons
    OSD_TURBOSPEED = 8,
    OSD_DRIVERLOAD = 9,
    OSD_REMOVEDRIVER = 10,
    OSD_DRIVERLOC = 11,
    OSD_MENULOC = 12,
    OSD_VALIDLOC = 13,
    OSD_TEMPLOC = 14,
    OSD_ABSOLUTE = 15,
    OSD_RAMBUFFER = 16,
    OSD_SOUNDBUFFER = 17,
    OSD_SOUNDBUFLOC = 18,
    OSD_RELOADCD = 19,
    OSD_TONOFFSET = 20, //automatic
    OSD_SEQOFFSET = 21, //driver decides
    OSD_PAUSEBUFFER = 22,
    OSD_SAMPLERATE = 23, //also doesnt work
    OSD_PANL = 24,
    OSD_PANR = 25,
    OSD_BANK = 26,
    OSD_VOLUME = 27
};

struct MenuItem
{
    uint32_t type;
    size_t offset; //uint32
    int32_t size; //relative to the start of the file
    char name[64];
};

struct VfsFile //ALL LITTLE ENDIAN
{
	char name[64];
	int32_t	size;//[4];
	int32_t	addr;//[4];
	int32_t	sector_size;//[4];
	int32_t	byte_addr;//[4];
	uint32_t stack;//[4];
};

struct VfsHeader //LITTLE ENDIAN
{
    char signature[4]; // "VFS\0"
    int32_t file_count;//[4];
    int32_t directory_sectors;//[4]; //sectors are 2048 bytes
};

struct MemAlignment
{
    int32_t entries;
    uint32_t bytes;
};

struct OSDITEM {
    const char* name;
    const char* intname;
    OSDItem type; //makes things easier
    int32_t stringmin;
    int32_t stringmax;
    const char* const* strings;
    bool nodata; //for items that just perform an action when selected
};

//(mod 512) + 73, should be converted to do this
constexpr std::array<MemAlignment, 20> AlignmentsWithHeader = {{
    {  9801, 823296 },
    {  9289, 780288 },
    {  8777, 737280 },
    {  8265, 694272 },
    {  7753, 651264 },
    {  7241, 608256 },
    {  6729, 565248 },
    {  6217, 522240 },
    {  5705, 479232 },
    {  5193, 436224 },
    {  4681, 393216 },
    {  4169, 350208 },
    {  3657, 307200 },
    {  3145, 264192 },
    {  2633, 221184 },
    {  2121, 178176 },
    {  1609, 135168 },
    {  1097,  92160 },
    {   585,  49152 },
    {    73,   6144 }
}};

//mod 512
constexpr std::array<MemAlignment, 20> AlignmentsWithoutHeader = {{
    {  9728, 817152 },
    {  9216, 774144 },
    {  8704, 731136 },
    {  8192, 688128 },
    {  7680, 645120 },
    {  7168, 602112 },
    {  6656, 559104 },
    {  6144, 516096 },
    {  5632, 473088 },
    {  5120, 430080 },
    {  4608, 387072 },
    {  4096, 344064 },
    {  3584, 301056 },
    {  3072, 258048 },
    {  2560, 215040 },
    {  2048, 172032 },
    {  1536, 129024 },
    {  1024,  86016 },
    {   512,  43008 },
    {     0,      0 }
}};

//this stuff should probably be enums
constexpr std::array<const char*, 7> ColorStrings = {{
    "Custom",
    "Black",
    "Red",
    "Green",
    "Blue",
    "Yellow",
    "Magenta"
}};

constexpr std::array<const char*, 2> VgmDriverLoadStrings = {{
    "Automatic",
    "Manual"
}};

constexpr std::array<const char*, 3> RAMLocationStrings = {{
    "HwRAM",
    "LwRAM",
    "CartRAM"
}};

const std::array<OSDITEM, 19> OSDMenuItems = {{
    {"", "*Unload VGM Driver", OSD_REMOVEDRIVER, 1, -1, nullptr, true}, //this needs to be first or button held breaks
    {"VGM Driver Load Mode", "", OSD_DRIVERLOAD, 0, 1, VgmDriverLoadStrings.data(), false},
    {"", "Volume", OSD_VOLUME, 1, -1, nullptr, false},
    //{"", "*Swap CD", OSD_RELOADCD, 1, -1, nullptr, true}, //not working
    {"", "Turbo Speed", OSD_TURBOSPEED, 1, -1, nullptr, false},
    {"", "Custom Red", OSD_RED, 1, -1, nullptr, false},
    {"", "Custom Green", OSD_GREEN, 1, -1, nullptr, false},
    {"", "Custom Blue", OSD_BLUE, 1, -1, nullptr, false},
    {"*Set Color", "", OSD_SETCOLOR, 0, 6, ColorStrings.data(), false},
    {"", "Frames until repeat", OSD_REPEAT, 1, -1, nullptr, false},
    {"", "Frames between repeat", OSD_REPEATFRAMES, 1, -1, nullptr, false},
    {"Driver Location", "", OSD_DRIVERLOC, 0, 2, RAMLocationStrings.data(), false},
    {"Menu Data Location", "", OSD_MENULOC, 0, 2, RAMLocationStrings.data(), false},
    {"Temp Data Location", "", OSD_TEMPLOC, 0, 2, RAMLocationStrings.data(), false},
    {"", "Current Filetype", OSD_ABSOLUTE, 1, -1, nullptr, false},
    {"", "ADPCM Buffer Size", OSD_RAMBUFFER, 1, -1, nullptr, false},
    {"", "Sound RAM Buffer Size", OSD_SOUNDBUFFER, 1, -1, nullptr, false},
    {"", "Stream Buffer Offset", OSD_SOUNDBUFLOC, 1, -1, nullptr, false},
    {"", "Pause Buffer Size", OSD_PAUSEBUFFER, 1, -1, nullptr, false},
    //{"", "Pan Left/Mono", OSD_PANL, 1, -1, nullptr, false},
    //{"", "Pan Right", OSD_PANR, 1, -1, nullptr, false},
    {"", "Bank", OSD_BANK, 1, -1, nullptr, false}
}};

void TurnOff68K();
void TurnOn68K();
int32_t LoadDriver(bool AllowReload = false);
int32_t LoadDriverSong(const char* fileName, int32_t offset, int32_t size, uint32_t type, bool StartHeld = false);
int32_t LoadVFS(const char* fileName, int32_t baseOffset = 0, bool AllFilesValid = false, bool isLastDir = false, bool isVFS2 = false);
int32_t LoadISO9660(const char* fileName, int32_t baseOffset = 0, bool NotAllFilesValid = false, bool isLastDir = false);
int32_t DrawFileList(int32_t highlight, int32_t offset);
void MoveHighlight(int32_t i, int32_t numFiles, int32_t *highlight, int32_t *offset);
const char* GetFileTypeString(uint32_t type);
const char* GetLongTypeString(uint32_t type);
uint32_t GetExtType(const char* filename);
SRL::Input::Digital::Button ButtonPressed(SRL::Input::Digital& pad, bool allowHold);
int32_t LoadFile(MenuItem* item, bool StartHeld);
int32_t LoadSSF(MenuItem* item);
bool PlayPauseMusic(bool playing);
int16_t SetVolume(int16_t newVolume, bool relative = true);
int32_t SetTempo(int32_t newTempo, bool relative = true);
void PrintTime(uint8_t x, uint8_t y);
int32_t LoadTOC(bool DontWaitBusy = false);
void LoadCDDA(int32_t track, bool StartHeld = false);
int32_t LoadSSF2File(const char* fileName, int32_t offset, int32_t size, uint32_t type, bool StartHeld = false);
int32_t LoadSegaDriver(const char* fileName, int32_t offset = 0, int32_t size = 0);
void DisplayHelp(SRL::Input::Digital& pad);
void DisplaySettings(int32_t selectedItem, SRL::Input::Digital& pad);
int32_t IncDecOSD(OSDItem type, int32_t value, bool noset, int32_t selectedItem);
void FrameSetTempo(int32_t newTempo);
int32_t LoadSSF2(const char* fileName, int32_t offset, int32_t size, bool StartHeld = false);

/* //VECTORS STORE DATA IN HWRAM, RAW ARRAYS ARE NEEDED
std::vector<MenuItem>* menuItems = lwnew std::vector<MenuItem>();
std::vector<uint32_t>* validFileList = lwnew std::vector<uint32_t>();
std::vector<uint8_t>* vgmdriver = lwnew std::vector<uint8_t>();
*/

uint8_t* vgmdriver = nullptr;
uint8_t* segaDriver = nullptr;
uint32_t* validFileList = nullptr;
MenuItem* menuItems = nullptr;
int32_t fileCount = 0;
int32_t validFiles = 0;
int32_t driverSize = 0;
int32_t segaDriverSize = 0;
char currentFile[16] = {0};
int32_t currentFileFid = -1;
std::vector<int32_t> lastDirOffsets;
uint8_t* driverEnd = nullptr;
bool MusicPlaying = false;
bool ManualVGMDriver = false;
bool TurboMode = false;
bool WaitedAfterTurningOff68K = false;
bool SddrvsLoaded = false;
uint8_t DriverLocation = static_cast<uint8_t>(MemoryLocation::LwRAM);
uint8_t MenuLocation = static_cast<uint8_t>(MemoryLocation::LwRAM);
uint8_t TempLocation = static_cast<uint8_t>(MemoryLocation::HwRAM);
uint32_t MusicType = 0;
int32_t RingBufferSize = 0x10000;
int32_t SoundBufferSize = 0x10000;
int32_t SoundBufferOffset = 0x20000;
int32_t PauseBufferSize = 0x2000;
int32_t SampleRate = 44100;
int16_t volume = 127; //for clamping
int16_t DirLevel = 0;
int32_t tempo = 4096; //2x speed
//int8_t panL = 0; //center
//int8_t panR = 0; //center
int16_t bank = 1; //for sega driver
int8_t colorR = 0;
int8_t colorG = 0;
int8_t colorB = 0;
uint8_t repeatFrames = 2;
uint16_t repeatDelay = 30;
bool LoadDriverManual = false;
int8_t MenuColor = static_cast<int8_t>(ColorPreset::Custom);

//MenuItem VGMDRIVER;
//int32_t VGMDRIVER_OFFSET = 0;


class AudioStream
{
    private:
        PcmHn phandle = nullptr;
        SRL::Cd::File file;
        PcmWork* work = nullptr;
        int8_t* ring = nullptr;
        int32_t* pauseBuffer = nullptr;
    public:
    AudioStream() : file("") //NEVER USE
    {

    }
    ~AudioStream()
    {
        if (this->phandle != nullptr)
        {
            PCM_Stop(this->phandle);
            PCM_DestroyGfsHandle(this->phandle);
            this->phandle = nullptr;
        }
        file.Close();
        if (this->work)
        {
            delete this->work;
            this->work = nullptr;
        }
        if (this->ring)
        {
            delete[] this->ring;
            this->ring = nullptr;
        }
        if (this->pauseBuffer)
        {
            delete[] this->pauseBuffer;
            this->pauseBuffer = nullptr;
        }
        PCM_Finish();
    }
    AudioStream(const char* fileName, uint32_t offset, int32_t size, int32_t fid = -1) : file(fileName)
    {
        SRL::Sound::Hardware::Initialize(); //have to do this every time for some reason
        
        PCM_Init();
        PCM_DeclareUseAdpcm();
        if (!SddrvsLoaded)
        {
            SddrvsLoaded = true;
        }
        WaitedAfterTurningOff68K = false;
        switch(TempLocation)
        {
            case 0:
                this->work = new PcmWork();
                break;
            case 1:
                this->work = lwnew PcmWork();
                break;
            case 2:
                this->work = cartnew PcmWork();
                break;
        }
        switch(DriverLocation)
        {
            case 0:
                this->ring = new int8_t[RingBufferSize];
                break;
            case 1:
                this->ring = lwnew int8_t[RingBufferSize];
                break;
            case 2:
                this->ring = cartnew int8_t[RingBufferSize];
                break;
        }
        PcmCreatePara p = {};
        
        PCM_PARA_WORK(&p) = this->work;
        PCM_PARA_RING_ADDR(&p) = this->ring;
        PCM_PARA_RING_SIZE(&p) = RingBufferSize;
        PCM_PARA_PCM_ADDR(&p) = (int8_t*)(0x25A00000 + SoundBufferOffset);
        PCM_PARA_PCM_SIZE(&p) = SoundBufferSize;

        if (offset > 0)
        {
            int32_t fileFid = (fid >= 0) ? fid : file.GetIdentifier();
            GfsHn gfs = GFS_Open(fileFid);
            int32_t startFad = GFS_GetFad(fileFid, offset);
            int32_t sectorCount = (size + 2047) / 2048;
            gfs->flow.finfo.finfo.fad = startFad;
            gfs->flow.finfo.finfo.size = size;
            gfs->flow.finfo.nsct = sectorCount;
            gfs->flow.finfo.sctsz = 2048;
            gfs->flow.finfo.lstrm = size - ((sectorCount - 1) * 2048);
            gfs->flow.dtsrc.rsrc.cd.sfad = startFad;
            gfs->flow.dtsrc.rsrc.cd.efad = startFad + sectorCount;
            file.Handle = gfs;
            this->phandle = PCM_CreateGfsHandle(&p, gfs);
        }
        else
        {
            file.Open(); //need this for GFS
            this->phandle = PCM_CreateGfsHandle(&p, file.Handle);
        }

        if (this->phandle != nullptr)
        {
            PCM_Start(this->phandle);
        }
        if (PauseBufferSize > 0)
        {
            switch(DriverLocation)
            {
                case 0:
                    this->pauseBuffer = new int32_t[PauseBufferSize / sizeof(int32_t)];
                    break;
                case 1:
                    this->pauseBuffer = lwnew int32_t[PauseBufferSize / sizeof(int32_t)];
                    break;
                case 2:
                    this->pauseBuffer = cartnew int32_t[PauseBufferSize / sizeof(int32_t)];
                    break;
            }
            PCM_SetPauseWork(this->pauseBuffer, PauseBufferSize);
        }
        //MusicType = type;
        MusicPlaying = true;
    }

    void Task()
    {
        if (this->phandle != nullptr)
        {
            PCM_Task(this->phandle);
        }
    }

    bool PlayPause(bool playing)
    {
        if (this->phandle != nullptr && this->pauseBuffer != nullptr)
        {
            if (playing)
            {
                PCM_Pause(this->phandle, PCM_PAUSE_ON_AT_ONCE);
            }
            else
            {
                PCM_Pause(this->phandle, PCM_PAUSE_OFF);
            }
        }
        return !playing;
    }
    void SetVol(int32_t newVolume)
    {
        if (this->phandle != nullptr)
        {
            PCM_SetVolume(this->phandle, newVolume >> 4);  //only 8 volume levels
            PCM_ChangePcmPara(this->phandle);
        }
    }
    int32_t GetTime(bool seconds = true)
    {
        if (this->phandle != nullptr)
        {
            if (seconds)
            {
                return PCM_GetTime(this->phandle) / this->work->status.info.sampling_rate;
            }
            return PCM_GetTime(this->phandle);
        }
        return -1;
    }
};


/** @brief Cinepak video player with 16-bit (VDP1 sprite) and 32-bit (VDP2 bitmap) modes
 * Controls: B=pause, C=quit, X=prev, Z=next, Y=restart,
 * Start=toggle 16/32-bit, D-pad=move, L/R=volume
 * @note In 32-bit mode, VDP1 sprites can be drawn on top of the video via the OnOverlay callback.
 */
class CinepakVDP2Player
{
public:
    /** @brief Overlay callback, called each frame after video transfer.
     * Draw VDP1 sprites here to overlay text/graphics on the video.
     * VDP1 commands are flushed via slSynch() after this callback returns.
     */
    SRL::Types::Event<CinepakVDP2Player&> OnOverlay;

    CinepakVDP2Player(SRL::Input::Digital& pad, MenuItem* items, int32_t itemCount, int32_t index)
        : pad(pad), items(items), itemCount(itemCount), requestedIndex(index) {}

    void Play()
    {
        Setup();

        int32_t cpkIndices[64];
        int32_t cpkCount = 0;
        if (requestedIndex == -1)
        {
            for (int32_t i = 0; i < itemCount && cpkCount < 64; i++)
                if (items[i].type == TYPE_CINEPAK)
                    cpkIndices[cpkCount++] = i;
        }
        else
        {
            cpkIndices[0] = requestedIndex;
            cpkCount = 1;
        }

        if (cpkCount == 0) { Teardown(); return; }

        int32_t current = 0;
        while (current >= 0 && current < cpkCount)
        {
            auto result = PlayItemWithControls(items[cpkIndices[current]]);
            switch (result)
            {
                case Result::Quit:      current = cpkCount; break;
                case Result::Next:      current++; break;
                case Result::Previous:  if (current > 0) current--; break;
                case Result::Restart:   break;
                case Result::Ended:     current++; break;
            }
        }

        Teardown();
    }

private:
    enum class Result { Ended, Quit, Next, Previous, Restart };

    static constexpr uint32_t CYCLE_A_REG = 0x25f80010;
    static constexpr uint32_t CYCLE_B_REG = 0x25f80018;
    static constexpr uint32_t CYCPAT_BM_READ = 0x44444444;
    static constexpr uint32_t CYCPAT_CPU_RW = 0xffffffff;
    static constexpr uint32_t RING_BUF_SIZ = 1024L * 200;
    static constexpr uint32_t PCM_SIZE = 4096L * 16;

    SRL::Input::Digital& pad;
    MenuItem* items;
    int32_t itemCount;
    int32_t requestedIndex;

    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    uint32_t movie_x = 0;
    uint32_t movie_x_size = 0;
    uint32_t movie_y_half = 0;
    uint32_t movie_y = 0;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    uint8_t volume = 7;
    bool use32bit = true;
    bool isVfsMovie = false;

    // VDP2 32-bit state
    void* cpk_vdp2_dst = nullptr;

    // VDP1 16-bit state
    int32_t spriteId = -1;

    uint32_t cpk_work[CPK_24WORK_DSIZE]; // 24-bit work is larger, covers both
    uint32_t* cpk_ring_buf = nullptr;
    uint32_t* cpk_decode_buf = nullptr;

    static inline volatile bool timer0Fired = false;
    static inline volatile bool vblankFired = false;

    static void Timer0Handler() { timer0Fired = true; }
    static void VblankHandler() { vblankFired = true; }

    static void VblankDispatch()
    {
        slGetStatus();
        SRL::Core::OnVblank.Invoke();
    }

    static inline void SetScuFunc(uint32_t num, void(*handler)())
    {
        typedef void (*SysSetUint)(uint32_t, void*);
        SysSetUint fn = *(SysSetUint*)0x6000300;
        fn(num, (void*)handler);
    }

    static inline void ChgScuMask(uint32_t andMask, uint32_t orMask)
    {
        typedef void (*SysChgIm)(uint32_t, uint32_t);
        SysChgIm fn = *(SysChgIm*)0x6000344;
        fn(andMask, orMask);
    }

    static inline void TimerSetCompare(uint32_t value)
    {
        *(volatile uint32_t*)0x25fe0090 = value;
    }

    static inline void TimerSetMode(uint32_t mode)
    {
        *(volatile uint32_t*)0x25fe0098 = mode;
    }

    void Setup()
    {
        CPK_Init();

        SRL::Core::OnVblank += VblankHandler;
        SRL::Core::OnVblank += CPK_VblIn;
        switch (DriverLocation)
        {
            case 0:
                cpk_ring_buf = (uint32_t*)(new uint8_t[RING_BUF_SIZ]);
                break;
            case 1:
                cpk_ring_buf = (uint32_t*)(lwnew uint8_t[RING_BUF_SIZ]);
                break;
            case 2:
                cpk_ring_buf = (uint32_t*)(cartnew uint8_t[RING_BUF_SIZ]);
                break;
        }
        //cpk_ring_buf = (uint32_t*)(lwnew uint8_t[RING_BUF_SIZ]);
        cpk_decode_buf = nullptr;
    }

    void Teardown()
    {
        SRL::Core::OnVblank -= VblankHandler;
        SRL::Core::OnVblank -= CPK_VblIn;

        if (cpk_decode_buf) { delete[] cpk_decode_buf; cpk_decode_buf = nullptr; }
        if (cpk_ring_buf) { delete[] (uint8_t*)cpk_ring_buf; cpk_ring_buf = nullptr; }

        // Disable Timer0 interrupt
        ChgScuMask(0xFFFFFFFF, INT_MSK_TIM0);
        TimerSetMode(0);

        // Fully reinitialize the system to restore VDP2 text, vblank handler, etc.
        SRL::Core::Initialize(SRL::Types::HighColor::Colors::Blue);
    }

    void RecalcVDP2Destination()
    {
        int32_t cx = (int32_t)(screenWidth - movie_x) / 2 + offsetX;
        int32_t cy = (int32_t)(screenHeight - movie_y) / 2 + offsetY;
        if (cx < 0) cx = 0;
        if (cy < 0) cy = 0;
        if (cx + (int32_t)movie_x > (int32_t)screenWidth) cx = screenWidth - movie_x;
        if (cy + (int32_t)movie_y > (int32_t)screenHeight) cy = screenHeight - movie_y;

        cpk_vdp2_dst = (uint8_t*)(VDP2_VRAM_A0 +
            4 * (512 * cy + cx) +
            4 * 512 * (512 / 2 - screenHeight) / 2);
    }

    void SetupMovieLayout(uint32_t width, uint32_t height)
    {
        movie_x = width;
        movie_y = height;
        movie_y_half = movie_y / 2;
        offsetX = 0;
        offsetY = 0;

        screenWidth = 320;
        if (height > 240)
            screenHeight = 256;
        else if (height > 224)
            screenHeight = 240;
        else
            screenHeight = 224;

        if (use32bit)
        {
            movie_x_size = movie_x * 4;

            uint16_t tvMode;
            if (screenHeight == 256) tvMode = TV_320x256;
            else if (screenHeight == 240) tvMode = TV_320x240;
            else tvMode = TV_320x224;

            SRL::TV::TVOff();
            slInitSystem(tvMode, SRL::VDP1::Textures->SglPtr(), 1);
            slIntFunction(VblankDispatch);

            slBitMapNbg0(COL_TYPE_1M, BM_512x256, (void*)VDP2_VRAM_A0);
            slColRAMMode(CRM16_1024);
            slScrAutoDisp(NBG0ON);
            slScrCycleSet(CYCPAT_BM_READ, CYCPAT_BM_READ, CYCPAT_BM_READ, CYCPAT_BM_READ);
            slScrPosNbg0(toFIXED(0), toFIXED((512 / 2 - screenHeight) / 2));
            slVRAMMode((uint16_t)0);

            SetScuFunc(INT_SCU_TIM0, Timer0Handler);
            TimerSetCompare(screenHeight / 2 + 2);
            TimerSetMode(0x101);
            ChgScuMask(~INT_MSK_TIM0, INT_MSK_NULL);

            SRL::TV::TVOn();
            RecalcVDP2Destination();
        }
        else
        {
            // 16-bit mode: use VDP1 sprite
            movie_x_size = movie_x * 2;

            SRL::TV::TVOff();
            slInitSystem((uint16_t)SRL::TV::Resolution, SRL::VDP1::Textures->SglPtr(), 1);
            slIntFunction(VblankDispatch);
            SRL::TV::TVOn();

            spriteId = SRL::VDP1::TryAllocateTexture(movie_x, movie_y, SRL::CRAM::TextureColorMode::RGB555, 0);
        }
    }

    void TransferFrame32(CpkHn cpk)
    {
        uint8_t* src = (uint8_t*)cpk_decode_buf;
        uint8_t* dst = (uint8_t*)cpk_vdp2_dst;

        timer0Fired = false;
        while (!timer0Fired) { CPK_Task(cpk); }
        vblankFired = false;

        *(volatile uint32_t*)CYCLE_A_REG = CYCPAT_CPU_RW;
        for (uint32_t i = 0; i < movie_y_half; i++)
        {
            DMA_ScuMemCopy(dst, src, movie_x_size);
            src += movie_x_size;
            dst += 2048;
        }
        *(volatile uint32_t*)CYCLE_A_REG = CYCPAT_BM_READ;

        while (!vblankFired) { CPK_Task(cpk); }

        *(volatile uint32_t*)CYCLE_B_REG = CYCPAT_CPU_RW;
        for (uint32_t i = movie_y_half; i < movie_y; i++)
        {
            DMA_ScuMemCopy(dst, src, movie_x_size);
            src += movie_x_size;
            dst += 2048;
        }
        *(volatile uint32_t*)CYCLE_B_REG = CYCPAT_BM_READ;
    }

    void TransferFrame16()
    {
        if (spriteId >= 0)
        {
            DMA_ScuMemCopy(SRL::VDP1::Textures[spriteId].GetData(), cpk_decode_buf, movie_x_size * movie_y);
        }
    }

    CpkHn CreateMovieHandle(GfsHn gfs)
    {
        uint32_t workSize = use32bit ? CPK_24WORK_BSIZE : CPK_15WORK_BSIZE;

        CpkCreatePara para;
        CPK_PARA_WORK_ADDR(&para) = cpk_work;
        CPK_PARA_WORK_SIZE(&para) = workSize;
        CPK_PARA_BUF_ADDR(&para) = cpk_ring_buf;
        CPK_PARA_BUF_SIZE(&para) = RING_BUF_SIZ;
        CPK_PARA_PCM_ADDR(&para) = (uint16_t*)0x25a20000;
        CPK_PARA_PCM_SIZE(&para) = PCM_SIZE;

        CpkHn cpk = CPK_CreateGfsMovie(&para, gfs);
        if (cpk == NULL) return NULL;

        CPK_SetColor(cpk, use32bit ? CPK_COLOR_24BIT : CPK_COLOR_15BIT);
        CPK_PreloadHeader(cpk);

        CpkHeader* header = CPK_GetHeader(cpk);
        SetupMovieLayout(header->width, header->height);

        if (cpk_decode_buf) delete[] cpk_decode_buf;
        int32_t bufSize = use32bit ? (header->width * header->height) : (header->width * header->height / 2);
        switch (TempLocation)
        {
            case 0:
                cpk_decode_buf = new uint32_t[bufSize];
                break;
            case 1:
                cpk_decode_buf = lwnew uint32_t[bufSize];
                break;
            case 2:
                cpk_decode_buf = cartnew uint32_t[bufSize];
                break;
        }
        //cpk_decode_buf = new uint32_t[bufSize];

        int32_t stride = header->width * (use32bit ? 4 : 2);
        CPK_SetDecodeAddr(cpk, cpk_decode_buf, stride);
        CPK_SetVolume(cpk, volume);

        return cpk;
    }

    Result PlayItemWithControls(const MenuItem& item)
    {
        if (item.offset == 0)
            return PlayGfs(item);
        else
            return PlayStm(item);
    }

    Result PlayGfs(const MenuItem& item)
    {
        isVfsMovie = false;
        int32_t fid = GFS_NameToId((int8_t*)item.name);
        GfsHn gfs = GFS_Open(fid);
        if (gfs == NULL) return Result::Next;

        CpkHn cpk = CreateMovieHandle(gfs);
        if (cpk == NULL) { GFS_Close(gfs); return Result::Next; }

        CPK_Start(cpk);
        Result result = RunPlaybackLoop(cpk);

        CPK_DestroyGfsMovie(cpk);
        GFS_Close(gfs);
        return result;
    }

    Result PlayStm(const MenuItem& item)
    {
        isVfsMovie = true;
        GfsHn gfs = GFS_Open(currentFileFid);
        if (gfs == NULL) return Result::Next;

        int32_t startFad = GFS_GetFad(currentFileFid, item.offset);
        int32_t sectorCount = item.size / 2048;
        gfs->flow.finfo.finfo.fad = startFad;
        gfs->flow.finfo.finfo.size = item.size;
        gfs->flow.finfo.nsct = sectorCount;
        gfs->flow.finfo.sctsz = 2048;
        gfs->flow.finfo.lstrm = item.size - ((sectorCount - 1) * 2048);
        gfs->flow.dtsrc.rsrc.cd.sfad = startFad;
        gfs->flow.dtsrc.rsrc.cd.efad = startFad + sectorCount;

        CpkHn cpk = CreateMovieHandle(gfs);
        if (cpk == NULL) { GFS_Close(gfs); return Result::Next; }

        CPK_Start(cpk);
        Result result = RunPlaybackLoop(cpk);

        CPK_DestroyGfsMovie(cpk);
        GFS_Close(gfs);
        return result;
    }

    Result RunPlaybackLoop(CpkHn cpk)
    {
        bool paused = false;
        int32_t noFrameCount = 0;
        using Btn = SRL::Input::Digital::Button;

        while (1)
        {
            CPK_Task(cpk);

            if (CPK_GetPlayStatus(cpk) == CPK_STAT_PLAY_END)
                return Result::Ended;
            if (isVfsMovie && noFrameCount > 30)
                return Result::Ended;

            if (!use32bit && spriteId >= 0)
            {
                SRL::Scene2D::DrawSprite(
                    spriteId,
                    SRL::Math::Vector3D(SRL::Math::Fxp((int16_t)offsetX), SRL::Math::Fxp((int16_t)offsetY), SRL::Math::Fxp((int16_t)500)),
                    SRL::Math::Vector2D(1.0, 1.0),
                    SRL::Scene2D::ZoomPoint::Center);
            }

            SRL::Core::Synchronize();

            // C = quit
            if (pad.WasPressed(Btn::C))
                return Result::Quit;

            // X = previous
            if (pad.WasPressed(Btn::X))
                return Result::Previous;

            // Z = next
            if (pad.WasPressed(Btn::Z))
                return Result::Next;

            // Y = restart current
            if (pad.WasPressed(Btn::Y))
                return Result::Restart;

            // Start = toggle 16/32-bit (requires restart)
            if (pad.WasPressed(Btn::START))
            {
                use32bit = !use32bit;
                return Result::Restart;
            }

            // B = play/pause
            if (pad.WasPressed(Btn::B))
            {
                if (paused)
                {
                    CPK_Pause(cpk, CPK_PAUSE_OFF);
                    paused = false;
                }
                else
                {
                    CPK_Pause(cpk, CPK_PAUSE_ON_AT_ONCE);
                    paused = true;
                }
            }

            // L = volume down, R = volume up
            if (pad.WasPressed(Btn::L) && volume > 0)
            {
                volume--;
                CPK_SetVolume(cpk, volume);
            }
            if (pad.WasPressed(Btn::R) && volume < 7)
            {
                volume++;
                CPK_SetVolume(cpk, volume);
            }

            // D-pad = move video
            if (movie_x < screenWidth || movie_y < screenHeight)
            {
                bool moved = false;
                if (pad.IsHeld(Btn::Left))  { offsetX--; moved = true; }
                if (pad.IsHeld(Btn::Right)) { offsetX++; moved = true; }
                if (pad.IsHeld(Btn::Up))    { offsetY--; moved = true; }
                if (pad.IsHeld(Btn::Down))  { offsetY++; moved = true; }
                if (moved && use32bit) RecalcVDP2Destination();
            }

            if (CPK_IsDispTime(cpk) == TRUE)
            {
                noFrameCount = 0;
                if (use32bit)
                {
                    TransferFrame32(cpk);
                    OnOverlay.Invoke(*this);
                }
                else
                {
                    TransferFrame16();
                }
                CPK_CompleteDisp(cpk);
            }
            else
            {
                noFrameCount++;
            }
        }
    }
};

AudioStream* pcmstream = nullptr;

int main()
{
    SRL::Core::Initialize(SRL::Types::HighColor::Colors::Blue);
    // Initialize gamepad
    SRL::Input::Digital pad(0);
    //TurnOff68K();
    int32_t offset = 0;
    int32_t highlight = 0;
    //LoadVFS("AUTORUN.VFS");
    //LoadSegaDriver("SDDRVS.TSK");
    LoadISO9660(".");
    // (SndPlayEX init removed)
    //SRL::Debug::PrintClearScreen();
    while (1)
    {
        for (int i = 4; i < 30; i++)
        {
            SRL::Debug::PrintClearLine(i); //PrintClearScreen DOESN'T WORK!
        }
        if (TurboMode)
        {
            FrameSetTempo(tempo); //keep turbo on through repeats
        }
        switch (ButtonPressed(pad, true))
        {
            case SRL::Input::Digital::Button::Up:
                if (STARTHELD(pad))
                {
                    menuItems[validFileList[highlight + offset]].type--;
                }
                else
                {
                    MoveHighlight(-1, validFiles, &highlight, &offset);
                }
                break;
            case SRL::Input::Digital::Button::Down:
                if (STARTHELD(pad))
                {
                    menuItems[validFileList[highlight + offset]].type++;
                }
                else
                {
                    MoveHighlight(1, validFiles, &highlight, &offset);
                }
                break;
            case SRL::Input::Digital::Button::Left:
                if (STARTHELD(pad))
                {
                    menuItems[validFileList[highlight + offset]].type -= 10;
                }
                else
                {
                    MoveHighlight(-10, validFiles, &highlight, &offset);
                }
                break;
            case SRL::Input::Digital::Button::Right:
                if (STARTHELD(pad))
                {
                    menuItems[validFileList[highlight + offset]].type += 10;
                }
                else
                {
                    MoveHighlight(10, validFiles, &highlight, &offset);
                }
                break;
            case SRL::Input::Digital::Button::A:
                if (menuItems[validFileList[highlight + offset]].type == TYPE_CINEPAK)
                {
                    //CinepakVDP2Player* player;
                    if (STARTHELD(pad))
                    {
                        CinepakVDP2Player* player = new CinepakVDP2Player(pad, menuItems, fileCount, highlight + offset);
                        player->Play();
                        delete player;
                    }
                    else
                    {
                        CinepakVDP2Player* player = new CinepakVDP2Player(pad, menuItems, fileCount, -1);
                        player->Play();
                        delete player;
                    }
                    //delete player;
                }
                else if (menuItems[validFileList[highlight + offset]].type == TYPE_CDDA)
                {
                    if ((highlight + offset) == 0)
                    {
                        LoadCDDA(1, STARTHELD(pad));
                    }
                    LoadCDDA(highlight + offset, STARTHELD(pad));
                }
                else if (LoadFile(&menuItems[validFileList[highlight + offset]], false) >= 0)
                {
                    highlight = 0;
                    offset = 0;
                }
                break;
            case SRL::Input::Digital::Button::B:
                if (STARTHELD(pad))
                {
                    if (MusicPlaying)
                    {
                        PlayPauseMusic(true);
                    }
                    TurnOff68K();
                    memset((void*)0x25A00000, 0, 0x80000); //stop music
                    MusicPlaying = false;
                    MusicType = 0;
                }
                else
                {
                    MusicPlaying = PlayPauseMusic(MusicPlaying);
                }
                break;
            case SRL::Input::Digital::Button::C:
                if (STARTHELD(pad))
                {
                    DirLevel = 0;
                    SRL::Cd::ChangeDir((char*)nullptr);
                    LoadISO9660(".");
                }
                else if (LoadFile(&menuItems[1], false) >= 0)
                {
                    highlight = 0;
                    offset = 0;
                }
                break;
            case SRL::Input::Digital::Button::L:
                if (STARTHELD(pad))
                {
                    SetVolume(-10, true);
                }
                else
                {
                    SetVolume(-1, true);
                }
                break;
            case SRL::Input::Digital::Button::R:
                if (STARTHELD(pad))
                {
                    SetVolume(10, true);
                }
                else
                {
                    SetVolume(1, true);
                }
                break;
            case SRL::Input::Digital::Button::X:
                if (STARTHELD(pad))
                {
                    SetTempo(-512, true);
                }
                else
                {
                    TurboMode = !TurboMode;
                    if (!TurboMode)
                    {
                        FrameSetTempo(0);
                    }
                }
                break;
            case SRL::Input::Digital::Button::Y:
                if (STARTHELD(pad))
                {
                    SetTempo(512, true);
                }
                break;
            case SRL::Input::Digital::Button::Z:
                if (STARTHELD(pad))
                {
                    DisplayHelp(pad);
                }
                else
                {
                    DisplaySettings(offset + highlight, pad);
                }
                break;
        }
        if (!STARTHELD(pad))
        {
            if (pad.WasPressed(SRL::Input::Digital::Button::Y))
            {
                TurboMode = true;
                SetTempo(0, true); //turns turbo on
            }
            if (pad.WasReleased(SRL::Input::Digital::Button::Y))//cant turn toggle turbo off with Y
            {
                //SetTempo(0, true);
                FrameSetTempo(0);
                TurboMode = false;
            }
        }
        SRL::Debug::Print(0, 1, "    S.I.M.P. v1.0 - Start+Z for controls");
        SRL::Debug::Print(1,  3, "                                      ");
        /*SRL::Debug::Print(4, 0, "S.I.M.P. 1.0 (%dH/%dL free)              ", //driverEnd == nullptr,
        SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam),// SRL::Memory::GetSize(SRL::Memory::Zone::HWRam),
        SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam));//, SRL::Memory::GetSize(SRL::Memory::Zone::LWRam));*/
        DrawFileList(highlight, offset);
        //PrintTime(4, 1);
        if (pcmstream != nullptr)
        {
            pcmstream->Task();
        }
        SRL::Core::Synchronize();
        if (pcmstream != nullptr)
        {
            PCM_VblIn();
        }
        
    }
    return 0;
}

void DisplayHelp(SRL::Input::Digital& pad)
{

    while (true)
    {



        for (int i = 4; i < 30; i++)
        {
            SRL::Debug::PrintClearLine(i);
        }

        SRL::Debug::Print(0,  1, " SIMP - Saturn Intelligent Media Player ");
        SRL::Debug::Print(1,  2, "                                      ");
        SRL::Debug::Print(1,  3, "Controls:          (Start held)       ");
        SRL::Debug::Print(1,  4, "A: Select / Play   Alt. Select/Play   ");
        SRL::Debug::Print(1,  5, "B: Pause/Resume    Stop               ");
        SRL::Debug::Print(1,  6, "C: Directory Up    Go to Root Dir     ");
        SRL::Debug::Print(1,  7, "Up: Up 1 File      Change Type -1     ");
        SRL::Debug::Print(1,  8, "Down: Down 1 File  Change Type +1     ");
        SRL::Debug::Print(1,  9, "Left: Up 10        Change Type -10    ");
        SRL::Debug::Print(1, 10, "Right: Down 10     Change Type +10    ");
        SRL::Debug::Print(1, 11, "L: Volume Down     Volume Down -10    ");
        SRL::Debug::Print(1, 12, "R: Volume Up       Volume Up +10      ");
        SRL::Debug::Print(1, 13, "X: Toggle Turbo    Change Tempo -512  ");
        SRL::Debug::Print(1, 14, "Y: Hold for Turbo  Change Tempo +512  ");
        SRL::Debug::Print(1, 15, "Z: Settings        Help/Controls      ");
        SRL::Debug::Print(1, 17, "Settings menu controls:               ");
        SRL::Debug::Print(1, 18, "Up/Down: Select Setting               ");
        SRL::Debug::Print(1, 19, "Left/Right: Change Setting  -/+ 1     ");
        SRL::Debug::Print(1, 20, "L/R: Change Setting -/+ 100           ");
        SRL::Debug::Print(1, 21, "X/Y: Change Setting -/+ 10000         ");
        SRL::Debug::Print(1, 22, "Z/C: Change Setting -/+ 1000000       ");
        SRL::Debug::Print(1, 24, "Start: Hold to change at x10 speed    ");
        SRL::Debug::Print(1, 25, "A: Select setting starting with *     ");
        SRL::Debug::Print(1, 26, "B: Exit settings menu                 ");
        SRL::Debug::Print(1, 30, "Press A or B to return to main menu   ");

        const auto pressed = ButtonPressed(pad, true);
        if (pressed == SRL::Input::Digital::Button::A || pressed == SRL::Input::Digital::Button::B)
        {
            break;
        }

        if (pcmstream != nullptr)
        {
            pcmstream->Task();
        }
        SRL::Core::Synchronize();
        if (pcmstream != nullptr)
        {
            PCM_VblIn();
        }
    }
}

void DisplaySettings(int32_t selectedItem, SRL::Input::Digital& pad)
{
    int32_t selectedSetting = 0;
    while (true)
    {
        int32_t start_value = 1;
        for (int i = 4; i < 30; i++)
        {
            SRL::Debug::PrintClearLine(i); //PrintClearScreen DOESN'T WORK!
        }
        if (TurboMode)
        {
            SetTempo(0, true); //keep turbo on through repeats
        }
        SRL::Debug::Print(4, 1, "Total RAM Free: %d/%d        ",
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam) +
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam) +
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam),
             SRL::Memory::GetSize(SRL::Memory::Zone::HWRam) +
             SRL::Memory::GetSize(SRL::Memory::Zone::LWRam) +
             SRL::Memory::GetSize(SRL::Memory::Zone::CartRam));
        SRL::Debug::Print(4, 2, "(%dH/%dL/%dC free)        ",
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam),
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam),
             SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam));
        for (int32_t i = 0; i < (int32_t)OSDMenuItems.size(); i++)
        {
            bool stringname = (IncDecOSD(OSDMenuItems[i].type, 0, true, selectedItem) >= OSDMenuItems[i].stringmin 
            && IncDecOSD(OSDMenuItems[i].type, 0, true, selectedItem) <= OSDMenuItems[i].stringmax);
            SRL::Debug::Print(1, 4 + i, "%s%s", (i == selectedSetting) ? "-> " : "   ", 
            stringname ? OSDMenuItems[i].name : OSDMenuItems[i].intname);
            if (!OSDMenuItems[i].nodata)
            {
                if (stringname)
                {
                    SRL::Debug::Print(28, 4 + i, "%s", OSDMenuItems[i].strings[IncDecOSD(OSDMenuItems[i].type, 0, true, selectedItem) - OSDMenuItems[i].stringmin]);
                }
                else
                {
                    SRL::Debug::Print(28, 4 + i, "%d", IncDecOSD(OSDMenuItems[i].type, 0, true, selectedItem));
                }
            }
        }


        if (validFiles > 0)
        {
            SRL::Debug::Print(4, OSDMenuItems.size() + START_LINES + 2, "File %d, type: %d%s",
            selectedItem + 1, menuItems[validFileList[selectedItem]].type - TYPE_BASE,
            GetLongTypeString(menuItems[validFileList[selectedItem]].type));
            SRL::Debug::Print(4, OSDMenuItems.size() + START_LINES + 3, "Size: %d bytes, Offset: %d",
            menuItems[validFileList[selectedItem]].size, menuItems[validFileList[selectedItem]].offset);
            //SRL::Debug::Print(4, OSDMenuItems.size() + START_LINES + 4, "Volume: %d, Tempo: %d", volume, tempo);
        }


        if (STARTHELD(pad))
        {
            start_value = 10;
        }
        else
        {
            start_value = 1;
        }
        switch (ButtonPressed(pad, true))
        {
            case SRL::Input::Digital::Button::Up:
                selectedSetting = std::max((int32_t)0, selectedSetting - 1);
                break;
            case SRL::Input::Digital::Button::Down:
                selectedSetting = std::min((int32_t)(OSDMenuItems.size() - 1), selectedSetting + 1);
                break;
            case SRL::Input::Digital::Button::Left:
                IncDecOSD(OSDMenuItems[selectedSetting].type, -1 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::Right:
                IncDecOSD(OSDMenuItems[selectedSetting].type, 1 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::L:
                IncDecOSD(OSDMenuItems[selectedSetting].type, -100 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::R:
                IncDecOSD(OSDMenuItems[selectedSetting].type, 100 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::X:
                IncDecOSD(OSDMenuItems[selectedSetting].type, -10000 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::Y:
                IncDecOSD(OSDMenuItems[selectedSetting].type, 10000 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::Z:
                IncDecOSD(OSDMenuItems[selectedSetting].type, -1000000 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::C:
                IncDecOSD(OSDMenuItems[selectedSetting].type, 1000000 * start_value, false, selectedItem);
                break;
            case SRL::Input::Digital::Button::B:
                return;
            case SRL::Input::Digital::Button::A:
                switch (OSDMenuItems[selectedSetting].type)
                {
                    case OSD_REMOVEDRIVER:
                        delete[] vgmdriver;
                        vgmdriver = nullptr;
                        driverSize = 0;
                        delete[] segaDriver;
                        segaDriver = nullptr;
                        segaDriverSize = 0;
                        break;
                    case OSD_SETCOLOR:
                        switch (IncDecOSD(OSD_SETCOLOR, 0, true, selectedItem))
                        {
                            case 0: //custom
                                SRL::Core::Initialize(SRL::Types::HighColor::FromRGB555(colorR, colorG, colorB));
                                break;
                            case 1: //black
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Black);
                                break;
                            case 2: //red
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Red);
                                break;
                            case 3: //green
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Green);
                                break;
                            case 4: //blue
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Blue);
                                break;
                            case 5: //yellow
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Yellow);
                                break;
                            case 6: //magenta
                                SRL::Core::Initialize(SRL::Types::HighColor::Colors::Magenta);
                                break;
                        }
                        break;
                    /*case OSD_RELOADCD:
                        SRL::Cd::ChangeDir((char*)nullptr);
                        DirLevel = 0;
                        LoadISO9660(".");
                        return;*/
                }
                break;
        }

        if (pcmstream != nullptr)
        {
            pcmstream->Task();
        }
        SRL::Core::Synchronize();
        if (pcmstream != nullptr)
        {
            PCM_VblIn();
        }
    }

}


int32_t IncDecOSD(OSDItem type, int32_t value, bool noset, int32_t selectedItem)
{
    switch (type)
    {
        case OSD_RED:
            if (!noset)
            {
                int temp = colorR + value;
                colorR = std::max(0, std::min(temp, 31)); //cant use int32_t
            }
            return colorR;
        case OSD_GREEN:
            if (!noset)
            {
                int temp = colorG + value;
                colorG = std::max(0, std::min(temp, 31));
            }
            return colorG;
        case OSD_BLUE:
            if (!noset)
            {
                int temp = colorB + value;
                colorB = std::max(0, std::min(temp, 31));
            }
            return colorB;
        case OSD_REPEATFRAMES:
            if (!noset)
            {
                int temp = repeatFrames + value;
                repeatFrames = std::max(0, std::min(temp, 0xFF));
            }
            return repeatFrames;
        case OSD_REPEAT:
            if (!noset)
            {
                int temp = repeatDelay + value;
                repeatDelay = std::max(0, std::min(temp, 0xFFFF));
            }
            return repeatDelay;
        case OSD_TURBOSPEED:
            if (!noset)
            {
                SetTempo(value, true);
            }
            return tempo;
        case OSD_DRIVERLOAD:
            if (!noset)
            {
                if (value < 0)
                {
                    LoadDriverManual = false;
                }
                else if (value > 0)
                {
                    LoadDriverManual = true;
                }
            }
            return LoadDriverManual;
        case OSD_DRIVERLOC:
            if (!noset)
            {
                int temp = DriverLocation + value;
                DriverLocation = std::max(0, std::min(temp, 2));
            }
            return DriverLocation;
        case OSD_TEMPLOC:
            if (!noset)
            {
                int temp = TempLocation + value;
                TempLocation = std::max(0, std::min(temp, 2));
            }
            return TempLocation;
        case OSD_MENULOC:
            if (!noset)
            {
                int temp = MenuLocation + value;
                MenuLocation = std::max(0, std::min(temp, 2));
            }
            return MenuLocation;
        case OSD_ABSOLUTE:
            if (!noset)
            {
                menuItems[selectedItem].type += value;
            }
            return (int32_t)menuItems[selectedItem].type;
        case OSD_RAMBUFFER:
            if (!noset)
            {
                int temp = RingBufferSize + value;
                RingBufferSize = std::max(0, std::min(temp, 0x100000));
            }
            return RingBufferSize;
        case OSD_SOUNDBUFFER:
            if (!noset)
            {
                int temp = SoundBufferSize + value;
                SoundBufferSize = std::max(0, std::min(temp, 0x80000));
            }
            return SoundBufferSize;
        case OSD_SOUNDBUFLOC:
            if (!noset)
            {
                int temp = SoundBufferOffset + value;
                SoundBufferOffset = std::max(0, std::min(temp, 0x80000));
            }
            return SoundBufferOffset;
        case OSD_PAUSEBUFFER:
            if (!noset)
            {
                int temp = PauseBufferSize + value;
                PauseBufferSize = std::max(0, std::min(temp, 0x80000));
            }
            return PauseBufferSize;
        /*case OSD_PANL:
            if (!noset)
            {
                int temp = panL + value;
                panL = std::max(-15, std::min(temp, 15));
            }
            return panL;
        case OSD_PANR:
            if (!noset)
            {
                int temp = panR + value;
                panR = std::max(-15, std::min(temp, 15));
            }
            return panR;*/
        case OSD_BANK:
            if (!noset)
            {
                int temp = bank + value;
                bank = std::max(0, std::min(temp, 15)); //probably correct
            }
            return bank;
        case OSD_SETCOLOR:
            if (!noset)
            {
                int temp = MenuColor + value;
                MenuColor = std::max(0, std::min(temp, 6));
            }
            return MenuColor;
        case OSD_VOLUME:
            if (!noset)
            {
                SetVolume(value, true);
            }
            return volume;
    }
    return 0;
}

void TurnOff68K()
{
    *(uint8_t *)(0x2010001F)=7;
	for(int w=0;w<500;w++)
	{
		asm("nop\n"); // waste time
	}
    //EVERYTHING AFTER THIS IS FOR THE VGM DRIVER
	*(uint16_t *)(0x5B00400)=0x20F;

	for(int w=0;w<500;w++)
	{
		asm("nop\n"); // waste time
	}

	*(uint16_t *)(0x5B00402)=0x3F;

	for(int w=0;w<500;w++)
	{
		asm("nop\n"); // waste time
	}
}

void TurnOn68K()
{
	*(uint8_t *)(0x2010001F)=6;

	while(*(uint16_t *)(0x25A0010C)!=0)
	{
		for(int w=0;w<500;w++)
		{
			asm("nop\n"); // waste time
		}
	}
}

int32_t LoadISO9660(const char* fileName, int32_t baseOffset, bool NotAllFilesValid, bool isLastDir)
{
    SRL::Cd::File file(fileName);
    uint8_t* buffer = nullptr;
    switch (TempLocation)
    {
        case 0:
            if ((size_t)file.Size.Bytes > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam))
            {
                return -1;
            }
            buffer = new uint8_t[file.Size.Bytes];
            break;
        case 1:
            if ((size_t)file.Size.Bytes > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam))
            {
                return -1;
            }
            buffer = lwnew uint8_t[file.Size.Bytes];
            break;
        case 2:
            if ((size_t)file.Size.Bytes > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam))
            {
                return -1;
            }
            buffer = cartnew uint8_t[file.Size.Bytes];
            break;
    }
    file.LoadBytes(baseOffset, file.Size.Bytes, buffer); //if dir is too big for hwram SRL wont work with it anyway
    delete[] validFileList;
    validFiles = 0;
    delete[] menuItems;
    fileCount = 0;
    //int32_t fcount = 0;
    //std::vector<int32_t> temp_valid;
    for (int32_t i = 0; i < file.Size.Bytes; i += 2048)
    {
        int32_t j = 0;
        while (buffer[i+j] && j < 2048)
        {
            fileCount++;
            j += buffer[i+j];
        }
    }
    file.Close();
    SRL::Cd::ChangeDir(fileName);
    switch (MenuLocation)
    {
        case 0:
            if ((fileCount * sizeof(MenuItem)) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam))
            {
                return -1;
            }
            menuItems = new MenuItem[fileCount];
            validFileList = new uint32_t[fileCount];
            break;
        case 1:
            if ((fileCount * sizeof(MenuItem)) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam))
            {
                return -1;
            }
            menuItems = lwnew MenuItem[fileCount];
            validFileList = lwnew uint32_t[fileCount];
            break;
        case 2:
            if ((fileCount * sizeof(MenuItem)) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam))
            {
                return -1;
            }
            menuItems = cartnew MenuItem[fileCount];
            validFileList = cartnew uint32_t[fileCount];
            break;
    }
    int32_t fcount = 0;
    for (int32_t i = 0; i < file.Size.Bytes; i += 2048)
    {
        int32_t j = 0;
        while (buffer[i+j] && j < 2048)
        {
            //memcpy(menuItems[fcount].name, &buffer[i+j+33], buffer[i+j+32]);
            menuItems[fcount].offset = 0;
            //menuItems[fcount].type = (buffer[i+j+25] & 2) ? TYPE_DIR :
            //GetExtType(reinterpret_cast<char*>(&menuItems[fcount].name));
            if (buffer[i+j+25] & 2)
            {
                menuItems[fcount].type = TYPE_DIR;
            }
            else
            {
                char* strsemi = strchr(reinterpret_cast<char*>(&buffer[i+j+33]), ';');
                if (strsemi == nullptr)
                {
                    strsemi = reinterpret_cast<char*>(&buffer[i+j+33]) + buffer[i+j+32];
                }
                *std::min(strsemi, reinterpret_cast<char*>(&buffer[i+j+33]) + buffer[i+j+32]) = 0; //remove ;1 from the end of the name
                menuItems[fcount].type = GetExtType(reinterpret_cast<char*>(&buffer[i+j+33]));
            }
            switch (fcount)
            {
                case 0:
                    strcpy(menuItems[fcount].name, ".");
                    break;
                case 1:
                    strcpy(menuItems[fcount].name, "..");
                    break;
                default:
                    strncpy(menuItems[fcount].name, reinterpret_cast<char*>(&buffer[i+j+33]), 
                    std::min(buffer[i+j+32], (uint8_t)63));
                    menuItems[fcount].name[std::min(buffer[i+j+32], (uint8_t)63)] = 0; //safe for . and ..
                    break;
            }

            
            //SRL::Debug::Print(1, fcount, "%d: %s", buffer[i+j+25], menuItems[fcount].name);
            if (!NotAllFilesValid || (menuItems[fcount].type > TYPE_BASE))
            {
                validFileList[validFiles++] = fcount;
            }
            if (vgmdriver == nullptr && menuItems[fcount].type == TYPE_VGM_DRIVER && !LoadDriverManual)
            {
                //VGMDRIVER = menuItems[fcount];
                //VGMDRIVER_OFFSET = fcount;
                SRL::Cd::File file2(menuItems[fcount].name);
                driverSize = file2.Size.Bytes;
                switch (DriverLocation)
                {
                    case 0:
                        vgmdriver = new uint8_t[driverSize];
                        break;
                    case 1:
                        vgmdriver = lwnew uint8_t[driverSize];
                        break;
                    case 2:
                        vgmdriver = cartnew uint8_t[driverSize];
                        break;
                }
                file2.LoadBytes(0, driverSize, vgmdriver);
                file2.Close();
            }
            if (segaDriver == nullptr && menuItems[fcount].type == TYPE_TSK && !LoadDriverManual)
            {
                SRL::Cd::File file2(menuItems[fcount].name);
                segaDriverSize = file2.Size.Bytes;
                switch (DriverLocation)
                {
                    case 0:
                        segaDriver = new uint8_t[segaDriverSize];
                        break;
                    case 1:
                        segaDriver = lwnew uint8_t[segaDriverSize];
                        break;
                    case 2:
                        segaDriver = cartnew uint8_t[segaDriverSize];
                        break;
                }
                file2.LoadBytes(0, segaDriverSize, segaDriver);
                file2.Close();
            }
            memcpy(&menuItems[fcount].size, &buffer[i+j+14], 4);
            //menuItems[fcount].size = *reinterpret_cast<int32_t*>(&buffer[i+j+14]); //crashes, not aligned
            j += buffer[i+j];
            fcount++;
            SRL::Core::Synchronize();
        }
    }
    delete[] buffer;
    lastDirOffsets.clear();
    if (DirLevel == -1)
    {
        DirLevel++;
    }
    else if (isLastDir || !strncmp(fileName, "..", 2))
    {
        DirLevel--;
    }
    else if (strncmp(fileName, ".", 1))
    {
        DirLevel++;
    }
    if (DirLevel == 0)
    {
        menuItems[1].type = TYPE_CDTRACKS;
    }
    currentFile[0] = 0;
    currentFileFid = -1;
    return fcount;
}

int32_t LoadFile(MenuItem* item, bool StartHeld)
{
    char* fileName = currentFile[0] ? currentFile : item->name;
    //Release CD
    if (pcmstream != nullptr)
    {
        delete pcmstream;
        pcmstream = nullptr;
    }
    if (MusicType == TYPE_CDDA && MusicPlaying)
    {
        PlayPauseMusic(true);
        MusicPlaying = false;
    }
    switch (item->type)
    {
        case TYPE_SSF:
            return LoadSSF(item);
        case TYPE_VGM_DRIVER:
        case TYPE_VGM_DRIVER_MANUAL:
            {
                delete[] vgmdriver;
                vgmdriver = nullptr;
                SRL::Cd::File file(fileName);
                driverSize = file.Size.Bytes;
                switch (DriverLocation)
                {
                    case 0:
                        vgmdriver = new uint8_t[driverSize];
                        break;
                    case 1:
                        vgmdriver = lwnew uint8_t[driverSize];
                        break;
                    case 2:
                        vgmdriver = cartnew uint8_t[driverSize];
                        break;
                }
                file.LoadBytes(0, driverSize, vgmdriver);
                file.Close();
                return -1;
            }
        case TYPE_DIR:
            return LoadISO9660(item->name, item->offset, StartHeld);
        case TYPE_DIR_UP:
            return LoadISO9660(".", item->offset, StartHeld, true);
        case TYPE_VFS:
            return LoadVFS(fileName, item->offset, StartHeld);
        case TYPE_LASTDIR:
            return LoadVFS(fileName, item->offset, StartHeld, true);
        case TYPE_VFS2:
            return LoadVFS(fileName, item->offset, StartHeld, false, true);
        case TYPE_CDTRACKS:
            return LoadTOC(StartHeld);
        case TYPE_MOD:
        case TYPE_VGM:
        case TYPE_MDX:
            return LoadDriverSong(fileName, item->offset, item->size, item->type, StartHeld);
        case TYPE_AIFF:
        case TYPE_ADPCM:
            if (currentFile[0])
            {
                pcmstream = new AudioStream(currentFile, item->offset, item->size, currentFileFid);
            }
            else
            {
                pcmstream = new AudioStream(fileName, 0, item->size);
            }
            MusicType = item->type;
            return -1;
        case TYPE_SEQ:
        case TYPE_TON:
        case TYPE_DSPPROG:
        case TYPE_DSPRAM:
        case TYPE_MAP:
            return LoadSSF2File(fileName, item->offset, item->size, item->type, StartHeld);
        case TYPE_TSK:
            return LoadSegaDriver(fileName, item->offset, item->size);
        case TYPE_SSF2:
            return LoadSSF2(fileName, item->offset, item->size, StartHeld);
        //case TYPE_CDDA:
        /*case TYPE_CINEPAK:
            if (!SddrvsLoaded)
            {
                SRL::Sound::Hardware::Initialize();
                SddrvsLoaded = true;
            }
            {
                SRL::Input::Digital cpad(0);
                CinepakVDP2Player* cinepakPlayer = new CinepakVDP2Player(cpad, item, 1, 0); //temp
                cinepakPlayer->Play();
                delete cinepakPlayer;
            }
            return -1;*/
    }
    return -1;
}

int32_t LoadVFS(const char* fileName, int32_t baseOffset, bool AllFilesValid, bool isLastDir, bool isVFS2)
{
    SRL::Cd::File file(fileName);
    VfsHeader header;
    file.LoadBytes(baseOffset, sizeof(VfsHeader), &header);
    int32_t fcount = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&header.file_count) );
    //SRL::Debug::Print(1, 1, "VFS File Count: %d", fcount);
    //int32_t validFiles = 0;
    //*totalFileCount = 0;
    if (header.signature[0] == 'V' && header.signature[1] == 'F' && header.signature[2] == 'S'
         && header.signature[3] == '\0' && fcount > 0)
    {
        bool isFirstLoad = true;
        //freeHwRam = 8000; //test
        fcount +=2; //account for . and .. entries
        delete[] menuItems;
        fileCount = fcount;
        delete[] validFileList;
        switch (MenuLocation)
        {
            case 0:
                if (fcount * sizeof(MenuItem) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam))
                {
                    return -1;
                }
                menuItems = new MenuItem[fcount];
                validFileList = new uint32_t[fcount];
                break;
            case 1:
                if (fcount * sizeof(MenuItem) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam))
                {
                    return -1;
                }
                menuItems = lwnew MenuItem[fcount];
                validFileList = lwnew uint32_t[fcount];
                break;
            case 2:
                if (fcount * sizeof(MenuItem) > SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam))
                {
                    return -1;
                }
                menuItems = cartnew MenuItem[fcount];
                validFileList = cartnew uint32_t[fcount];
                break;
        }
        validFiles = 0;
        uint32_t freeHwRam = 0;
        switch (TempLocation)
        {
            case 0:
                freeHwRam = SRL::Memory::GetFreeSpace(SRL::Memory::Zone::HWRam);
                break;
            case 1:
                freeHwRam = SRL::Memory::GetFreeSpace(SRL::Memory::Zone::LWRam);
                break;
            case 2:
                freeHwRam = SRL::Memory::GetFreeSpace(SRL::Memory::Zone::CartRam);
                break;
        }
        //validFileList->resize(fcount);

        //);
        menuItems[0].type = TYPE_DIR;
        menuItems[0].offset = 0;
        menuItems[0].size = 0;
        strcpy(menuItems[0].name, ".");
        if (!currentFile[0])
        {
            strncpy(currentFile, fileName, 15); //more reliable than checking lastDirOffsets
            currentFile[15] = 0; //menu should not get confused about menu type now
            currentFileFid = file.GetIdentifier();
        }
        if (isLastDir && lastDirOffsets.size() > 0)
        {
            lastDirOffsets.pop_back();
        }
        else
        {
            lastDirOffsets.push_back(baseOffset);
        }
        
        if (lastDirOffsets.size() >= 2)
        {
            menuItems[1].type = TYPE_LASTDIR;
            menuItems[1].offset = lastDirOffsets[lastDirOffsets.size() - 2];
        }
        else
        {
            menuItems[1].type = TYPE_DIR_UP;
            menuItems[1].offset = 0;
        }
        menuItems[1].size = 0;
        strcpy(menuItems[1].name, "..");
        validFileList[validFiles++] = 0;
        validFileList[validFiles++] = 1;
        uint8_t* fileData = nullptr;// = new uint8_t[((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader)];
        VfsFile* files = nullptr;// = fileData != nullptr ? reinterpret_cast<VfsFile*>(fileData + sizeof(VfsHeader)) : nullptr;
        //SRL::Debug::Print(1, 2, "Loading file entries, total %d bytes", 

        int32_t loadedItems = 2;
        int32_t loadedSectors = 0;
        int32_t oldLoadedItems = 2;
        int32_t totalSectors = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&header.directory_sectors) );
        while (loadedItems < fcount)
        {
            if (isFirstLoad)
            {
                if (freeHwRam >= ((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader))
                {
                    delete[] fileData;
                    fileData = nullptr;
                    switch (TempLocation)
                    {
                        case 0:
                            fileData = new uint8_t[((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader)];
                            break;
                        case 1:
                            fileData = lwnew uint8_t[((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader)];
                            break;
                        case 2:
                            fileData = cartnew uint8_t[((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader)];
                            break;
                    }
                    files = reinterpret_cast<VfsFile*>(fileData + sizeof(VfsHeader));
                    file.LoadBytes(baseOffset + loadedSectors, ((fcount - 2) * sizeof(VfsFile)) + sizeof(VfsHeader), fileData);
                    loadedItems = fcount;
                    loadedSectors = totalSectors;
                    isFirstLoad = false;
                }
                else
                {
                    for (uint32_t i = 0; i < AlignmentsWithHeader.size(); i++)
                    {
                        if (freeHwRam >= AlignmentsWithHeader[i].bytes)
                        {
                            delete[] fileData;
                            fileData = nullptr;
                            switch (TempLocation)
                            {
                                case 0:
                                    fileData = new uint8_t[AlignmentsWithHeader[i].bytes];
                                    break;
                                case 1:
                                    fileData = lwnew uint8_t[AlignmentsWithHeader[i].bytes];
                                    break;
                                case 2:
                                    fileData = cartnew uint8_t[AlignmentsWithHeader[i].bytes];
                                    break;
                            }
                            file.LoadBytes(baseOffset + loadedSectors, (AlignmentsWithHeader[i].bytes), fileData);
                            files = reinterpret_cast<VfsFile*>(fileData + sizeof(VfsHeader));
                            loadedItems += AlignmentsWithHeader[i].entries;
                            loadedSectors += AlignmentsWithHeader[i].bytes >> 11; //All bytes values are divisible by 2048, nothing lost
                            isFirstLoad = false;
                            break;
                        }
                    }
                }
            }
            else
            {
                if (freeHwRam >= ((fcount - loadedItems) * sizeof(VfsFile)))
                {
                    delete[] fileData;
                    fileData = nullptr;
                    switch (TempLocation)
                    {
                        case 0:
                            fileData = new uint8_t[(fcount - loadedItems) * sizeof(VfsFile)];
                            break;
                        case 1:
                            fileData = lwnew uint8_t[(fcount - loadedItems) * sizeof(VfsFile)];
                            break;
                        case 2:
                            fileData = cartnew uint8_t[(fcount - loadedItems) * sizeof(VfsFile)];
                            break;
                    }
                    
                    file.LoadBytes(baseOffset + loadedSectors, ((fcount - loadedItems) * sizeof(VfsFile)), fileData);
                    files = reinterpret_cast<VfsFile*>(fileData);
                    loadedItems = fcount;
                    loadedSectors = totalSectors;
                }
                else
                {
                    for (uint32_t i = 0; i < AlignmentsWithoutHeader.size(); i++)
                    {
                        if (freeHwRam >= AlignmentsWithoutHeader[i].bytes)
                        {
                            delete[] fileData;
                            fileData = nullptr;
                            switch (TempLocation)
                            {
                                case 0:
                                    fileData = new uint8_t[AlignmentsWithoutHeader[i].bytes];
                                    break;
                                case 1:
                                    fileData = lwnew uint8_t[AlignmentsWithoutHeader[i].bytes];
                                    break;
                                case 2:
                                    fileData = cartnew uint8_t[AlignmentsWithoutHeader[i].bytes];
                                    break;
                            }
                            
                            file.LoadBytes(baseOffset + loadedSectors, AlignmentsWithoutHeader[i].bytes, fileData);
                            files = reinterpret_cast<VfsFile*>(fileData);
                            loadedItems += AlignmentsWithoutHeader[i].entries;
                            loadedSectors += AlignmentsWithoutHeader[i].bytes >> 11; //All bytes values are divisible by 2048, nothing lost
                            break;
                        }
                    }
                }
            }
            int32_t f = 0;
            for (int32_t i = oldLoadedItems; i < loadedItems; i++)
            {
                menuItems[i].type = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&files[f].stack) );
                menuItems[i].offset = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&files[f].addr) ) + baseOffset;
                menuItems[i].size = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&files[f].size) );
                memcpy(menuItems[i].name, files[f].name, 64);
                f++;
                //SRL::Debug::Print(1, 2, "Name: %s", files[0].name);
                if (AllFilesValid)
                {
                    validFileList[validFiles++] = i;
                    //continue;
                }
                else switch (menuItems[i].type)
                {
                    case TYPE_SSF:
                    case TYPE_MOD:
                    case TYPE_VGM:
                    case TYPE_MDX:
                    case TYPE_AIFF:
                    case TYPE_ADPCM:
                    case TYPE_CINEPAK:
                    case TYPE_VGM_DRIVER_MANUAL:
                    case TYPE_SSF2:
                        validFileList[validFiles++] = i;
                        //validFiles++;
                        break;
                    case TYPE_SEQ:
                    case TYPE_MAP:
                    case TYPE_TSK:
                    case TYPE_TON:
                    case TYPE_DSPPROG:
                    case TYPE_DSPRAM:
                    case TYPE_VFS: //makes things easier
                    case TYPE_VFS2:
                        if (!isVFS2)
                        {
                            validFileList[validFiles++] = i;
                        }
                        break;
                    case TYPE_VGM_DRIVER:
                        if (vgmdriver == nullptr && !LoadDriverManual) //only load the first vgm driver we find
                        {
                            //vgmdriver->clear();
                            driverSize = menuItems[i].size;
                            switch (DriverLocation)
                            {
                                case 0:
                                    vgmdriver = new uint8_t[driverSize];
                                    break;
                                case 1:
                                    vgmdriver = lwnew uint8_t[driverSize];
                                    break;
                                case 2:
                                    vgmdriver = cartnew uint8_t[driverSize];
                                    break;
                            }
                            file.LoadBytes(menuItems[i].offset, driverSize, vgmdriver);
                            //SRL::Debug::Print(1, 1, "VGM Driver Loaded to RAM taking up %d bytes", menuItems[i].size);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (isLastDir)
            {
                DirLevel--;
            }
            else
            {
                DirLevel++;
            }
            oldLoadedItems = loadedItems;
            //validFileList->resize(validFiles);
            delete[] fileData;
        }
        file.Close();
        //*totalFileCount = filecount;

        return fileCount;
    }
    file.Close();
    return 0;
}

int32_t LoadTOC(bool DontWaitBusy)
{
    fileCount = 0;
    delete[] menuItems;
    delete[] validFileList;
    validFiles = 0;
    if (!DontWaitBusy)
    {
        //CDC_CdOpen();
        // wait (bounded) for CDC not-busy
        CdcStat stat;
        int32_t tries = 200;
        while (tries--) {
            CDC_GetCurStat(&stat);
            if ((CDC_STAT_STATUS(&stat) & CDC_STC_MSK) != CDC_ST_BUSY) break;
            for (int32_t w=0;w<500;w++)
            {
                asm("nop\n"); // waste time
            }
        }
    }
    uint32_t toc[102]; //might want to replace this with SRL
    if (CDC_TgetToc(toc) < 0)
    {
        return -1;
    }
    fileCount = ((toc[100] >> 16) & 0xFF) + 1;
    switch (MenuLocation)
    {
        case 0:
            menuItems = new MenuItem[fileCount];
            validFileList = new uint32_t[fileCount];
            break;
        case 1:
            menuItems = lwnew MenuItem[fileCount];
            validFileList = lwnew uint32_t[fileCount];
            break;
        case 2:
            menuItems = cartnew MenuItem[fileCount];
            validFileList = cartnew uint32_t[fileCount];
            break;
    }
    menuItems[0].type = TYPE_CDDA;
    strcpy(menuItems[0].name, ".");
    menuItems[1].type = TYPE_DIR_UP;
    strcpy(menuItems[1].name, "..");
    validFileList[validFiles++] = 0;
    validFileList[validFiles++] = 1;
    for (int32_t i = 2; i < fileCount; i++)
    {
        uint32_t lba = toc[i] & 0xFFFFFF; //all this stuff is useless so its only for display (overwrite old data)
        uint32_t size = (i < fileCount - 1) ? ((toc[i + 1] & 0xFFFFFF) - lba) : (2048 * 75 * 60) - lba; //size of last track is calculated based on max CD size
        menuItems[i].offset = lba << 11; //convert from sectors to bytes
        menuItems[i].size = size;
        menuItems[i].type = TYPE_CDDA;
        snprintf(menuItems[i].name, sizeof(menuItems[i].name), "Track %d", i);
        validFileList[validFiles++] = i;
    }
    DirLevel = -1;
    return fileCount;
}

int32_t DrawFileList(int32_t highlight, int32_t offset)
{
    int32_t number = offset +1;
    //int32_t ctr = 0;
    int32_t fileNumber = offset;
    int32_t numDisplayFiles = DISPLAY_FILES;
    if (numDisplayFiles > validFiles)
    {
        numDisplayFiles = validFiles;
    }
    //SRL::Debug::Print(4, 2, "                                   ");
    SRL::Debug::Print(4, 2, "Files %d to %d of %d               ", offset+1, offset+numDisplayFiles, validFiles);
    for (int32_t i = 0; i < numDisplayFiles; i++)
    {
        SRL::Debug::Print(1, (i << 1) + START_LINES, "%s", (i == highlight) ? "-> " : "");

        SRL::Debug::PrintWithWrap(4, (i << 1) + START_LINES, 4, 36, menuItems[validFileList[i + offset]].name);
        SRL::Debug::Print(37, (i << 1) + START_LINES, "%s", GetFileTypeString(menuItems[validFileList[i + offset]].type));
    }
    if (validFiles > 0)
    {
        SRL::Debug::Print(4, (DISPLAY_FILES << 1) + START_LINES + 2, "File %d, type: %d%s",
        highlight + offset + 1, menuItems[validFileList[highlight + offset]].type - TYPE_BASE,
        GetLongTypeString(menuItems[validFileList[highlight + offset]].type));
        SRL::Debug::Print(4, (DISPLAY_FILES << 1) + START_LINES + 3, "Size: %d bytes, Offset: %d",
        menuItems[validFileList[highlight + offset]].size, menuItems[validFileList[highlight + offset]].offset);
        SRL::Debug::Print(4, (DISPLAY_FILES << 1) + START_LINES + 4, "Volume: %d, Tempo: %d", volume, tempo);
    }
    return highlight + offset;
}

void MoveHighlight(int32_t i, int32_t numFiles, int32_t *highlight, int32_t *offset)
{
	int displayFiles = DISPLAY_FILES;
	// Check that we have greater than ten files to view
	// if not then set the displayed files correctly
	if(displayFiles > numFiles)
		displayFiles = numFiles;
	
	if (*highlight + i < 0) //negative numbers (up by i)
	{
		if (*offset + i < 0)
		{
			int32_t newAbsolute = *offset + *highlight + i;
			if (newAbsolute < 0) newAbsolute = 0;
			*offset = 0;
			*highlight = newAbsolute;
		}
		else
		{
			*offset += i;
		}
	}
	else if (*highlight + i >= displayFiles) //positive numbers (down by i)
	{
		if (*offset + i >= numFiles - displayFiles)
		{
			int32_t newAbsolute = *offset + *highlight + i;
			if (newAbsolute >= numFiles) newAbsolute = numFiles - 1;
			*offset = numFiles - displayFiles;
			*highlight = newAbsolute - *offset;
		}
		else
		{
			*offset += i;
		}
	}
	else
	{
		*highlight += i;
	}
	return;
}

const char* GetFileTypeString(uint32_t type)
{
    switch (type)
    {
        case TYPE_SSF:
        case TYPE_SSF2:
            return "SSF";
        case TYPE_MOD:
            return "MOD";
        case TYPE_VGM:
            return "VGM";
        case TYPE_MDX:
            return "MDX";
        case TYPE_VGM_DRIVER:
        case TYPE_VGM_DRIVER_MANUAL:
            return "DRV";
        case TYPE_DIR:
        case TYPE_DIR_UP:
            return "DIR";
        case TYPE_LASTDIR:
            //return ".."; //for testing
        case TYPE_VFS:
        case TYPE_VFS2:
            return "VFS";
        case TYPE_ADPCM:
            return "ADP";
        case TYPE_AIFF:
            return "AIF";
        case TYPE_CINEPAK:
            return "CPK";
        case TYPE_SEQ:
            return "SEQ";
        case TYPE_TON:
            return "TON";
        case TYPE_DSPPROG:
            return "EXB";
        case TYPE_DSPRAM:
            return "DSP";
        case TYPE_CDDA:
            return "DA";
        case TYPE_CDTRACKS:
            return "TOC";
        case TYPE_MAP:
            return "MAP";
        case TYPE_TSK:
            return "TSK";
        default:
            return "";
    }
}

const char* GetLongTypeString(uint32_t type)
{
    switch (type)
    {
        case TYPE_SSF:
        case TYPE_SSF2:
            return " (SSF)";
        case TYPE_MOD:
            return " (MOD)";
        case TYPE_VGM:
            return " (VGM)";
        case TYPE_MDX:
            return " (MDX)";
        case TYPE_VGM_DRIVER:
        case TYPE_VGM_DRIVER_MANUAL:
            return " (Driver)";
        case TYPE_DIR:
        case TYPE_DIR_UP:
            return " (Directory)";
        case TYPE_LASTDIR:
            //return " (Last Directory)"; //testing
        case TYPE_VFS:
        case TYPE_VFS2:
            return " (VFS)";
        case TYPE_ADPCM:
            return " (ADPCM)";
        case TYPE_AIFF:
            return " (AIFF)";
        case TYPE_CINEPAK:
            return " (Cinepak)";
        case TYPE_SEQ:
            return " (SEQ)";
        case TYPE_TON:
            return " (TON)";
        case TYPE_DSPPROG:
            return " (DSP Program)";
        case TYPE_DSPRAM:
            return " (DSP RAM)";
        case TYPE_CDDA:
            return " (CDDA)";
        case TYPE_CDTRACKS:
            return " (CD Tracks)";
        case TYPE_MAP:
            return " (MAP)";
        case TYPE_TSK:
            return " (Sega Driver)";
        default:
            return "";
    }
}

uint32_t GetExtType(const char* filename)
{
    char* ext = strchr(filename, '.');
    if (ext == nullptr)
    {
        return TYPE_BASE;
    }
    else if (!strncmp(filename, "VGM68.BIN", 9))
    {
        return TYPE_VGM_DRIVER;
    }
    else if (!strncmp(ext, ".VFS", 4))
    {
        return TYPE_VFS;
    }
    else if (!strncmp(ext, ".VF2", 5))
    {
        return TYPE_VFS2;
    }
    else if (!strncmp(ext, ".MOD", 4))
    {
        return TYPE_MOD;
    }
    else if (!strncmp(ext, ".VGM", 4))
    {
        return TYPE_VGM;
    }
    else if (!strncmp(ext, ".MDX", 4))
    {
        return TYPE_MDX;
    }
    else if (!strncmp(ext, ".AIF", 4))
    {
        return TYPE_AIFF;
    }
    else if (!strncmp(ext, ".ADP", 4))
    {
        return TYPE_ADPCM;
    }
    else if (!strncmp(ext, ".V68", 4))
    {
        return TYPE_VGM_DRIVER_MANUAL;
    }
    else if (!strncmp(ext, ".CPK", 4))
    {
        return TYPE_CINEPAK;
    }
    else if (!strncmp(ext, ".TON", 4))
    {
        return TYPE_TON;
    }
    else if (!strncmp(ext, ".SEQ", 4))
    {
        return TYPE_SEQ;
    }
    else if (!strncmp(ext, ".EXB", 4)) //probably exb
    {
        return TYPE_DSPPROG;
    }
    else if (!strncmp(ext, ".DSP", 4)) //dont know extension for this
    {
        return TYPE_DSPRAM;
    }
    else if (!strncmp(ext, ".68K", 4))
    {
        return 0x5A00000;
    }
    else if (!strncmp(ext, ".MAP", 4))
    {
        return TYPE_MAP;
    }
    else if (!strncmp(ext, ".TSK", 5))
    {
        return TYPE_TSK;
    }
    return TYPE_BASE;
}

SRL::Input::Digital::Button ButtonPressed(SRL::Input::Digital& pad, bool allowHold)
{
    static int32_t frames = 0;
    static SRL::Input::Digital::Button lastButton = static_cast<SRL::Input::Digital::Button>(0);

    if (pad.IsConnected())
    {
        for (int i = 0; i < 16; i++)
        {
            SRL::Input::Digital::Button button = static_cast<SRL::Input::Digital::Button>(1 << (15 - i));
            if (button != SRL::Input::Digital::Button::START && pad.WasPressed(button))
            {
                lastButton = button;
                frames = 0;
                return button;
            }
        }
        if (pad.IsHeld(lastButton))
        {
            frames++;
            if (allowHold && frames > repeatDelay && frames % repeatFrames == 0) //after 30 frames of holding the button down, start registering it as being pressed again
            {
                return lastButton;
            }
            else
            {
                return static_cast<SRL::Input::Digital::Button>(0); //placeholder for no button pressed
            }
        }
    }
    return static_cast<SRL::Input::Digital::Button>(0); //placeholder
}

int32_t LoadSSF(MenuItem* item)
{
    if (currentFile[0])
    {
        TurnOff68K();
        SddrvsLoaded = false; //even though most ssfs are sega driver
        driverEnd = nullptr;
        SRL::Cd::File file(currentFile);
        int8_t* buffer = nullptr;
        switch (TempLocation)
        {
            case 0:
                buffer = new int8_t[item->size];
                break;
            case 1:
                buffer = lwnew int8_t[item->size];
                break;
            case 2:
                buffer = cartnew int8_t[item->size];
                break;
        }
        file.LoadBytes(item->offset, item->size, buffer);
        for (uint32_t i = 0; i < (item->size - sizeof(int32_t)); i += sizeof(int32_t))
        {
            int32_t bufferBE = SRL::Endian::DeserializeUint32( reinterpret_cast<uint8_t*>(&buffer[i]) ) + 2;
            file.LoadBytes(
                menuItems[bufferBE].offset, 
                menuItems[bufferBE].size, 
                reinterpret_cast<uint8_t*>(menuItems[bufferBE].type)
            );
        }
        file.Close();
        delete[] buffer;
        TurnOn68K();
        MusicType = TYPE_SSF;
        MusicPlaying = true;
        WaitedAfterTurningOff68K = false;
        SetVolume(0, true);
        return -1;
    }
    
    
    return -2;
}

int32_t LoadSSF2File(const char* fileName, int32_t offset, int32_t size, uint32_t type, bool StartHeld)
{
    if (!SddrvsLoaded)
    {
        SRL::Sound::Hardware::Initialize();
        SddrvsLoaded = true;
    }
    MusicType = TYPE_SSF2;
    uint8_t* buffer = nullptr;
    switch (TempLocation)
    {
        case 0:
            buffer = new uint8_t[size];
            break;
        case 1:
            buffer = lwnew uint8_t[size];
            break;
        case 2:
            buffer = cartnew uint8_t[size];
            break;
    }
    if (currentFile[0])
    {
        SRL::Cd::File file(currentFile);
        if (file.LoadBytes(offset, size, buffer) < 0)
        {
            delete[] buffer;
            file.Close();
            return -1;
        }
        file.Close();
    }
    else
    {
        SRL::Cd::File file(fileName);
        if (file.LoadBytes(0, size, buffer) < 0)
        {
            delete[] buffer;
            file.Close();
            return -2;
        }
        file.Close();
    }
    switch (type)
    {
        case TYPE_TSK:
            segaDriver = buffer;
            return -1;
        case TYPE_MAP:
            SndIniDt indt;
            if (!StartHeld && segaDriver == nullptr)
            {
                LoadSegaDriver("SDDRVS.TSK");
            }
            SND_INI_PRG_ADR(indt) = (uint16_t*)segaDriver;
            SND_INI_PRG_SZ(indt) = (uint16_t)segaDriverSize;
            SND_INI_ARA_ADR(indt) = (uint16_t*)buffer;
            SND_INI_ARA_SZ(indt) = (uint16_t)size;
            SND_Init(&indt);
            SND_ChgMap(0);
            break;
        case TYPE_SEQ:
            SND_MoveData((uint16_t*)buffer, size, SND_KD_SEQ, bank);
            SND_StartSeq(0, bank, 0, 8);
            SetVolume(0, true);
            break;
        case TYPE_TON:
            SND_MoveData((uint16_t*)buffer, size, SND_KD_TONE, bank);
            break;
        case TYPE_DSPPROG:
            SND_MoveData((uint16_t*)buffer, size, SND_KD_DSP_PRG, bank); //is bank correct?
            break;
        case TYPE_DSPRAM:
            SND_MoveData((uint16_t*)buffer, size, SND_KD_DSP_RAM, bank);
            break;
        default:
            delete[] buffer;
            return -2;
    }
    delete[] buffer;
    return -1;
}

int32_t LoadDriver(bool AllowReload)
{
    if (AllowReload && driverEnd != nullptr) //sounds terrible
    {
        for (int w=0;w<100000;w++)
        {
            asm("nop\n"); // waste time
        }
        return *(int32_t*)(0x25A00100);
    }
    else if (vgmdriver != nullptr)
    {
  
        TurnOff68K();
        SddrvsLoaded = false;
        MusicPlaying = false;
        if (!WaitedAfterTurningOff68K || driverEnd != nullptr)
        {
            for (int w=0;w<50000;w++)        
            {
                asm("nop\n"); // waste time
            }
            WaitedAfterTurningOff68K = true;
        }
        memcpy((void*)0x5A00000, vgmdriver, driverSize); //0x5A00000?
        TurnOn68K();
        //SRL::Debug::Print(1, 1, "VGM %d bytes", driverSize);
        while (*(uint16_t*)(0x25A00108) != 0)
        {
            for (int w=0;w<500;w++)
            {
                asm("nop\n"); // waste time
            }
        }
        uint32_t driverOffset = *(uint32_t*)(0x25A00100);
        *(uint32_t*)(0x25A00104) = driverOffset; //not needed?
        driverEnd = (uint8_t*)(driverOffset + 0x5A00000);
        //SRL::Debug::Print(1, 1, "Driver DONE!!!!!!!!!!", driverOffset + 0x5A00000);
        return driverOffset;
    }
    else
    {
        return -1;
    }
}

bool PlayPauseMusic(bool playing)
{
    if (playing)
    {
        switch (MusicType)
        {
            case TYPE_SSF:
            case TYPE_SSF2:
                for (int32_t i=0;i<8;i++)
                {
                    SND_PauseSeq(i);
                }
                return false;
            case TYPE_MOD:
            case TYPE_VGM:
            case TYPE_MDX:
                *(uint16_t*)(0x25A00108) = 0; //pause
                return false;
            case TYPE_AIFF:
            case TYPE_ADPCM:
                if (pcmstream != nullptr)
                {
                    pcmstream->PlayPause(true);
                }
                return false;
            case TYPE_CDDA:
                SRL::Sound::Cdda::StopPause();
                return false;
            default:
                break;
        }
    }
    else
    {
        switch (MusicType)
        {
            case TYPE_SSF:
            case TYPE_SSF2:
                for (int32_t i=0;i<8;i++)
                {
                    SND_ContSeq(i);
                }
                return true;
            case TYPE_MOD:
                *(uint16_t*)(0x25A00108) = 4;
                return true;
            case TYPE_VGM:
                *(uint16_t*)(0x25A00108) = 1; 
                return true;
            case TYPE_MDX:
                *(uint16_t*)(0x25A00108) = 3;
                return true;
            case TYPE_AIFF:
            case TYPE_ADPCM:
                if (pcmstream != nullptr)
                {
                    pcmstream->PlayPause(false);
                }
                return true;
            case TYPE_CDDA:
                SRL::Sound::Cdda::Resume();
                return true;
            default:
                break;
        }
    }
    return false;
}

int16_t SetVolume(int16_t newVolume, bool relative)
{
    if (relative)
    {
        volume += newVolume;
    }
    else
    {
        volume = newVolume;
    }
    if (volume < 0) volume = 0;
    else if (volume > 127) volume = 127;
    switch (MusicType)
    {
        case TYPE_SSF:
        case TYPE_SSF2:
            for (int8_t i=0;i<8;i++)
            {
                SND_SetSeqVl(i, volume, 0); //no fade
            }
            break;
        case TYPE_AIFF:
        case TYPE_ADPCM:
            if (pcmstream != nullptr)
            {
                pcmstream->SetVol(volume); //no idea why this cant get to full volume
            }
            break;
        case TYPE_CDDA:
            SRL::Sound::Cdda::SetVolume(volume >> 4); //CDDA volume is 0-7
            break;
        default:
            break;
    }
    return volume;
}

void PrintTime(uint8_t x, uint8_t y)
{
    //SRL::Debug::PrintClearLine(x);
    switch (MusicType)
    {
        case TYPE_SSF:
        case TYPE_SSF2:
            {
                SndSeqPlayPos sspp[8];
                for (int8_t i=0;i<8;i++)
                {
                    SND_GetSeqPlayPos(&sspp[i], i);
                }
                SRL::Debug::Print(x, y, "Time: %d,%d,%d,%d,%d,%d,%d,%d      ",
                     sspp[0], sspp[1], sspp[2], sspp[3], sspp[4], sspp[5], sspp[6], sspp[7]);
            }
            break;
        case TYPE_AIFF:
        case TYPE_ADPCM:
            if (pcmstream != nullptr)
            {
                int32_t time = pcmstream->GetTime();
                SRL::Debug::Print(x, y, "Time: %d:%02d (%d)          ", time / 60, time % 60, pcmstream->GetTime(false));
            }
            break;
        default:
            SRL::Debug::Print(x, y, "                               ");
            break;
    }
}

int32_t SetTempo(int32_t newTempo, bool relative)
{
    if (relative)
    {
        tempo += newTempo;
    }
    else
    {
        tempo = newTempo;
    }
    if (tempo > 0x7FFF) tempo = 0x7FFF;
    if (tempo < -0x8000) tempo = -0x8000;
    if (TurboMode)
    {
        FrameSetTempo(tempo);
    }
    return tempo;
}


void FrameSetTempo(int32_t newTempo)
{
    switch (MusicType)
    {
        case TYPE_SSF:
        case TYPE_SSF2:
            for (int32_t i = 0; i < 8; i++)
            {
                SND_ChgTempo(i, newTempo);
            }
            break;
        default:
            break;
    }
}

void LoadCDDA(int32_t track, bool StartHeld)
{
    if (!SddrvsLoaded)
    {
        SRL::Sound::Hardware::Initialize();
        SddrvsLoaded = true;
    }
    MusicType = TYPE_CDDA;
    MusicPlaying = true;
    SRL::Sound::Cdda::PlaySingle(track, StartHeld);
}

int32_t LoadSegaDriver(const char* fileName, int32_t offset, int32_t size)
{
    SRL::Cd::File file(fileName);
    if (size == 0)
    {
        size = file.Size.Bytes;
    }
    segaDriverSize = size;
    if (segaDriver != nullptr)
    {
        delete[] segaDriver;
        segaDriver = nullptr;
    }
    switch (DriverLocation)
    {
        case 0:
            segaDriver = new uint8_t[segaDriverSize];
            break;
        case 1:
            segaDriver = lwnew uint8_t[segaDriverSize];
            break;
        case 2:
            segaDriver = cartnew uint8_t[segaDriverSize];
            break;
    }

    file.LoadBytes(offset, size, segaDriver);
    file.Close();
    return -1;
}

int32_t LoadDriverSong(const char* fileName, int32_t offset, int32_t size, uint32_t type, bool StartHeld)
{
    //int32_t lddrv = LoadDriver();
    //SRL::Debug::Print(1, 4, "End: %d Size: %d", lddrv, size);
    if (size <= (0x80000 - LoadDriver(StartHeld)) && driverEnd != nullptr)
    {
        SRL::Cd::File file(fileName);
        file.LoadBytes(offset, size, driverEnd);
        file.Close();
        MusicType = type;
        //MusicPlaying = false;
        MusicPlaying = PlayPauseMusic(false);
        return -1;
    }

    return -2;
}

int32_t LoadSSF2(const char* fileName, int32_t offset, int32_t size, bool StartHeld)
{
    SRL::Cd::File file(fileName);
    int32_t* buffer = nullptr;
    switch (TempLocation)
    {
        case 0:
            buffer = new int32_t[size >> 2];
            break;
        case 1:
            buffer = lwnew int32_t[size >> 2];
            break;
        case 2:
            buffer = cartnew int32_t[size >> 2];
            break;
    }
    file.LoadBytes(offset, size, buffer);
    file.Close();
    for (int32_t i = 0; i < (size >> 2); i++)
    {
        //SRL::Debug::Print(1, 1, "Loading %s, [%d]", menuItems[buffer[i]].name, buffer[i]);
        LoadSSF2File(currentFile[0] ? currentFile : menuItems[buffer[i] + 2].name, 
            menuItems[buffer[i] + 2].offset, 
            menuItems[buffer[i] + 2].size, 
            menuItems[buffer[i] + 2].type, StartHeld);
    }
    delete[] buffer;
    return -1;
}