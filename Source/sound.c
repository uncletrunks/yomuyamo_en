/******************************************

		�u��ނ���v

					�T�E���h�f�[�^

 ******************************************/

#include	"base.h"
#include	"sound.h"


const u_char	seq_main[] = {					/* ���C��BGM */
#include	"seq\battle1.c"
};

const u_char	seq_over[] = {					/* �I��BGM */
#include	"seq\kwover2.c"
};

const u_char	seq_title[] = {					/* �^�C�g��BGM */
#include	"seq\title.c"
};

#include	"wave\adpcm_move.c"					/* ���ʉ� */
#include	"wave\adpcm_select.c"
#include	"wave\adpcm_pass.c"
#include	"wave\adpcm_attack.c"
#include	"wave\adpcm_guard.c"

const u_char*	se_table[] = {					/* ���ʉ��e�[�u�� */
					ADPCM_MOVE, ADPCM_SELECT, ADPCM_PASS, ADPCM_ATTACK, ADPCM_GUARD
				};

/******************** End of File ******************************************/