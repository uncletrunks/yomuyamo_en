/**************************

		基本関数

 **************************/

#include	"base.h"


u_char	vram[SCREEN_W*SCREEN_H];			/* 画面バッファ */
int				fade_flag = 0;				/* 画面フェード状態 */
static int		fade_cnt = 0;				/* 画面の明るさ */
static int		sys_bright;					/* 明るさ設定値 */

Bool			sound_flag = TRUE;			/* サウンド再生フラグ */
int				snd_fade_cnt = 0;			/* サウンドフェードカウンタ */

int		common_counter;						/* 汎用カウンタ */
int		master_volume;						/* 全体の音量 */


void		app_main();						/* アプリメイン */
void		init_app();						/* アプリ初期化 */
void		end_app();						/* アプリ終了 */

/***********************************
    スプライト描画
		引数	 spr = スプライト
				x, y = 描画座標
 ***********************************/
void	draw_sprite(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x, y, spr->sx, spr->sy, spr->w, spr->h, spr->param);
	pceLCDDrawObject(obj);								/* 描画 */
}

/***************************************
    スプライト設定
		引数	   spr = スプライト
				   bmp = ビットマップ
				sx, sy = 転送元座標
				  w, h = 大きさ
				 param = パラメータ
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
    BMP設定
		引数	bmp = BMPバッファ
				pat = パターンデータ
 **************************************/
void	set_bmp(PIECE_BMP* bmp, const u_char* pat)
{
	bmp->header = *(PBMP_FILEHEADER*)pat;						/* ヘッダ */
	bmp->buf    = pat + sizeof(PBMP_FILEHEADER);				/* パターン */
	bmp->mask   = bmp->buf + bmp->header.w*bmp->header.h/4;		/* マスク */
}


/*************************************
    BGM再生
		引数	seq = BGMシーケンス
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
    サウンドフェードアウト
 ****************************/
void	fade_sound(void)
{
	if ( snd_fade_cnt == 0 ) {
		snd_fade_cnt = 24;
	}
}


/**********
    実行
 **********/
void	pceAppProc(int cnt)
{
	pceLCDTrans();								/* 実画面転送 */
	if ( fade_flag ) {
		if ( fade_flag == 1 ) {					/* フェードイン */
			if ( ++fade_cnt >= FADE_COUNT ) {
				fade_flag = 0;
				fade_cnt = FADE_COUNT;
			}
		}
		else {									/* フェードアウト */
			if ( --fade_cnt <= 0 ) {
				fade_flag = 0;
				fade_cnt = 0;
			}
		}
		pceLCDSetBright(fade_cnt*sys_bright/FADE_COUNT);
	}
	if ( snd_fade_cnt ) {						/* サウンドフェードアウト */
		if ( --snd_fade_cnt == 0 ) {
			StopMusic();						/* BGM停止 */
		}
		else if ( sound_flag ) {				/* 音量変更 */
			pceWaveSetMasterAtt((master_volume*snd_fade_cnt + 127*(24 - snd_fade_cnt))/24);
		}
	}
	common_counter = cnt;						/* 汎用カウンタ */
	rand();

	app_main();									/* アプリメイン */
}

/************
    初期化
 ************/
void	pceAppInit(void)
{
	PCETIME	tm;

	pceLCDDispStop();									/* 画面表示停止 */
	pceLCDSetBuffer(vram);								/* 画面バッファ設定 */
	memset(vram, 0, SCREEN_W*SCREEN_H);					/* 画面クリア */
	sys_bright = pceLCDSetBright(0);					/* 明るさ設定値 */
	pceLCDDispStart();									/* 画面表示開始 */

	InitMusic();										/* 音楽ライブラリ初期化 */
	master_volume = pceWaveSetMasterAtt(INVALIDVAL);	/* 全体の音量 */

	pceTimeGet(&tm);
	srand((int)tm.s100 + (int)tm.ss*100 + (int)tm.mi*100*60);	/* 乱数初期化 */

	pceAppSetProcPeriod(25);							/* 同期設定 */

	init_app();											/* アプリ初期化 */
}

/**********
    終了
 **********/
void	pceAppExit(void)
{
	end_app();										/* アプリ終了 */

	pceWaveSetMasterAtt(master_volume);				/* 音量を戻す */
	pceLCDSetBright(sys_bright);					/* 明るさを戻す */
}

/************** End of File ***********************************************/