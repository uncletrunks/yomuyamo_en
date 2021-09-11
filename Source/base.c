/**************************

		��{�֐�

 **************************/

#include	"base.h"


u_char	vram[SCREEN_W*SCREEN_H];			/* ��ʃo�b�t�@ */
int				fade_flag = 0;				/* ��ʃt�F�[�h��� */
static int		fade_cnt = 0;				/* ��ʂ̖��邳 */
static int		sys_bright;					/* ���邳�ݒ�l */

Bool			sound_flag = TRUE;			/* �T�E���h�Đ��t���O */
int				snd_fade_cnt = 0;			/* �T�E���h�t�F�[�h�J�E���^ */

int		common_counter;						/* �ėp�J�E���^ */
int		master_volume;						/* �S�̂̉��� */


void		app_main();						/* �A�v�����C�� */
void		init_app();						/* �A�v�������� */
void		end_app();						/* �A�v���I�� */

/***********************************
    �X�v���C�g�`��
		����	 spr = �X�v���C�g
				x, y = �`����W
 ***********************************/
void	draw_sprite(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->sx, spr->sy, spr->w, spr->h, spr->param);
	pceLCDDrawObject(obj);								/* �`�� */
}

/***************************************
    �X�v���C�g�ݒ�
		����	   spr = �X�v���C�g
				   bmp = �r�b�g�}�b�v
				sx, sy = �]�������W
				  w, h = �傫��
				 param = �p�����[�^
 ***************************************/
void	set_sprite(Sprite* spr, PIECE_BMP* bmp, int sx, int sy, int w, int h, int param)
{
	spr->bmp   = bmp;
	spr->sx    = sx;
	spr->sy    = sy;
	spr->w     = w;
	spr->h     = h;
	spr->param = param;
}

/**************************************
    BMP�ݒ�
		����	bmp = BMP�o�b�t�@
				pat = �p�^�[���f�[�^
 **************************************/
void	set_bmp(PIECE_BMP* bmp, const u_char* pat)
{
	bmp->header = *(PBMP_FILEHEADER*)pat;						/* �w�b�_ */
	bmp->buf    = pat + sizeof(PBMP_FILEHEADER);				/* �p�^�[�� */
	bmp->mask   = bmp->buf + bmp->header.w*bmp->header.h/4;		/* �}�X�N */
}


/*************************************
    BGM�Đ�
		����	seq = BGM�V�[�P���X
 *************************************/
void	play_music(const u_char* seq)
{
	if ( snd_fade_cnt ) {
		StopMusic();
		snd_fade_cnt = 0;
	}
	if ( sound_flag ) {
		pceWaveSetMasterAtt(master_volume);
	}
	PlayMusic(seq);
}

/****************************
    �T�E���h�t�F�[�h�A�E�g
 ****************************/
void	fade_sound(void)
{
	if ( snd_fade_cnt == 0 ) {
		snd_fade_cnt = 24;
	}
}


/**********
    ���s
 **********/
void	pceAppProc(int cnt)
{
	pceLCDTrans();								/* ����ʓ]�� */
	if ( fade_flag ) {
		if ( fade_flag == 1 ) {					/* �t�F�[�h�C�� */
			if ( ++fade_cnt >= FADE_COUNT ) {
				fade_flag = 0;
				fade_cnt = FADE_COUNT;
			}
		}
		else {									/* �t�F�[�h�A�E�g */
			if ( --fade_cnt <= 0 ) {
				fade_flag = 0;
				fade_cnt = 0;
			}
		}
		pceLCDSetBright(fade_cnt*sys_bright/FADE_COUNT);
	}
	if ( snd_fade_cnt ) {						/* �T�E���h�t�F�[�h�A�E�g */
		if ( --snd_fade_cnt == 0 ) {
			StopMusic();						/* BGM��~ */
		}
		else if ( sound_flag ) {				/* ���ʕύX */
			pceWaveSetMasterAtt((master_volume*snd_fade_cnt + 127*(24 - snd_fade_cnt))/24);
		}
	}
	common_counter = cnt;						/* �ėp�J�E���^ */
	rand();

	app_main();									/* �A�v�����C�� */
}

/************
    ������
 ************/
void	pceAppInit(void)
{
	PCETIME	tm;

	pceLCDDispStop();									/* ��ʕ\����~ */
	pceLCDSetBuffer(vram);								/* ��ʃo�b�t�@�ݒ� */
	memset(vram, 0, SCREEN_W*SCREEN_H);					/* ��ʃN���A */
	sys_bright = pceLCDSetBright(0);					/* ���邳�ݒ�l */
	pceLCDDispStart();									/* ��ʕ\���J�n */

	InitMusic();										/* ���y���C�u���������� */
	master_volume = pceWaveSetMasterAtt(INVALIDVAL);	/* �S�̂̉��� */

	pceTimeGet(&tm);
	srand((int)tm.s100 + (int)tm.ss*100 + (int)tm.mi*100*60);	/* ���������� */

	pceAppSetProcPeriod(25);							/* �����ݒ� */

	init_app();											/* �A�v�������� */
}

/**********
    �I��
 **********/
void	pceAppExit(void)
{
	end_app();										/* �A�v���I�� */

	pceWaveSetMasterAtt(master_volume);				/* ���ʂ�߂� */
	pceLCDSetBright(sys_bright);					/* ���邳��߂� */
}

/************** End of File ***********************************************/