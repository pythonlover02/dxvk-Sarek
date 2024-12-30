#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <utility>

#include "config.h"

#include "../log/log.h"

#include "../util_env.h"

namespace dxvk {

  const static std::vector<std::pair<const char*, Config>> g_appDefaults = {{
    /**********************************************/
    /* D3D11 GAMES                                */
    /**********************************************/

    /* Batman Arkham Knight - doesn't like intel vendor id 
      (refuses to boot if vendor isn't 0x10de or 0x1002)  */
    { R"(\\BatmanAK\.exe$)", {{
      { "dxgi.hideIntelGpu",                "True" },
    }} },
    /* Assassin's Creed Syndicate: amdags issues  */
    { R"(\\ACS\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Dissidia Final Fantasy NT Free Edition */
    { R"(\\dffnt\.exe$)", {{
      { "dxgi.deferSurfaceCreation",        "True" },
    }} },
    /* Elite Dangerous: Compiles weird shaders    *
     * when running on AMD hardware               */
    { R"(\\EliteDangerous64\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
     /* EVE Online: Needs this to expose D3D12     *
     * otherwise D3D12 option on launcher is      *
     * greyed out                                 */
    { R"(\\evelauncher\.exe$)", {{
      { "d3d11.maxFeatureLevel",            "12_1" },
    }} },
    /* The Evil Within: Submits command lists     * 
     * multiple times                             */
    { R"(\\EvilWithin(Demo)?\.exe$)", {{
      { "d3d11.dcSingleUseMode",            "False" },
      { "d3d11.cachedDynamicResources",     "vi"   },
    }} },
    /* Far Cry 3: Assumes clear(0.5) on an UNORM  *
     * format to result in 128 on AMD and 127 on  *
     * Nvidia. We assume that the Vulkan drivers  *
     * match the clear behaviour of D3D11.        *
     * Intel needs to match the AMD result        */
    { R"(\\(farcry3|fc3_blooddragon)_d3d11\.exe$)", {{
      { "dxgi.hideNvidiaGpu",              "False" },
      { "dxgi.hideIntelGpu",                "True" },
    }} },
    /* Far Cry 4 and Primal: Same as Far Cry 3    */
    { R"(\\(FarCry4|FCPrimal)\.exe$)", {{
      { "dxgi.hideNvidiaGpu",              "False" },
      { "dxgi.hideIntelGpu",                "True" },
    }} },
    /* Frostpunk: Renders one frame with D3D9     *
     * after creating the DXGI swap chain         */
    { R"(\\Frostpunk\.exe$)", {{
      { "dxgi.deferSurfaceCreation",        "True" },
      { "d3d11.cachedDynamicResources",     "c" },
    }} },
    /* Nioh: Apparently the same as the Atelier games  */
    { R"(\\nioh\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Quantum Break: Mever initializes shared    *
     * memory in one of its compute shaders.      *
     * Also loves using MAP_WRITE on the same     *
     * set of resources multiple times per frame. */
    { R"(\\QuantumBreak\.exe$)", {{
      { "d3d11.zeroInitWorkgroupMemory",    "True" },
      { "d3d11.maxImplicitDiscardSize",     "-1"   },
    }} },
    /* Anno 2205: Random crashes with state cache */
    { R"(\\anno2205\.exe$)", {{
      { "dxvk.enableStateCache",            "False" },
    }} },
    /* Anno 1800: Poor performance without this   */
    { R"(\\Anno1800\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "c"    },
    }} },
    /* Fifa '19+: Binds typed buffer SRV to shader *
     * that expects raw/structured buffer SRV     */
    { R"(\\FIFA(19|[2-9][0-9])(_demo)?\.exe$)", {{
      { "dxvk.useRawSsbo",                  "True" },
    }} },
    /* Resident Evil 2/3: Ignore WaW hazards      */
    { R"(\\re(2|3|3demo)\.exe$)", {{
      { "d3d11.relaxedBarriers",            "True" },
    }} },
    /* Devil May Cry 5                            */
    { R"(\\DevilMayCry5\.exe$)", {{
      { "d3d11.relaxedBarriers",            "True" },
    }} },
    /* Call of Duty WW2                           */
    { R"(\\s2_sp64_ship\.exe$)", {{
      { "dxgi.hideNvidiaGpu",              "False" },
    }} },
    /* Need for Speed 2015                        */
    { R"(\\NFS16\.exe$)", {{
      { "dxgi.hideNvidiaGpu",              "False" },
    }} },
    /* Mass Effect Andromeda                      */
    { R"(\\MassEffectAndromeda\.exe$)", {{
      { "dxgi.hideNvidiaGpu",              "False" },
    }} },
    /* Mirror`s Edge Catalyst: Crashes on AMD     */
    { R"(\\MirrorsEdgeCatalyst(Trial)?\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Star Wars Battlefront (2015)               */
    { R"(\\starwarsbattlefront(trial)?\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Dark Souls Remastered                      */
    { R"(\\DarkSoulsRemastered\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Grim Dawn                                  */
    { R"(\\Grim Dawn\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* NieR:Automata                              */
    { R"(\\NieRAutomata\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* NieR Replicant                             */
    { R"(\\NieR Replicant ver\.1\.22474487139\.exe)", {{
      { "d3d11.cachedDynamicResources",     "vi"   },
    }} },
    /* SteamVR performance test                   */
    { R"(\\vr\.exe$)", {{
      { "d3d11.dcSingleUseMode",            "False" },
    }} },
    /* Hitman 2 - requires AGS library      */
    { R"(\\HITMAN2\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Modern Warfare Remastered                  */
    { R"(\\h1(_[ms]p64_ship|-mod)\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* H2M-Mod - Modern Warfare Remastered        */
    { R"(\\h2m-mod\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Modern Warfare 2 Campaign Remastered       *
     * AMD AGS crash same as above                */
    { R"(\\MW2CR\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Titan Quest                                */
    { R"(\\TQ\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Saints Row IV                              */
    { R"(\\SaintsRowIV\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Saints Row: The Third                      */
    { R"(\\SaintsRowTheThird_DX11\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Crysis 3 - slower if it notices AMD card     *
     * Apitrace mode helps massively in cpu bound   *
     * game parts                                   */
    { R"(\\Crysis3\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Crysis 3 Remastered                          *
     * Apitrace mode helps massively in cpu bound   *
     * game parts                                   */
    { R"(\\Crysis3Remastered\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Atelier series - games try to render video *
     * with a D3D9 swap chain over the DXGI swap  *
     * chain, which breaks D3D11 presentation     */
    { R"(\\Atelier_(Ayesha|Escha_and_Logy|Shallie)(_EN)?\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Atelier Firis                              */
    { R"(\\A18\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Atelier Rorona/Totori/Meruru               */
    { R"(\\A(11R|12V|13V)_x64_Release(_en)?\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Just how many of these games are there?    */
    { R"(\\Atelier_(Lulua|Lydie_and_Suelle|Ryza(_2|_3)?|Sophie_2)\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* ...                                        */
    { R"(\\Atelier_(Lydie_and_Suelle|Firis|Sophie)_DX\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Fairy Tail                                 */
    { R"(\\FAIRY_TAIL\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Nights of Azure                            */
    { R"(\\CNN\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Star Wars Battlefront II: amdags issues    */
    { R"(\\starwarsbattlefrontii\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* F1 games - do not synchronize TGSM access  *
     * in a compute shader, causing artifacts     */
    { R"(\\F1_20(1[89]|[2-9][0-9])\.exe$)", {{
      { "d3d11.forceTgsmBarriers",          "True" },
    }} },
    /* Blue Reflection                            */
    { R"(\\BLUE_REFLECTION\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Secret World Legends                       */
    { R"(\\SecretWorldLegendsDX11\.exe$)", {{
      { "d3d11.constantBufferRangeCheck",   "True" },
    }} },
    /* Darksiders Warmastered - apparently reads  *
     * from write-only mapped buffers             */
    { R"(\\darksiders1\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Monster Hunter World                       */
    { R"(\\MonsterHunterWorld\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Kingdome Come: Deliverance                 */
    { R"(\\KingdomCome\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Homefront: The Revolution                  */
    { R"(\\Homefront2_Release\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Sniper Ghost Warrior Contracts             */
    { R"(\\SGWContracts\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Armored Warfare             */
    { R"(\\armoredwarfare\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "c"    },
    }} },
    /* Shadow of the Tomb Raider - invariant      *
     * position breaks character rendering on NV  */
    { R"(\\SOTTR\.exe$)", {{
      { "d3d11.invariantPosition",          "False" },
      { "d3d11.floatControls",              "False" },
    }} },
    /* Nioh 2                                     */
    { R"(\\nioh2\.exe$)", {{
      { "dxgi.deferSurfaceCreation",        "True" },
    }} },
    /* DIRT 5 - uses amd_ags_x64.dll when it      *
     * detects an AMD GPU                         */
    { R"(\\DIRT5\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
    }} },
    /* Crazy Machines 3 - crashes on long device  *
     * descriptions                               */
    { R"(\\cm3\.exe$)", {{
      { "dxgi.customDeviceDesc",            "DXVK Adapter" },
    }} },
    /* World of Final Fantasy: Broken and useless *
     * use of 4x MSAA throughout the renderer     */
    { R"(\\WOFF\.exe$)", {{
      { "d3d11.disableMsaa",                "True" },
    }} },
     /* Mary Skelter 2 - Broken MSAA              */
    { R"(\\MarySkelter2\.exe$)", {{
      { "d3d11.disableMsaa",                "True" },
    }} },
    /* Final Fantasy XIV - Stuttering on NV       */
    { R"(\\ffxiv_dx11\.exe$)", {{
      { "dxvk.shrinkNvidiaHvvHeap",         "True" },
      { "d3d11.cachedDynamicResources",     "vi"   },
    }} },
    /* Final Fantasy XV: VXAO does thousands of   *
     * draw calls with the same UAV bound         */
    { R"(\\ffxv_s\.exe$)", {{
      { "d3d11.ignoreGraphicsBarriers",     "True" },
    }} },
    /* God of War - relies on NVAPI/AMDAGS for    *
     * barrier stuff, needs nvapi for DLSS        */
    { R"(\\GoW\.exe$)", {{
      { "d3d11.ignoreGraphicsBarriers",     "True" },
      { "d3d11.relaxedBarriers",            "True" },
      { "dxgi.hideNvidiaGpu",              "False" },
      { "dxgi.maxFrameLatency",             "1"    },
    }} },
    /* AoE 2 DE - runs poorly for some users      */
    { R"(\\AoE2DE_s\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Total War: Warhammer III                   */
    { R"(\\Warhammer3\.exe$)", {{
      { "d3d11.maxDynamicImageBufferSize",  "4096" },
    }} },
    /* Assassin's Creed 3 and 4                   */
    { R"(\\ac(3|4bf)[sm]p\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "a"    },
    }} },
    /* Stranger of Paradise - FF Origin           */
    { R"(\\SOPFFO\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    /* Small Radios Big Televisions               */
    }} },
    { R"(\\SRBT\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* A Way Out: fix for stuttering and low fps  */
    { R"(\\AWayOut(_friend)?\.exe$)", {{
      { "dxgi.maxFrameLatency",                "1" },
    }} },
    /* Garden Warfare 2
       Won't start on amd Id without atiadlxx     */
    { R"(\\GW2.Main_Win64_Retail\.exe$)", {{
      { "dxgi.customVendorId",           "10de"   },
    }} },
    /* DayZ */
    { R"(\\DayZ_x64\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "cr" },
    }} },
    /* Stray - writes to the same UAV every draw, *
     * presumably for culling, which doesn't play *
     * nicely with D3D11 without vendor libraries */
    { R"(\\Stray-Win64-Shipping\.exe$)", {{
      { "d3d11.ignoreGraphicsBarriers",     "True" },
    }} },
    /* Metal Gear Solid V: Ground Zeroes          *
     * Texture quality can break at high vram     */
    { R"(\\MgsGroundZeroes\.exe$)", {{
      { "dxgi.maxDeviceMemory",     "4095" },
    }} },
    /* Shantae and the Pirate's Curse             *
     * Game speeds up above 60 fps                */
    { R"(\\ShantaeCurse\.exe$)", {{
      { "dxgi.maxFrameRate",                 "60" },
    }} },
    /* Mighty Switch Force! Collection            *
     * Games speed up above 60 fps                */
    { R"(\\MSFC\.exe$)", {{
      { "dxgi.maxFrameRate",                 "60" },
    }} },
    /* Cardfight!! Vanguard Dear Days:            *
     * Submits command lists multiple times       */
    { R"(\\VG2\.exe$)", {{
      { "d3d11.dcSingleUseMode",             "False" },
    }} },
    /* Battlefield: Bad Company 2                 *
     * Gets rid of black flickering               */
    { R"(\\BFBC2Game\.exe$)", {{
      { "d3d11.floatControls",            "False" },
    }} },
    /* Sonic Frontiers - flickering shadows and   *
     * vegetation when GPU-bound                  */
    { R"(\\SonicFrontiers\.exe$)", {{
      { "dxgi.maxFrameLatency",             "1" },
    }} },
    /* TRAHA Global                               *
     * Shadow issues when it sees AMD/Nvidia      */
    { R"(\\RapaNui-Win64-Shipping\.exe$)", {{
      { "dxgi.customVendorId",              "8086" },
    }} },
    /* SpellForce 3 Reforced & expansions         *
     * Greatly improves CPU bound performance     */
    { R"(\\SF3ClientFinal\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "v" },
    }} },
    /* Tom Clancy's Ghost Recon Breakpoint        */
    { R"(\\GRB\.exe$)", {{
      { "dxgi.hideNvidiaGpu",           "False" },
    }} },
    /* GTA V performance issues                   */
    { R"(\\GTA5\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "vi"   },
    }} },
    /* Crash Bandicoot N. Sane Trilogy            *
     * Work around some vsync funkiness           */
    { R"(\\CrashBandicootNSaneTrilogy\.exe$)", {{
      { "dxgi.syncInterval",                "1"   },
    }} },
    /* SnowRunner                                 */
    { R"(\\SnowRunner\.exe$)", {{
      { "d3d11.dcSingleUseMode",            "False" },
    }} },
    /* Fallout 76
     * Game tries to be too "smart" and changes sync
     * interval based on performance (in fullscreen)
     * or tries to match (or ratio below) 60fps
     * (in windowed).
     *
     * Ends up getting in a loop where it will switch
     * and start stuttering, or get stuck at targeting
     * 30Hz in fullscreen.
     * Windowed mode being locked to 60fps as well is
     * pretty suboptimal...
     */
    { R"(\\Fallout76\.exe$)", {{
      { "dxgi.syncInterval",                "1" },
    }} },
    /* Bladestorm Nightmare                       *
     * Game speed increases when above 60 fps in  *
     * the tavern area                            */
    { R"(\\BLADESTORM Nightmare\\Launch_(EA|JP)\.exe$)", {{
      { "dxgi.maxFrameRate",                "60"  },
    }} },
    /* Ghost Recon Wildlands                      */
    { R"(\\GRW\.exe$)", {{
      { "d3d11.dcSingleUseMode",            "False" },
    }} },
    /* Riders Republic - Statically linked AMDAGS */
    { R"(\\RidersRepublic(_BE)?\.exe$)", {{
      { "dxgi.hideAmdGpu",                "True"   },
    }} },
    /* Kenshi                                     *
     * Helps CPU bound performance                */
    { R"(\\kenshi_x64\.exe$)", {{
      { "d3d11.cachedDynamicResources",     "v"    },
    }} },
    /* Crysis 1/Warhead - Game bug in d3d10 makes *
     * it select lowest supported refresh rate    */
    { R"(\\Crysis(64)?\.exe$)", {{
      { "d3d9.maxFrameRate",              "-1"      },
      { "dxgi.maxFrameRate",              "-1"      },
    }} },
    /* Kena: Bridge of Spirits: intel water       * 
     * flickering issues                          */
    { R"(\\Kena-Win64-Shipping\.exe$)", {{
      { "dxgi.hideIntelGpu",                 "True" },
    }} },

    /**********************************************/
    /* D3D9 GAMES                                 */
    /**********************************************/

    /* A Hat in Time                              */
    { R"(\\HatinTimeGame\.exe$)", {{
      { "d3d9.strictPow",                   "False" },
      { "d3d9.lenientClear",                "True" },
    }} },
    /* Anarchy Online                             */
    { R"(\\anarchyonline\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
    }} },
    /* Borderlands                                */
    { R"(\\Borderlands\.exe$)", {{
      { "d3d9.lenientClear",                "True" },
    }} },
    /* Borderlands 2                               *
     * Missing lava in Vault of the Warrior        *
     * without Strict floats                       */
    { R"(\\Borderlands2\.exe$)", {{
      { "d3d9.lenientClear",                "True" },
      { "d3d9.supportDFFormats",            "False" },
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Borderlands: The Pre-Sequel                  */
    { R"(\\BorderlandsPreSequel\.exe$)", {{
      { "d3d9.lenientClear",                "True" },
      { "d3d9.supportDFFormats",            "False" },
    }} },
    /* Gothic 3                                   */
    { R"(\\Gothic(3|3Final| III Forsaken Gods)\.exe$)", {{
      { "d3d9.supportDFFormats",            "False" },
    }} },
    /* Sonic Adventure 2                          */
    { R"(\\Sonic Adventure 2\\(launcher|sonic2app)\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* The Sims 2,
       Body Shop,
       The Sims Life Stories,
       The Sims Pet Stories,
       and The Sims Castaway Stories             */
    { R"(\\(Sims2.*|TS2BodyShop|SimsLS|SimsPS|SimsCS)"
      R"(|The Sims 2 Content Manager|TS2HomeCrafterPlus)\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
      { "d3d9.customDeviceId",              "0091" },
      { "d3d9.customDeviceDesc",            "GeForce 7800 GTX" },
      { "d3d9.disableA8RT",                 "True" },
      { "d3d9.supportX4R4G4B4",             "False" },
      { "d3d9.maxAvailableMemory",          "2048" },
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Dead Space uses the a NULL render target instead
       of a 1x1 one if DF24 is NOT supported     
       Mouse and physics issues above 60 FPS
       Built-in Vsync Locks the game to 30 FPS    */
    { R"(\\Dead Space\.exe$)", {{
      { "d3d9.supportDFFormats",                 "False" },
      { "d3d9.maxFrameRate",                     "60" },
      { "d3d9.presentInterval",                  "1" },
    }} },
    /* Dead Space 2
       Physics issues above 60 FPS
       Built-in Vsync Locks the game to 30 FPS
    */
    { R"(\\deadspace2\.exe$)", {{
      { "d3d9.maxFrameRate",                     "60" },
      { "d3d9.presentInterval",                  "1" },
    }} },
    /* Halo CE/HaloPC                             */
    { R"(\\halo(ce)?\.exe$)", {{
      // Game enables minor decal layering fixes
      // specifically when it detects AMD.
      // Avoids chip being detected as unsupported
      // when on intel. Avoids possible path towards
      // invalid texture addressing methods.
      { "d3d9.customVendorId",              "1002" },
      // Avoids card not recognized error.
      // Keeps game's rendering methods consistent
      // for optimal compatibility.
      { "d3d9.customDeviceId",              "4172" },
      // The game uses incorrect sampler types in
      // the shaders for glass rendering which
      // breaks it on native + us if we don't
      // spec-constantly chose the sampler type
      // automagically.
      { "d3d9.forceSamplerTypeSpecConstants", "True" },
    }} },
    /* Counter Strike: Global Offensive
       Needs NVAPI to avoid a forced AO + Smoke
       exploit so we must force AMD vendor ID.    */
    { R"(\\csgo\.exe$)", {{
      { "d3d9.customVendorId",              "1002" },
    }} },
    /* Vampire - The Masquerade Bloodlines        */
    { R"(\\vampire\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",          "1024" },
    }} },
    /* Senran Kagura Shinovi Versus               */
    { R"(\\SKShinoviVersus\.exe$)", {{
      { "d3d9.forceAspectRatio",            "16:9" },
    }} },
    /* Metal Slug X                               */
    { R"(\\mslugx\.exe$)", {{
      { "d3d9.supportD32",                  "False" },
    }} },
    /* Skyrim (NVAPI)                             */
    { R"(\\TESV\.exe$)", {{
      { "d3d9.customVendorId",              "1002" },
    }} },
    /* RTHDRIBL Demo                              
       Uses DONOTWAIT after GetRenderTargetData
       then goes into an infinite loop if it gets
       D3DERR_WASSTILLDRAWING.
       This is a better solution than penalizing
       other apps that use this properly.         */
    { R"(\\rthdribl\.exe$)", {{
      { "d3d9.allowDoNotWait",              "False" },
    }} },
    /* Hyperdimension Neptunia U: Action Unleashed */
    { R"(\\Neptunia\.exe$)", {{
      { "d3d9.forceAspectRatio",            "16:9" },
    }} },
    /* ZUSI 3 - Aerosoft Edition                  */
    { R"(\\ZusiSim\.exe$)", {{
      { "d3d9.noExplicitFrontBuffer",       "True" },
    }} },
    /* GTA IV (NVAPI)                             */
    /* Also thinks we're always on Intel          *
     * and will report/use bad amounts of VRAM
     * if we report more than 128 MB of VRAM.
     * Disabling support for DF texture formats
     * makes the game use a better looking render
     * path for mirrors                           */
    { R"(\\(GTAIV|EFLC)\.exe$)", {{
      { "d3d9.customVendorId",              "1002" },
      { "dxgi.maxDeviceMemory",             "128" },
      { "d3d9.supportDFFormats",            "False" },
    }} },
    /* Battlefield 2 & Battlefield 2142           *
     * Bad z-pass and ingame GUI loss on alt tab  *
     * Also hang when alt tabbing which seems     *
     * like a game bug that d3d9 drivers work     *
     * around.                                    */
    { R"(\\(BF2|BF2142)\.exe$)", {{
      { "d3d9.longMad",                     "True" },
    }} },
    /* SpellForce 2 Series                        */
    { R"(\\SpellForce2.*\.exe$)", {{
      { "d3d9.forceSamplerTypeSpecConstants", "True" },
    }} },
    /* Everquest 2                                */
    { R"(\\EverQuest2.*\.exe$)", {{
      { "d3d9.alphaTestWiggleRoom", "True" },
    }} },
    /* Tomb Raider: Legend, Anniversary, Underworld  *
     * Read from a buffer created with:              *
     * D3DPOOL_DEFAULT,                              *
     * D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY buffer. *
     * Legend flickers with next gen content option. */
    { R"(\\(trl|tra|tru)\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Everquest                                 */
    { R"(\\eqgame\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Dark Messiah of Might & Magic             */
    { R"(\\mm\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
      { "d3d9.memoryTrackTest",             "True" },
    }} },
    /* Mafia 2                                   */
    { R"(\\mafia2\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
      { "d3d9.customDeviceId",              "0402" },
    }} },
    /* Warhammer: Online                         */
    { R"(\\(WAR(-64)?|WARTEST(-64)?)\.exe$)", {{
      { "d3d9.customVendorId",              "1002" },
    }} },
    /* Dragon Nest                               */
    { R"(\\DragonNest_x64\.exe$)", {{
      { "d3d9.memoryTrackTest ",            "True" },
    }} },
    /* Dal Segno                                 */
    { R"(\\DST\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Kohan II                                  */
    { R"(\\k2\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
    }} },
    /* Time Leap Paradise SUPER LIVE             */
    { R"(\\tlpsl\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Ninja Gaiden Sigma 1/2                    */
    { R"(\\NINJA GAIDEN SIGMA(2)?\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Demon Stone breaks at frame rates > 60fps */
    { R"(\\Demonstone\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Far Cry 1 has worse water rendering when it detects AMD GPUs */
    { R"(\\FarCry\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
    }} },
    /* Earth Defense Force 5 */
    { R"(\\EDF5\.exe$)", {{
      { "dxgi.tearFree",                    "False" },
      { "dxgi.syncInterval",                "1"     },
    }} },
    /* Sine Mora EX */
    { R"(\\SineMoraEX\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Fantasy Grounds                           */
    { R"(\\FantasyGrounds\.exe$)", {{
      { "d3d9.noExplicitFrontBuffer",       "True" },
    }} },
    /* Red Orchestra 2                           */
    { R"(\\ROGame\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Dark Souls II                            */
    { R"(\\DarkSoulsII\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Dogfight 1942                            */
    { R"(\\Dogfight1942\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Bayonetta                                */
    { R"(\\Bayonetta\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Rayman Origins                           */
    { R"(\\Rayman Origins\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Guilty Gear Xrd -Relevator-              */
    { R"(\\GuiltyGearXrd\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Richard Burns Rally                      */
    { R"(\\RichardBurnsRally_SSE\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* BlazBlue Centralfiction                  */
    { R"(\\BBCF\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Resident Evil games                      */
    { R"(\\(rerev|rerev2|re0hd|bhd|re5dx9|BH6)\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",                "False" },
    }} },
    /* Limbo                                    */
    { R"(\\limbo\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Warhammer: Return of Reckoning Launcher
       Forcing SM1 fixes a black window otherwise caused by
       the lack of support for partial presentation */
    { R"(\\RoRLauncher\.exe$)", {{
      { "d3d9.shaderModel",                 "1" },
    }} },
    /* Halo CE SPV3 launcher
       Same issue as Warhammer: RoR above       */
    { R"(\\spv3\.exe$)", {{
      { "d3d9.shaderModel",                 "1" },
    }} },
    /* Escape from Tarkov launcher
       Same issue as Warhammer: RoR above       */
    { R"(\\BsgLauncher\.exe$)", {{
      { "d3d9.shaderModel",                 "1" },
    }} },
    /* Star Wars The Force Unleashed 2          *
     * Black particles because it tries to bind *
     * a 2D texture for a shader that           *
     * declares a 3d texture.                   */
    { R"(\\SWTFU2\.exe$)", {{
      { "d3d9.forceSamplerTypeSpecConstants",  "True" },
    }} },
    /* Majesty 2 (Collection)                   *
     * Crashes on UMA without a memory limit,   *
     * since the game(s) will allocate all      *
     * available VRAM on startup.               */
    { R"(\\Majesty2\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",          "2048" },
    }} },
    /* Myst V End of Ages                             
       Game has white textures on amd radv.
       Expects Nvidia, Intel or ATI VendorId.
       "Radeon" in gpu description also works   */
    { R"(\\eoa\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
    }} },
    /* Beyond Good And Evil                     *
     * Fixes missing sun and light shafts       *
     * UI breaks at high fps                    */
    { R"(\\BGE\.exe$)", {{
      { "d3d9.allowDoNotWait",             "False" },
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Supreme Commander & Forged Alliance Forever */
    { R"(\\(SupremeCommander|ForgedAlliance)\.exe$)", {{
      { "d3d9.floatEmulation",            "Strict" },
    }} },
    /* Star Wars The Old Republic */
    { R"(\\swtor\.exe$)", {{
      { "d3d9.forceSamplerTypeSpecConstants", "True" },
    }} },
    /* Bionic Commando                          
       Physics break at high fps               */
    { R"(\\bionic_commando\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Port Royale 3                            *
     * Fixes infinite loading screens           */
    { R"(\\PortRoyale3\.exe$)", {{
      { "d3d9.allowDoNotWait",           "False" },
    }} },
    /* Ninja Blade                              *
     * Transparent main character on Nvidia     */
    { R"(\\NinjaBlade\.exe$)", {{
      { "d3d9.alphaTestWiggleRoom",       "True" },
    }} },
    /* King Of Fighters XIII                     *
     * In-game speed increases on high FPS       */
    { R"(\\kof(xiii|13_win32_Release)\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* YS Origin                                *
     * Helps very bad frametimes in some areas  */
    { R"(\\yso_win\.exe$)", {{
      { "d3d9.maxFrameLatency",              "1" },
    }} },
    /* Heroes of Annihilated Empires            *
     * Has issues with texture rendering and    *
     * video memory detection otherwise.        */
    { R"(\\Heroes (o|O)f Annihilated Empires.*\\engine\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",          "2048" },
    }} },
    /* The Ship (2004)                          */
    { R"(\\ship\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
    }} },
    /* SiN Episodes Emergence                   */
    { R"(\\SinEpisodes\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
    }} },
    /* Witcher 1: Very long loading times       */
    { R"(\\witcher\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Hammer World Editor                      */
    { R"(\\(hammer(plusplus)?|mallet|wc)\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Dragon Age Origins                       *
     * Keeps unmapping the same 3 1MB buffers   *
     * thousands of times when you alt-tab out  *
     * Causing it to crash OOM                  */
    { R"(\\DAOrigins\.exe$)" , {{
      { "d3d9.allowDirectBufferMapping",    "False" },
    }} },
    /* Fallout 3 - Doesn't like Intel Id       */
    { R"(\\Fallout3\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
    }} },
    /* Sonic & All-Stars Racing Transformed    *
     * Helps performance when Resizable BAR    *
     * is enabled                              */
    { R"(\\ASN_App_PcDx9_Final\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Black Mesa                              *
     * Artifacts & broken flashlight on Intel  */
    { R"(\\bms\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
    }} },
    /* Secret World Legends launcher           *
     * Invisible UI                            */
    { R"(\\Secret World Legends\\ClientPatcher\.exe$)", {{
      { "d3d9.shaderModel",                 "2" },
    }} },
    /* Alien Rage                              *
     * GTX 295 & disable Hack to fix shadows   */
    { R"(\\(ShippingPC-AFEARGame|ARageMP)\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
      { "d3d9.customDeviceId",              "05E0" },
      { "dxgi.hideNvidiaGpu",              "False" },
    }} },
    /* Battle Fantasia Revised Edition         *
     * Speedup above 60fps                     */
    { R"(\\bf10\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Codename Panzers Phase One/Two          *
     * Main menu won't render after intros     *
     * and CPU bound performance               */
    { R"(\\(PANZERS|PANZERS_Phase_2)\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Halo Online                             *
     * Black textures                          */
    { R"(\\eldorado\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict"   },
      { "d3d9.allowDirectBufferMapping",    "False" },
    }} },
    /* Injustice: Gods Among Us                *
     * Locks a buffer that's still in use      */
    { R"(\\injustice\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",    "False" },
    }} },
    /* STEINS;GATE ELITE                       */
    { R"(\\SG_ELITE\\Game\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* The Incredibles                         */
    { R"(\\IncPC\.exe$)", {{
      { "d3d9.maxFrameRate",                "59" },
    }} },
    /* Conflict Vietnam                        */
    { R"(\\Vietnam\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Project: Snowblind                      */
    { R"(\\Snowblind\.(SP|MP|exe)$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Aviary Attorney                         */
    { R"(\\Aviary Attorney\\nw\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Drakensang: The Dark Eye                */
    { R"(\\drakensang\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Age of Empires 2 - janky frame timing   */
    { R"(\\AoK HD\.exe$)", {{
      { "d3d9.maxFrameLatency",             "1" },
    }} },
    /* Battlestations Midway                   */
    { R"(\\Battlestationsmidway\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* SkyDrift                                 *
     * Works around alt tab OOM crash           */
    { R"(\\SkyDrift\.exe$)" , {{
      { "d3d9.allowDirectBufferMapping",    "False" },
    }} },
    /* Sonic CD                                */
    { R"(\\soniccd\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* UK Truck Simulator 1                    */
    { R"(\\UK Truck Simulator\\bin\\win_x86\\game\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Operation Flashpoint: Red River           *
     * Flickering issues                         */
    { R"(\\RedRiver\.exe$)", {{
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Dark Void - Crashes above 60fps in places */
    { R"(\\ShippingPC-SkyGame\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* 9th Dawn II                               *
     * OpenGL game that also spins up d3d9       *
     * Black screens without config              */
    { R"(\\ninthdawnii\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* Delta Force: Xtreme 1 & 2                 *
     * Black screen on Alt-Tab and performance   */
    { R"(\\(DFX|dfx2)\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Prototype                                 *
     * Incorrect shadows on AMD & Intel          */
    { R"(\\prototypef\.exe$)", {{ 
      { "d3d9.supportDFFormats",            "False" },
    }} },
    /* Fallout New Vegas - Various visual issues *
     * with mods such as New Vegas Reloaded      */
    { R"(\\FalloutNV\.exe$)", {{ 
      { "d3d9.floatEmulation",              "Strict" },
    }} },
    /* Dungeons and Dragons: Dragonshard         *
     * Massive FPS decreases in some scenes      */
    { R"(\\Dragonshard\.exe$)", {{ 
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Battle for Middle-earth 2 and expansion   *
     * Slowdowns in certain scenarios            */
    { R"(\\(The Battle for Middle-earth( \(tm\))? II( Demo)?)"
      R"(|The Lord of the Rings, The Rise of the Witch-king)\\game\.dat$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* WRC4 - Audio breaks above 60fps           */
    { R"(\\WRC4\.exe$)", {{
      { "d3d9.maxFrameRate",                "60" },
    }} },
    /* Splinter Cell Conviction - Alt-tab black  *
     * screen and unsupported GPU complaint      */
    { R"(\\conviction_game\.exe$)", {{
      { "dxgi.customVendorId",              "10de" },
      { "dxgi.customDeviceId",              "05e0" },
      { "dxgi.customDeviceDesc",            "GeForce GTX 295" },
    }} },
    /* APB: Reloaded                               *
     * Fixes frametime jumps when shooting         */
    { R"(\\APB\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Battle Mages - helps CPU bound perf         */
    { R"(\\Battle Mages\\mages\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Prince of Persia (2008) - Can get stuck     *
     * during loading at very high fps             */
    { R"(\\Prince( of Persia|OfPersia_Launcher)\.exe$)", {{
      { "d3d9.maxFrameRate",                 "240" },
    }} },
    /* F.E.A.R 1 & expansions                      *
     * Graphics glitches at very high fps          */
    { R"(\\FEAR(MP|XP|XP2)?\.exe$)", {{
      { "d3d9.maxFrameRate",                 "360" },
    }} },
    /* Secret World Legends - d3d9 mode only sees  *
     * 512MB vram locking higher graphics presets  */
    { R"(\\SecretWorldLegends\.exe$)", {{
      { "d3d9.memoryTrackTest",              "True" },
    }} },
    /* Far Cry 2: Set vendor ID to Nvidia to       *
     * avoid vegetation artifacts on Intel, and    *
     * set apitrace mode to True to improve perf   *
     * on all hardware.                            */
    { R"(\\(FarCry2|farcry2game)\.exe$)", {{
      { "d3d9.customVendorId",              "10de" },
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Alpha Protocol - Rids unwanted reflections  */
    { R"(\\APGame\.exe$)", {{
      { "d3d9.forceSamplerTypeSpecConstants", "True" },
    }} },

    /**********************************************/
    /* D3D8 GAMES                                 */
    /**********************************************/

    /* D&D - The Temple Of Elemental Evil        */
    { R"(\\ToEE(a)?\.exe$)", {{
      { "d3d9.allowDiscard",               "False" },
    }} },
    /* Duke Nukem Forever (2001)                 */
    { R"(\\DukeForever\.exe$)", {{
      { "d3d9.maxFrameRate",                "60"   },
    }} },
    /* Anito: Defend a Land Enraged              */
    { R"(\\Anito\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",          "1024" },
    }} },
    /* Red Faction                               *
     * Fixes crashing when starting a new game   */
    { R"(\\RF\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",   "False" },
    }} },
    /* Commandos 3                               *
     * The game doesn't use NOOVERWRITE properly *
     * and reads from actively modified buffers, *
     * which causes graphical glitches at times  */
    { R"(\\Commandos3\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",   "False" },
    }} },
    /* Motor City Online                         */
    { R"(\\MCity_d\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
      { "d3d8.batching",                    "True" },
    }} },
    /* Railroad Tycoon 3                         */
    { R"(\\RT3\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Pure Pinball 2.0 REDUX                    *
     * This game reads from undeclared vs inputs *
     * but somehow works on native. Let's just   *
     * change its declaration to make them work. */
    { R"(\\Pure Pinball 2\.0 REDUX\.exe$)", {{
      { "d3d8.forceVsDecl",  "0:2,4:2,7:4,9:1,8:1" },
    }} },
    /* Need for Speed III: Hot Pursuit           *
       (with the "Modern Patch")                 */
    { R"(\\nfs3\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
      { "d3d8.batching",                    "True" },
    }} },
    /* Need for Speed: High Stakes / Road         *
       Challenge (with the "Modern Patch") -      *
       Won't actually render anything in game     *
       without a memory limit in place            */
    { R"(\\nfs4\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",           "256" },
      { "d3d8.batching",                    "True" },
    }} },
    /* Need for Speed: Hot Pursuit 2              */
    { R"(\\NFSHP2\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Project I.G.I. 2: Covert Strike            */
    { R"(\\igi2\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Treasure Planet: Battle at Procyon        *
     * Declares v5 as color but shader uses v6   */
    { R"(\\TP_Win32\.exe$)", {{
      { "d3d8.forceVsDecl",      "0:2,3:2,6:4,7:1" },
    }} },
    /* Scrapland (Remastered)                   */
    { R"(\\Scrap\.exe$)", {{
      { "d3d9.deferSurfaceCreation",        "True" },
    }} },
    /* V-Rally 3                                  */
    { R"(\\VRally3(Demo)?\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Soldiers: Heroes Of World War II           *
     * Fills up all available memory and hangs    *
     * while loading the main menu otherwise      */
    { R"(\\Soldiers\.exe$)", {{
      { "d3d9.memoryTrackTest",             "True" },
      { "d3d9.maxAvailableMemory",          "512"  },
    }} },
    /* Cossacks II: Napoleonic Wars &             *
     * Battle for Europe                          */
    { R"(\\Cossacks II.*\\engine\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Alexander                                  */
    { R"(\\Alexander\\Data\\engine\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* 3DMark2001 (SE)                            *
     * Fixes a drastic performance drop in the    *
     * "Car Chase - High Detail" benchmark        */
    { R"(\\3DMark2001(SE)?\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",   "False" },
    }} },
    /* Delta Force: Black Hawk Down               */
    { R"(\\dfbhd\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* X2: The Threat                             */
    { R"(\\X2\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* The Lord of the Rings:                     *
     * The Fellowship of the Ring                 */
    { R"(\\Fellowship\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
      { "d3d8.placeP8InScratch",            "True" },
    }} },
    /* Art of Murder FBI Confidential - CPU perf  */
    { R"(\\Art of Murder - FBI Confidential\\game\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Max Payne 1 - Stalls waiting for an index buffer */
    { R"(\\MaxPayne\.exe$)", {{
      { "d3d9.allowDirectBufferMapping",   "False" },
    }} },
    /* Z: Steel Soldiers                          */
    { R"(\\z2\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* FIFA Football 2003                         */
    { R"(\\fifa2003(demo)?\.exe$)", {{
      { "d3d9.cachedDynamicBuffers",        "True" },
    }} },
    /* Splinter Cell: Pandora Tomorrow            *
     * Broken inputs and physics above 60 FPS     */
    { R"(\\SplinterCell2\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Chrome: Gold Edition                       *
     * Broken character model motion at high FPS  */
    { R"(\\Chrome(Single|Net)\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
    }} },
    /* Rayman 3: Hoodlum Havoc                    *
     * Missing geometry and textures without      *
     * legacy DISCARD behavior                    */
    { R"(\\Rayman3\.exe$)", {{
      { "d3d9.maxFrameRate",                  "60" },
      { "d3d8.forceLegacyDiscard",          "True" },
    }} },
  }};


  static bool isWhitespace(char ch) {
    return ch == ' ' || ch == '\x9' || ch == '\r';
  }

  
  static bool isValidKeyChar(char ch) {
    return (ch >= '0' && ch <= '9')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= 'a' && ch <= 'z')
        || (ch == '.' || ch == '_');
  }


  static size_t skipWhitespace(const std::string& line, size_t n) {
    while (n < line.size() && isWhitespace(line[n]))
      n += 1;
    return n;
  }


  struct ConfigContext {
    bool active;
  };


  static void parseUserConfigLine(Config& config, ConfigContext& ctx, const std::string& line) {
    std::stringstream key;
    std::stringstream value;

    // Extract the key
    size_t n = skipWhitespace(line, 0);

    if (n < line.size() && line[n] == '[') {
      n += 1;

      size_t e = line.size() - 1;
      while (e > n && line[e] != ']')
        e -= 1;

      while (n < e)
        key << line[n++];
      
      ctx.active = key.str() == env::getExeName();
    } else {
      while (n < line.size() && isValidKeyChar(line[n]))
        key << line[n++];
      
      // Check whether the next char is a '='
      n = skipWhitespace(line, n);
      if (n >= line.size() || line[n] != '=')
        return;

      // Extract the value
      bool insideString = false;
      n = skipWhitespace(line, n + 1);

      while (n < line.size()) {
        if (!insideString && isWhitespace(line[n]))
          break;

        if (line[n] == '"') {
          insideString = !insideString;
          n++;
        } else
          value << line[n++];
      }
      
      if (ctx.active)
        config.setOption(key.str(), value.str());
    }
  }


  Config::Config() { }
  Config::~Config() { }


  Config::Config(OptionMap&& options)
  : m_options(std::move(options)) { }


  void Config::merge(const Config& other) {
    for (auto& pair : other.m_options)
      m_options.insert(pair);
  }


  void Config::setOption(const std::string& key, const std::string& value) {
    m_options.insert_or_assign(key, value);
  }


  std::string Config::getOptionValue(const char* option) const {
    auto iter = m_options.find(option);

    return iter != m_options.end()
      ? iter->second : std::string();
  }


  bool Config::parseOptionValue(
    const std::string&  value,
          std::string&  result) {
    result = value;
    return true;
  }


  bool Config::parseOptionValue(
    const std::string&  value,
          bool&         result) {
    static const std::array<std::pair<const char*, bool>, 2> s_lookup = {{
      { "true",  true  },
      { "false", false },
    }};

    return parseStringOption(value,
      s_lookup.begin(), s_lookup.end(), result);
  }


  bool Config::parseOptionValue(
    const std::string&  value,
          int32_t&      result) {
    if (value.size() == 0)
      return false;
    
    // Parse sign, don't allow '+'
    int32_t sign = 1;
    size_t start = 0;

    if (value[0] == '-') {
      sign = -1;
      start = 1;
    }

    // Parse absolute number
    int32_t intval = 0;

    for (size_t i = start; i < value.size(); i++) {
      if (value[i] < '0' || value[i] > '9')
        return false;
      
      intval *= 10;
      intval += value[i] - '0';
    }

    // Apply sign and return
    result = sign * intval;
    return true;
  }
  
  
  bool Config::parseOptionValue(
    const std::string&  value,
          Tristate&     result) {
    static const std::array<std::pair<const char*, Tristate>, 3> s_lookup = {{
      { "true",  Tristate::True  },
      { "false", Tristate::False },
      { "auto",  Tristate::Auto  },
    }};

    return parseStringOption(value,
      s_lookup.begin(), s_lookup.end(), result);
  }


  template<typename I, typename V>
  bool Config::parseStringOption(
          std::string   str,
          I             begin,
          I             end,
          V&            value) {
    str = Config::toLower(str);

    for (auto i = begin; i != end; i++) {
      if (str == i->first) {
        value = i->second;
        return true;
      }
    }

    return false;
  }


  Config Config::getAppConfig(const std::string& appName) {
    auto appConfig = std::find_if(g_appDefaults.begin(), g_appDefaults.end(),
      [&appName] (const std::pair<const char*, Config>& pair) {
        std::regex expr(pair.first, std::regex::extended | std::regex::icase);
        return std::regex_search(appName, expr);
      });
    
    if (appConfig != g_appDefaults.end()) {
      // Inform the user that we loaded a default config
      Logger::info(str::format("Found built-in config:"));

      for (auto& pair : appConfig->second.m_options)
        Logger::info(str::format("  ", pair.first, " = ", pair.second));

      return appConfig->second;
    }

    return Config();
  }


  Config Config::getUserConfig() {
    Config config;

    // Load either $DXVK_CONFIG_FILE or $PWD/dxvk.conf
    std::string filePath = env::getEnvVar("DXVK_CONFIG_FILE");

    if (filePath == "")
      filePath = "dxvk.conf";
    
    // Open the file if it exists
    std::ifstream stream(str::tows(filePath.c_str()).c_str());

    if (!stream)
      return config;
    
    // Inform the user that we loaded a file, might
    // help when debugging configuration issues
    Logger::info(str::format("Found config file: ", filePath));

    // Initialize parser context
    ConfigContext ctx;
    ctx.active = true;

    // Parse the file line by line
    std::string line;

    while (std::getline(stream, line))
      parseUserConfigLine(config, ctx, line);
    
    return config;
  }


  void Config::logOptions() const {
    if (!m_options.empty()) {
      Logger::info("Effective configuration:");

      for (auto& pair : m_options)
        Logger::info(str::format("  ", pair.first, " = ", pair.second));
    }
  }

  std::string Config::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
      [] (unsigned char c) { return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; });
    return str;
  }

}
