#include "mod.h"
#include "patch.h"

#include <common.h>
#include <util.h>
#include <cutscene_helpers.h>
#include <evtpatch.h>
#include <evt_cmd.h>
#include <spm/rel/he1_01.h>
#include <spm/rel/ls3_12.h>
#include <spm/animdrv.h>
#include <spm/bgdrv.h>
#include <spm/camdrv.h>
#include <spm/dispdrv.h>
#include <spm/eff_small_star.h>
#include <spm/eff_spm_confetti.h>
#include <spm/eff_zunbaba.h>
#include <spm/evt_cam.h>
#include <spm/evt_dimen.h>
#include <spm/evt_eff.h>
#include <spm/evt_fairy.h>
#include <spm/evt_frame.h>
#include <spm/evt_guide.h>
#include <spm/evt_hit.h>
#include <spm/evt_img.h>
#include <spm/evt_item.h>
#include <spm/evt_map.h>
#include <spm/evt_mario.h>
#include <spm/evt_msg.h>
#include <spm/evt_mobj.h>
#include <spm/evt_npc.h>
#include <spm/evt_offscreen.h>
#include <spm/evt_paper.h>
#include <spm/evt_pouch.h>
#include <spm/evt_shop.h>
#include <spm/evt_snd.h>
#include <spm/evt_sub.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/evt_door.h>
#include <spm/fontmgr.h>
#include <spm/hitdrv.h>
#include <spm/hud.h>
#include <spm/itemdrv.h>
#include <spm/item_data.h>
#include <spm/lz_embedded.h>
#include <spm/map_data.h>
#include <spm/mapdrv.h>
#include <spm/mario.h>
#include <spm/mario_pouch.h>
#include <spm/mobjdrv.h>
#include <spm/memory.h>
#include <spm/msgdrv.h>
#include <spm/npcdrv.h>
#include <spm/parse.h>
#include <spm/seqdef.h>
#include <spm/seqdrv.h>
#include <spm/seq_title.h>
#include <spm/spmario.h>
#include <spm/swdrv.h>
#include <spm/system.h>
#include <spm/rel/dan.h>
#include <spm/rel/machi.h>
#include <wii/os/OSError.h>
#include <wii/cx.h>
#include <wii/gx.h>
#include <msl/math.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <cstdio>

namespace mod
{
    static spm::seqdef::SeqFunc *seq_titleMainReal;
    static void seq_titleMainOverride(spm::seqdrv::SeqWork *wp)
    {
        wii::gx::GXColor notgreen = {200, 50, 25, 255};
        f32 scale = 0.8f;
        const char *msg = "SPM Speed Patches v1";
        spm::fontmgr::FontDrawStart();
        spm::fontmgr::FontDrawEdge();
        spm::fontmgr::FontDrawColor(&notgreen);
        spm::fontmgr::FontDrawScale(scale);
        spm::fontmgr::FontDrawNoiseOff();
        spm::fontmgr::FontDrawRainbowColorOff();
        f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
        spm::fontmgr::FontDrawString(x, 200.0f, msg);
        seq_titleMainReal(wp);
    }
    static void titleScreenCustomTextPatch()
    {
        seq_titleMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main;
        spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main = &seq_titleMainOverride;
    }

    // Chapter 1 Wrong Warp
    EVT_BEGIN(ch1_ww)
    RETURN_FROM_CALL()

    // Pre-2 Block
    EVT_BEGIN(pre2_ladder_setattr)
    USER_FUNC(spm::evt_snd::evt_snd_bgmoff_f_d, 0, 0)
    USER_FUNC(spm::evt_snd::evt_snd_bgmon, 0, PTR("BGM_EVT_RELAXATION1"))
    RETURN()
    EVT_END()

    EVT_BEGIN(pre2_ladder)
    USER_FUNC(spm::evt_mobj::evt_mobj_blk, 32769, PTR("mame"), FLOAT(237.5), 75, 0, 0, PTR(pre2_ladder_setattr), 0)
    RETURN_FROM_CALL()

    // 8-1 Torch Skip
    EVT_BEGIN(bowser_skip)
    IF_SMALL(GSW(0), 363)
    SET(GSW(0), 363)
    END_IF()
    RETURN_FROM_CALL()

    // 8-4 Tippi Door Skip
    EVT_BEGIN(tippi_skip)
    IF_SMALL(GSW(0), 407)
    SET(GSW(0), 407)
    END_IF()
    RETURN_FROM_CALL()
    
    // Override rift and spawn blue switch in 8-3
    EVT_BEGIN(ls3_12_bs)
    WAIT_MSEC(500)
    USER_FUNC(spm::evt_eff::evt_eff, 0, PTR("event_flash"), 0, 30, 114, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(spm::evt_snd::evt_snd_sfxon_3d, PTR("SFX_MAP_BLUEBLOCK_APPEAR1"), 30, 114, 0)
    WAIT_MSEC(500)
    USER_FUNC(spm::evt_mobj::evt_mobj_blk, 25, PTR("b_00"), 30, 100, 0, 0, spm::ls3_12::ls3_12_blk_hit, 0)
    RETURN_FROM_CALL()

    // Override rifts and spawn blue switches for the rest of the Dimentio Chase
    EVT_BEGIN(dimentio_chase_bss)
    USER_FUNC(spm::evt_mobj::evt_mobj_blk, 25, PTR("_dmen"), LW(14), LW(15), 0, 0, spm::evt_dimen::evt_dimen_handle_mapchanges, 0)
    RETURN_FROM_CALL()

    static void evtPatches()
    {
        // Initialize the patches to the EVT interpreter to add custom opcodes
        evtpatch::evtmgrExtensionInit();

        evtpatch::hookEvtReplace(spm::he1_01::tippi_tutorial_evt, 41, ch1_ww);
        spm::evtmgr_cmd::EvtScriptCode *ls1_06_init_evt = spm::map_data::mapDataPtr("ls1_06")->initScript;
        evtpatch::hookEvt(ls1_06_init_evt, 17, pre2_ladder); //Turtle
        spm::evtmgr_cmd::EvtScriptCode *ls1_10_init_evt = spm::map_data::mapDataPtr("ls1_10")->initScript;
        evtpatch::hookEvt(ls1_10_init_evt, 2, bowser_skip);
        spm::evtmgr_cmd::EvtScriptCode *ls4_13_init_evt = spm::map_data::mapDataPtr("ls4_13")->initScript;
        evtpatch::hookEvt(ls4_13_init_evt, 2, tippi_skip);
        evtpatch::hookEvtReplaceBlock(spm::ls3_12::ls3_12_dimentio_intro, 59, ls3_12_bs, 61);
        evtpatch::hookEvtReplaceBlock(spm::evt_dimen::evt_dimen_handle_cutscenes, 137, dimentio_chase_bss, 138);
    }

    void main()
    {
        wii::os::OSReport("SPM Rel Loader: the fast mod has ran!\n");
        titleScreenCustomTextPatch();
        evtPatches();
    }
}
