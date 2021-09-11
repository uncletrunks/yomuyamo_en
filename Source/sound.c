/******************************************

		「よむやも」

					サウンドデータ

 ******************************************/

#include	"base.h"
#include	"sound.h"


const u_char	seq_main[] = {					/* メインBGM */
#include	"seq\battle1.c"
};

const u_char	seq_over[] = {					/* 終了BGM */
#include	"seq\kwover2.c"
};

const u_char	seq_title[] = {					/* タイトルBGM */
#include	"seq\title.c"
};

#include	"wave\adpcm_move.c"					/* 効果音 */
#include	"wave\adpcm_select.c"
#include	"wave\adpcm_pass.c"
#include	"wave\adpcm_attack.c"
#include	"wave\adpcm_guard.c"

const u_char*	se_table[] = {					/* 効果音テーブル */
					ADPCM_MOVE, ADPCM_SELECT, ADPCM_PASS, ADPCM_ATTACK, ADPCM_GUARD
				};

/******************** End of File ******************************************/