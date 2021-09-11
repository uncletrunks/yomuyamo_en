
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<piece.h>
#include	<muslib.h>


typedef	int		Bool;

#define	TRUE	1
#define	FALSE	0

typedef	unsigned int	u_int;
typedef	unsigned short	u_short;
typedef	unsigned char	u_char;

/*** スプライト *******/
typedef struct {
	PIECE_BMP*	bmp;			/* ビットマップデータ */
	int			sx;				/* 転送元座標 */
	int			sy;
	int			w;				/* 大きさ */
	int			h;
	int			param;			/* 描画パラメータ */
} Sprite;


extern u_char	vram[SCREEN_W*SCREEN_H];				/* 画面バッファ */
extern int		fade_flag;								/* 画面フェード状態 */

#define		FADE_COUNT	32								/* フェードイン・アウトの時間 */
#define		fade_in()	{fade_flag = 1;}				/* フェードイン */
#define		fade_out()	{fade_flag = -1;}				/* フェードアウト */

extern Bool		sound_flag;								/* サウンド再生フラグ */
extern int		snd_fade_cnt;							/* サウンドフェードカウンタ */

extern int		common_counter;							/* 汎用カウンタ */
extern int		master_volume;							/* 全体の音量 */


#define	rnd(n)	(rand() % (n))							/* 乱数取得 */

extern void		draw_sprite(Sprite*, int, int);								/* スプライト描画 */
extern void		set_sprite(Sprite*, PIECE_BMP*, int, int, int, int, int);	/* スプライト設定 */
extern void		set_bmp(PIECE_BMP*, const u_char*);							/* BMP設定 */

extern void		play_music(const u_char*);				/* BGM再生 */
extern void		fade_sound(void);						/* サウンドフェードアウト */

/****************** End of File *********************************************/